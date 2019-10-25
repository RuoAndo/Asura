#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <alloca.h>
#include "timer.h"

#include <regex.h>

#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<linux/if.h>
#include<net/ethernet.h>
#include<netpacket/packet.h>
#include<netinet/if_ether.h>
#include<netinet/ip.h>
#include<netinet/ip6.h>
#include<netinet/ip_icmp.h>
#include<netinet/icmp6.h>
#include<netinet/tcp.h>
#include<netinet/udp.h>

#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#define WORKER_THREAD_NUM (2)
#define MAX_QUEUE_NUM (10240)
#define END_MARK_FNAME   "///"
#define END_MARK_FLENGTH 3

/* srcIP, destIP */
typedef struct _addrpair {
  map<string, string> m;
  pthread_mutex_t mutex;
} addrpair_t;
addrpair_t addrpair;

/* reduced */
typedef struct _reduced {
  map<int, int> count;
  map<int, int> tlen;
  map<int, int> ttl;
  map<int, int> sport;
  map<int, int> dport;
  pthread_mutex_t mutex;
} reduced_t;
reduced_t reduced;

// static int counter = 0;
static int chksum = 0;

struct pseudo_ip{
        struct in_addr  ip_src;
        struct in_addr  ip_dst;
        unsigned char   dummy;
        unsigned char   ip_p;
        unsigned short  ip_len;
};

struct pseudo_ip6_hdr{
        struct in6_addr src;
        struct in6_addr dst;
        unsigned long   plen;
        unsigned short  dmy1;
        unsigned char   dmy2;
        unsigned char   nxt;
};

u_int16_t checksum(u_char *data,int len)
{
register u_int32_t       sum;
register u_int16_t       *ptr;
register int     c;

        sum=0;
        ptr=(u_int16_t *)data;

        for(c=len;c>1;c-=2){
                sum+=(*ptr);
                if(sum&0x80000000){
                        sum=(sum&0xFFFF)+(sum>>16);
                }
                ptr++;
        }
        if(c==1){
                u_int16_t       val;
                val=0;
                memcpy(&val,ptr,sizeof(u_int8_t));
                sum+=val;
        }

        while(sum>>16){
                sum=(sum&0xFFFF)+(sum>>16);
        }

	return(~sum);
}

u_int16_t checksum2(u_char *data1,int len1,u_char *data2,int len2)
{
register u_int32_t       sum;
register u_int16_t       *ptr;
register int     c;

        sum=0;
        ptr=(u_int16_t *)data1;
        for(c=len1;c>1;c-=2){
                sum+=(*ptr);
                if(sum&0x80000000){
                        sum=(sum&0xFFFF)+(sum>>16);
                }
                ptr++;
        }
        if(c==1){
                u_int16_t       val;
                val=((*ptr)<<8)+(*data2);
                sum+=val;
                if(sum&0x80000000){
                        sum=(sum&0xFFFF)+(sum>>16);
                }
                ptr=(u_int16_t *)(data2+1);
                len2--;
        }
        else{
                ptr=(u_int16_t *)data2;
        }
        for(c=len2;c>1;c-=2){
                sum+=(*ptr);
                if(sum&0x80000000){
                        sum=(sum&0xFFFF)+(sum>>16);
                }
                ptr++;
        }
        if(c==1){
                u_int16_t       val;
                val=0;
                memcpy(&val,ptr,sizeof(u_int8_t));
                sum+=val;
        }

        while(sum>>16){
                sum=(sum&0xFFFF)+(sum>>16);
        }

	return(~sum);
}

int checkIPchecksum(struct iphdr *iphdr,u_char *option,int optionLen)
{
  unsigned short	sum;

	if(optionLen==0){
		sum=checksum((u_char *)iphdr,sizeof(struct iphdr));
		if(sum==0||sum==0xFFFF){
			return(1);
		}
		else{
			return(0);
		}
	}
	else{
		sum=checksum2((u_char *)iphdr,sizeof(struct iphdr),option,optionLen);
		if(sum==0||sum==0xFFFF){
			return(1);
		}
		else{
			return(0);
		}
	}
}

int checkIPDATAchecksum(struct iphdr *iphdr,unsigned char *data,int len)
{
  struct pseudo_ip        p_ip;
  unsigned short  sum;

        memset(&p_ip,0,sizeof(struct pseudo_ip));
        p_ip.ip_src.s_addr=iphdr->saddr;
        p_ip.ip_dst.s_addr=iphdr->daddr;
        p_ip.ip_p=iphdr->protocol;
        p_ip.ip_len=htons(len);

        sum=checksum2((unsigned char *)&p_ip,sizeof(struct pseudo_ip),data,len);
        if(sum==0||sum==0xFFFF){
                return(1);
        }
        else{
                return(0);
        }
}

int checkIP6DATAchecksum(struct ip6_hdr *ip,unsigned char *data,int len)
{
  struct pseudo_ip6_hdr   p_ip;
  unsigned short  sum;

        memset(&p_ip,0,sizeof(struct pseudo_ip6_hdr));

        memcpy(&p_ip.src,&ip->ip6_src,sizeof(struct in6_addr));
        memcpy(&p_ip.dst,&ip->ip6_dst,sizeof(struct in6_addr));
        p_ip.plen=ip->ip6_plen;
        p_ip.nxt=ip->ip6_nxt;

	sum=checksum2((unsigned char *)&p_ip,sizeof(struct pseudo_ip6_hdr),data,len);
	if(sum==0||sum==0xFFFF){
		return(1);
	}
	else{
		return(0);
	}
}

typedef struct _result {
    int num;
    char* fname;
    pthread_mutex_t mutex;    
} result_t;
result_t result;

typedef struct _queue {
    char* fname[MAX_QUEUE_NUM];
    int flength[MAX_QUEUE_NUM];
    int rp, wp;
    int remain;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} queue_t;

typedef struct _thread_arg {
    int id;
    int cpuid;
    queue_t* q;
    char* srchstr;
    char* dirname;
    int filenum;
} thread_arg_t;

char *ip_ip2str(u_int32_t ip,char *buf,socklen_t size)
{
  struct in_addr*addr;

  addr=(struct in_addr *)&ip;
  inet_ntop(AF_INET,addr,buf,size);

  return(buf);
}

int ProcIpHeader(struct iphdr *iphdr,u_char *option,int optionLen,FILE *fp)
{
  int i;
  char buf[80];
  
  char src[80];
  char dst[80];

  string saddr;
  string daddr;
  
  fprintf(fp,"protocol=%u, ",iphdr->protocol);
  fprintf(fp,"saddr=%s,",ip_ip2str(iphdr->saddr,buf,sizeof(buf)));
  fprintf(fp,"daddr=%s\n",ip_ip2str(iphdr->daddr,buf,sizeof(buf)));
  fprintf(fp,"length=%u\n",ntohs(iphdr->tot_len));

  sprintf(src, ip_ip2str(iphdr->saddr,buf,sizeof(buf)));
  sprintf(dst, ip_ip2str(iphdr->daddr,buf,sizeof(buf)));

  saddr = string(src);
  daddr = string(dst);

  pthread_mutex_lock(&addrpair.mutex);
  addrpair.m.insert(pair<string, string>(saddr,daddr));
  pthread_mutex_unlock(&addrpair.mutex); 

  /*
  fprintf(fp,"ip--------------------------------------\n");
  fprintf(fp,"version=%u,",iphdr->version);
  fprintf(fp,"ihl=%u,",iphdr->ihl);
  fprintf(fp,"tos=%x,",iphdr->tos);
  fprintf(fp,"tot_len=%u,",ntohs(iphdr->tot_len));
  fprintf(fp,"id=%u\n",ntohs(iphdr->id));
  fprintf(fp,"frag_off=%x,%u,",(ntohs(iphdr->frag_off)>>13)&0x07,ntohs(iphdr->frag_off)&0x1FFF);
  fprintf(fp,"ttl=%u,",iphdr->ttl);
  fprintf(fp,"protocol=%u",iphdr->protocol);
  if(iphdr->protocol<=17){
    fprintf(fp,"(%s),",Proto[iphdr->protocol]);
  }
  else{
    fprintf(fp,"(undefined),");
  }
  fprintf(fp,"check=%x\n",iphdr->check);
  fprintf(fp,"saddr=%s,",ip_ip2str(iphdr->saddr,buf,sizeof(buf)));
  fprintf(fp,"daddr=%s\n",ip_ip2str(iphdr->daddr,buf,sizeof(buf)));
  */

  return(0);
}

int AnalyzeIp(u_char *data,int size)
{
  u_char*ptr;
  int lest;
  struct iphdr*iphdr;
  u_char*option;
  int optionLen,len;
  unsigned short  sum;

  struct tcphdr *tcp;
  struct udphdr *udp;
  
  ptr=data;
  lest=size;

  if(lest<sizeof(struct iphdr)){
    fprintf(stderr,"lest(%d)<sizeof(struct iphdr)\n",lest);
    return(-1);
  }
  iphdr=(struct iphdr *)ptr;
  ptr+=sizeof(struct iphdr);
  lest-=sizeof(struct iphdr);

  optionLen=iphdr->ihl*4-sizeof(struct iphdr);
  if(optionLen>0){
    if(optionLen>=1500){
      fprintf(stderr,"IP optionLen(%d):too big\n",optionLen);
      return(-1);
    }
    option=ptr;
    ptr+=optionLen;
    lest-=optionLen;
  }

  if(checkIPchecksum(iphdr,option,optionLen)==0){
    fprintf(stderr,"bad ip checksum(%d) \n", chksum);
    chksum++;
    return(-1);
  }

  /*
  printf("| Source IP Address: %15s|\n", inet_ntoa(*(struct in_addr *) &(iphdr->saddr)));
  printf("| Dest IP Address: %15s|\n", inet_ntoa(*(struct in_addr *) &(iphdr->daddr)));
  */  

  ProcIpHeader(iphdr,option,optionLen,stdout);

  /*
  fprintf(fp,"protocol=%u, ",iphdr->protocol);
  */

  /*
  if(iphdr->protocol == 6)
    {
      tcp = (struct tcphdr *) ptr;
      ptr += ((int) (tcp->th_off) << 2);   

      printf("[TCP] sPort=%u, dPort=%u\n", ntohs(tcp->th_sport), ntohs(tcp->th_dport));  
    }

  if(iphdr->protocol == IPPROTO_UDP)
    {
      udp = (struct udphdr *) ptr;
      // ptr += ((int) (udp->uh_off) << 2);
      ptr += sizeof (struct udphdr); 
      printf("[UDP] sPort=%u, dPort=%u\n", ntohs(udp->uh_sport), ntohs(udp->uh_dport));  
    }
  */  

  /*
  if(iphdr->protocol==IPPROTO_ICMP){
    len=ntohs(iphdr->tot_len)-iphdr->ihl*4;
    sum=checksum(ptr,len);
    if(sum!=0&&sum!=0xFFFF){
      fprintf(stderr,"bad icmp checksum\n");
      return(-1);
    }
    AnalyzeIcmp(ptr,lest);
  }
  else if(iphdr->protocol==IPPROTO_TCP){
    len=ntohs(iphdr->tot_len)-iphdr->ihl*4;
    if(checkIPDATAchecksum(iphdr,ptr,len)==0){
      fprintf(stderr,"bad tcp checksum\n");
      return(-1);
    }
    AnalyzeTcp(ptr,lest);
  }
  else if(iphdr->protocol==IPPROTO_UDP){
    struct udphdr*udphdr;
    udphdr=(struct udphdr *)ptr;
    len=ntohs(iphdr->tot_len)-iphdr->ihl*4;
    if(udphdr->check!=0&&checkIPDATAchecksum(iphdr,ptr,len)==0){
      fprintf(stderr,"bad udp checksum\n");
      return(-1);
    }
    AnalyzeUdp(ptr,lest);
  }
  */
  
  return(0);
}

int PrintTcp(struct tcphdr *tcphdr,FILE *fp)
{
	fprintf(fp,"tcp-------------------------------------\n");

        fprintf(fp,"source=%u,",ntohs(tcphdr->source));
        fprintf(fp,"dest=%u\n",ntohs(tcphdr->dest));
        fprintf(fp,"seq=%u\n",ntohl(tcphdr->seq));
        fprintf(fp,"ack_seq=%u\n",ntohl(tcphdr->ack_seq));
        fprintf(fp,"doff=%u,",tcphdr->doff);
        fprintf(fp,"urg=%u,",tcphdr->urg);
        fprintf(fp,"ack=%u,",tcphdr->ack);
        fprintf(fp,"psh=%u,",tcphdr->psh);
        fprintf(fp,"rst=%u,",tcphdr->rst);
        fprintf(fp,"syn=%u,",tcphdr->syn);
        fprintf(fp,"fin=%u,",tcphdr->fin);
        fprintf(fp,"th_win=%u\n",ntohs(tcphdr->window));
        fprintf(fp,"th_sum=%u,",ntohs(tcphdr->check));
        fprintf(fp,"th_urp=%u\n",ntohs(tcphdr->urg_ptr));

	return(0);
}

int AnalyzeTcp(u_char *data,int size)
{
u_char	*ptr;
int	lest;
struct tcphdr	*tcphdr;

	ptr=data;
	lest=size;

	if(lest<sizeof(struct tcphdr)){
		fprintf(stderr,"lest(%d)<sizeof(struct tcphdr)\n",lest);
		return(-1);
	}

	tcphdr=(struct tcphdr *)ptr;
	ptr+=sizeof(struct tcphdr);
	lest-=sizeof(struct tcphdr);

	PrintTcp(tcphdr,stdout);
	
	return(0);
}

char *tcp_ftoa(int flag)
{
  static int  f[] = {'U', 'A', 'P', 'R', 'S', 'F'}; /* TCPフラグを表す文字  */
#define TCP_FLG_MAX (sizeof f / sizeof f[0])
  static char str[TCP_FLG_MAX + 1];            /* 戻り値を格納するバッファ  */
  unsigned int mask = 1 << (TCP_FLG_MAX - 1);  /* フラグを取り出すマスク    */
  int i;                                       /* ループ変数                */

  for (i = 0; i < TCP_FLG_MAX; i++) {
    if (((flag << i) & mask) != 0)
      str[i] = f[i];
    else
      str[i] = '0';
  }
  str[i] = '\0';

  return str;
}



int AnalyzeIp2(u_char *data,int size)
{
  u_char*ptr;
  int lest;
  struct iphdr*iphdr;
  u_char*option;
  int optionLen,len;
  unsigned short  sum;

  struct tcphdr *tcp;
  struct udphdr *udp;

  int map_counter = 0;
  
  ptr=data;
  lest=size;

  map<string, string> myAddrPair;

  string src;
  string dst;
  
  int tlen = 0;
  int ttl = 0;

  int sport = 0;
  int dport = 0;
  
  int count = 0;
  
  pthread_mutex_lock(&addrpair.mutex);
  myAddrPair = addrpair.m;
  pthread_mutex_unlock(&addrpair.mutex); 
  
  if(lest<sizeof(struct iphdr)){
    fprintf(stderr,"lest(%d)<sizeof(struct iphdr)\n",lest);
    return(-1);
  }
  iphdr=(struct iphdr *)ptr;
  ptr+=sizeof(struct iphdr);
  lest-=sizeof(struct iphdr);

  optionLen=iphdr->ihl*4-sizeof(struct iphdr);
  if(optionLen>0){
    if(optionLen>=1500){
      fprintf(stderr,"IP optionLen(%d):too big\n",optionLen);
      return(-1);
    }
    option=ptr;
    ptr+=optionLen;
    lest-=optionLen;
  }

  if(checkIPchecksum(iphdr,option,optionLen)==0){
    fprintf(stderr,"bad ip checksum(%d) \n", chksum);
    chksum++;
    return(-1);
  }

  if(iphdr->protocol == IPPROTO_TCP)
    {
      // len=ntohs(iphdr->tot_len)-iphdr->ihl*4;
      /*
      if(checkIPDATAchecksum(iphdr,ptr,len)==0){
	fprintf(stderr,"bad tcp checksum\n");
	return(-1);
      }
      */
      
      // AnalyzeTcp(ptr,lest);

      tcp = (struct tcphdr *) ptr;
      ptr += ((int) (tcp->th_off) << 2);   

      printf("[TCP] sPort=%u, dPort=%u do=%u wo=%u \n", ntohs(tcp->th_sport), ntohs(tcp->th_dport), tcp->th_off, ntohs(tcp->th_win));

      sport = ntohs(tcp->th_sport);
      dport = ntohs(tcp->th_dport);
      
      /*
	printf("Protocol: TCP\n");
	printf("+-------------------------+-------------------------+\n");
	printf("| Source Port:       %5u| Destination Port:  %5u|\n",
         ntohs(tcp->th_sport), ntohs(tcp->th_dport));
	 printf("+-------------------------+-------------------------+\n");
	 printf("| Sequence Number:                        %10lu|\n",
         (unsigned long) ntohl(tcp->th_seq));
	 printf("+---------------------------------------------------+\n");
	 printf("| Acknowledgement Number:                 %10lu|\n",
         (unsigned long) ntohl(tcp->th_ack));
	 printf("+------+---------+--------+-------------------------+\n");
	 printf("| DO:%2u| Reserved|F:%6s| Window Size:       %5u|\n",
         tcp->th_off, tcp_ftoa(tcp->th_flags), ntohs(tcp->th_win));
	 printf("+------+---------+--------+-------------------------+\n");
	 printf("| Checksum:          %5u| Urgent Pointer:    %5u|\n",
         ntohs(tcp->th_sum), ntohs(tcp->th_urp));
	 printf("+-------------------------+-------------------------+\n");
      */
      
    }

  /*
  if(iphdr->protocol == IPPROTO_UDP)
    {
      udp = (struct udphdr *) ptr;
      // ptr += ((int) (udp->uh_off) << 2);
      ptr += sizeof (struct udphdr); 
      printf("[UDP] sPort=%u, dPort=%u\n", ntohs(udp->uh_sport), ntohs(udp->uh_dport));  
    }
  */
  
  /*
  printf("| Source IP Address: %15s|\n", inet_ntoa(*(struct in_addr *) &(iphdr->saddr)));
  printf("| Dest IP Address: %15s|\n", inet_ntoa(*(struct in_addr *) &(iphdr->daddr)));
  */

  /* reduce */
  
  map<string, string>::iterator itr;

  /* retrivel from header */
  src = inet_ntoa(*(struct in_addr *) &(iphdr->saddr));
  dst = inet_ntoa(*(struct in_addr *) &(iphdr->daddr));
  tlen = ntohs(iphdr->tot_len);
  ttl = iphdr->ttl;

  // printf("DEBUG:ttl=%u \n",iphdr->ttl);
  
  map_counter = 0;
  for (itr = myAddrPair.begin(); itr != myAddrPair.end(); itr++)
    {
      if(src == itr->first && dst == itr->second) {
	std::cout << "HIT:" << src << ":" << dst << std::endl;

	pthread_mutex_lock(&reduced.mutex);
	std::map<int, int>::iterator it; 
	
	it = reduced.count.find(map_counter);
	if(it == reduced.count.end())
	  {
	    reduced.count.insert(std::make_pair(map_counter, 1));
	  }
	else
	  {
	    count = (int)it->second + 1;
	    reduced.count.erase(map_counter);
	    reduced.count.insert(std::make_pair(map_counter, count));
	  }   

	/* tlen */
	it = reduced.tlen.find(map_counter);
	if(it == reduced.tlen.end())
	  {
	    reduced.tlen.insert(std::make_pair(map_counter, tlen));
	  }
	else
	  {
	    tlen = tlen + (int)it->second;
	    reduced.tlen.erase(map_counter);
	    reduced.tlen.insert(std::make_pair(map_counter, tlen));
	  }

	/* ttl */
	it = reduced.ttl.find(map_counter);
	if(it == reduced.ttl.end())
	  {
	    reduced.ttl.insert(std::make_pair(map_counter, ttl));
	  }
	else
	  {
	    ttl = ttl + (int)it->second;
	    reduced.ttl.erase(map_counter);
	    reduced.ttl.insert(std::make_pair(map_counter, ttl));
	  }

	/* sport */
	it = reduced.sport.find(map_counter);
	if(it == reduced.sport.end())
	  {
	    reduced.sport.insert(std::make_pair(map_counter, sport));
	  }
	else
	  {
	    sport = sport + (int)it->second;
	    reduced.sport.erase(map_counter);
	    reduced.sport.insert(std::make_pair(map_counter, sport));
	  }

	/* dport */
	it = reduced.dport.find(map_counter);
	if(it == reduced.dport.end())
	  {
	    reduced.dport.insert(std::make_pair(map_counter, dport));
	  }
	else
	  {
	    dport = dport + (int)it->second;
	    reduced.dport.erase(map_counter);
	    reduced.dport.insert(std::make_pair(map_counter, dport));
	  }   
		
	pthread_mutex_unlock(&reduced.mutex);  
      }
      map_counter++;
    }
 
  return(0);
}

int traverse_buffer2(char* buf, int thread_id, char* filename)
{
  regex_t preg;
  char *regex ="([0-9]{3})\\.([0-9]{3})\\.([0-9]{3})\\.([0-9]{3})";
  regmatch_t   pmatch[5];

  FILE *file;

  regcomp(&preg,regex,REG_EXTENDED|REG_NEWLINE);
    
  if( regexec(&preg,buf,5,pmatch,0) != REG_NOMATCH ) {
    /*
      printf("THREAD:%d, filename:%s, IP='%.*s '[%d]:%s ",
	     thread_id, filename, pmatch[4].rm_eo-pmatch[1].rm_so,&buf[pmatch[1].rm_so], pmatch[1].rm_so,buf);
    */

    /*
      printf("   filename:%s, IP='%.*s '[%d]:%s ", filename, pmatch[4].rm_eo-pmatch[1].rm_so,&buf[pmatch[1].rm_so], pmatch[1].rm_so,buf);
    */
    /*
      file = fopen("dump.txt","a");      
      fprintf(file,"THREAD:%d IP= '%.*s'", thread_id, pmatch[4].rm_eo-pmatch[1].rm_so, &buf[pmatch[1].rm_so]);
      fclose(file);
    */
      return 1;
    }  

   regfree(&preg);
   return 0;
}

int traverse_file(char* filename, char* srchstr, int thread_id) {
    char buf[256];
    int n = 0, sumn = 0;

    FILE    *fp;
    int     data;	
    char    tmp[256];
    
    char    *s1 = "08";
    char    *s2 = "00";
    char    *s3 = "45";

    int j;
    
    u_char *ptr;
	
    int counter = 0;
	
    fp = fopen(filename, "rb");
    if(fp == NULL){
      printf("file cannot be opened. \n");
      return 0;
    }

    for(;;){
      
      if((data = getc(fp) ) == EOF){
	printf("\n"); fclose(fp); return 1;}
      
      /* init */
      for(j=0;j<256;j++)
	tmp[j] = 0;
      for(j=0;j<256;j++)
	buf[j] = 0;
	  
      sprintf(tmp, "%02X", data);
      if(strcmp(tmp, s1)==0)
	{
	  data = getc(fp);	
	  sprintf(tmp, "%02X", data);
	  
	  if(strcmp(tmp, s2)==0)
	    {
	      data = getc(fp);
	      sprintf(tmp, "%02X", data);

	      sprintf(tmp, "%02X", data);
	      if(strcmp(tmp, s3)==0)
		{
		  printf("worker1:threadID:%d:filename:%s IP 080045:counter:%d \n", thread_id, filename, counter);
		  
		  fseek(fp,-1.5L,SEEK_CUR);
		  if (fgets(buf, sizeof(struct iphdr)+8, fp) != NULL)
		    {
		      ptr = buf;
		      AnalyzeIp(ptr,sizeof(struct iphdr));
		      counter = counter + 1;
		    }
		  fseek(fp,-3L,SEEK_CUR);
		}
	      
	    } // if(strcmp(tmp,s2)	      
	} // if(strcmp(tmp,s1) *s1 = "08";
	    
    } // for (;;)

    fclose(fp);
    return sumn;
}


int traverse_file2(char* filename, char* srchstr, int thread_id) {
    char buf[256];
    int n = 0, sumn = 0;

    FILE    *fp;
    int     data;	
    char    tmp[256];
    
    char    *s1 = "08";
    char    *s2 = "00";
    char    *s3 = "45";

    int j;
    
    u_char *ptr;
	
    int counter = 0;
	
    fp = fopen(filename, "rb");
    if(fp == NULL){
      printf("file cannot be opened. \n");
      return 0;
    }

    for(;;){
      
      if((data = getc(fp) ) == EOF){
	printf("\n"); fclose(fp); return 1;}
      
      /* init */
      for(j=0;j<256;j++)
	tmp[j] = 0;
      for(j=0;j<256;j++)
	buf[j] = 0;
	  
      sprintf(tmp, "%02X", data);
      if(strcmp(tmp, s1)==0)
	{
	  data = getc(fp);	
	  sprintf(tmp, "%02X", data);
	  
	  if(strcmp(tmp, s2)==0)
	    {
	      data = getc(fp);
	      sprintf(tmp, "%02X", data);

	      sprintf(tmp, "%02X", data);
	      if(strcmp(tmp, s3)==0)
		{
		  printf("worker2:threadID:%d:filename:%s IP 080045:counter:%d \n", thread_id, filename, counter);
		  
		  fseek(fp,-1.5L,SEEK_CUR);
		  if (fgets(buf, sizeof(struct iphdr)+8, fp) != NULL)
		    {
		      ptr = buf;
		      AnalyzeIp2(ptr,sizeof(struct iphdr));
		      counter = counter + 1;
		    }
		  fseek(fp,-3L,SEEK_CUR);
		}
	      
	    } // if(strcmp(tmp,s2)	      
	} // if(strcmp(tmp,s1) *s1 = "08";
	    
    } // for (;;)

    fclose(fp);
    return sumn;
}

void initqueue(queue_t* q) {
    int i;
    q->rp = q->wp = q->remain= 0;
    for (i = 0; i < MAX_QUEUE_NUM; ++i) q->fname[i] = NULL;
    pthread_mutex_init(&q->mutex,    NULL);
    pthread_cond_init(&q->not_full,  NULL);
    pthread_cond_init(&q->not_empty, NULL);
    return;
}

void enqueue(queue_t* q, char* path, int size) {
  
    pthread_mutex_lock(&q->mutex);
    while (q->remain == MAX_QUEUE_NUM) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }
    char** fname = (char**)&q->fname[q->wp];
    if (*fname != NULL) free(*fname);
    *fname = (char*)malloc(size);
    strcpy(*fname, path);
    q->flength[q->wp] = size;
    q->wp++; q->remain++;

    if (q->wp == MAX_QUEUE_NUM) q->wp = 0;

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
    return;
}

void dequeue(queue_t* q, char** fname, int* flen) {

    pthread_mutex_lock(&q->mutex);
    while (q->remain == 0) 
        pthread_cond_wait(&q->not_empty, &q->mutex);

    *flen  = q->flength[q->rp];
    if (*fname != NULL) free(*fname);
    *fname = (char*)malloc(*flen);
    strcpy(*fname, q->fname[q->rp]);
    q->rp++; q->remain--;
    if (q->rp == MAX_QUEUE_NUM) q->rp = 0;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
    if (strcmp(*fname,"")==0) printf("rp=%d\n", q->rp-1);
    return;
}

int traverse_dir_thread(queue_t* q, char* dirname) {
    static int cnt = 0;
    struct dirent* dent;
    DIR* dd = opendir(dirname);

    if (dd == NULL) {
        printf("Could not open the directory %s\n", dirname); return 0;
    }

    while ((dent = readdir(dd)) != NULL) {
        if (strncmp(dent->d_name, ".",  2) == 0) continue;
        if (strncmp(dent->d_name, "..", 3) == 0) continue;	

        int size = strlen(dirname) + strlen(dent->d_name) + 2;
#if 0
        char* path = (char*)malloc(size);
        sprintf(path, "%s/%s", dirname, dent->d_name);

        struct stat fs;
        if (stat(path, &fs) < 0)
            continue;
        else {
            if (S_ISDIR(fs.st_mode))
                traverse_dir_thread(q, path);
            else if (S_ISREG(fs.st_mode)) {
                enqueue(q, path, size);
                cnt++;
            }
        }
#else
        {
            char* path = (char*)alloca(size);
            sprintf(path, "%s/%s", dirname, dent->d_name);

            struct stat fs;
            if (stat(path, &fs) < 0)
                continue;
            else {
                if (S_ISDIR(fs.st_mode))
                    traverse_dir_thread(q, path);
                else if (S_ISREG(fs.st_mode)) {
                    enqueue(q, path, size);
                    cnt++;
                }
            }
        }
#endif
    }
    closedir(dd);
    return cnt;
}

void master_func(thread_arg_t* arg) {
    queue_t* q = arg->q;
    int i;
    arg->filenum = traverse_dir_thread(q, arg->dirname);
    /* enqueue END_MARK */
    for (i = 0; i < WORKER_THREAD_NUM; ++i) 
        enqueue(q, END_MARK_FNAME, END_MARK_FLENGTH);
    return;
}

void worker_func(thread_arg_t* arg) {
    int flen;
    char* fname = NULL;
    queue_t* q = arg->q;
    char* srchstr = arg->srchstr;

    int thread_id = arg->id;
    // printf("DEBUG: %d \n", arg->id);

#ifdef __CPU_SET
    cpu_set_t mask;    
    __CPU_ZERO(&mask);
    __CPU_SET(arg->cpuid, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) == -1)
        printf("WARNING: faild to set CPU affinity...\n");
#endif

#if 0
    while (1) {
        int n;

        dequeue(q, &fname, &flen));

        if (strncmp(fname, END_MARK_FNAME, END_MARK_FLENGTH + 1) == 0)
            break;

        n = traverse_file(fname, srchstr, thread_id);
        pthread_mutex_lock(&result.mutex);

        if (n > result.num) {
            result.num = n;
            if (result.fname != NULL) free(result.fname);
            result.fname = (char*)malloc(flen);
            strcpy(result.fname, fname);
        }
        pthread_mutex_unlock(&result.mutex);
    }
#else
    char* my_result_fname;
    int my_result_num = 0;
    int my_result_len = 0;
    while (1) {
        int n;

        dequeue(q, &fname, &flen);

        if (strncmp(fname, END_MARK_FNAME, END_MARK_FLENGTH + 1) == 0)
            break;

        n = traverse_file(fname, srchstr, thread_id);

        if (n > my_result_num) {
            my_result_num = n;
            my_result_len = flen;
            my_result_fname = (char*)alloca(flen);
            strcpy(my_result_fname, fname);
        }
    }
    pthread_mutex_lock(&result.mutex);
    if (my_result_num > result.num) {
        result.num = my_result_num;
	if (result.fname != NULL) free(result.fname);
        result.fname = (char*)malloc(my_result_len);
        strcpy(result.fname, my_result_fname);	
    }
    pthread_mutex_unlock(&result.mutex);
#endif
    return;
}

void worker_func2(thread_arg_t* arg) {
    int flen;
    char* fname = NULL;
    queue_t* q = arg->q;
    char* srchstr = arg->srchstr;

    int thread_id = arg->id;
    printf("DEBUG: %d \n", arg->id);

#ifdef __CPU_SET
    cpu_set_t mask;    
    __CPU_ZERO(&mask);
    __CPU_SET(arg->cpuid, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) == -1)
        printf("WARNING: faild to set CPU affinity...\n");
#endif

#if 0
    while (1) {
        int n;

        dequeue(q, &fname, &flen));

        if (strncmp(fname, END_MARK_FNAME, END_MARK_FLENGTH + 1) == 0)
            break;

	printf("worker2 %s \n", fname);
	n = traverse_file2(fname, srchstr, thread_id);
	// cout << "worker2:" << fname << endl;

	/*
        n = traverse_file(fname, srchstr, thread_id);
        pthread_mutex_lock(&result.mutex);

        if (n > result.num) {
            result.num = n;
            if (result.fname != NULL) free(result.fname);
            result.fname = (char*)malloc(flen);
            strcpy(result.fname, fname);
        }
        pthread_mutex_unlock(&result.mutex);
	*/
    }
#else
    char* my_result_fname;
    int my_result_num = 0;
    int my_result_len = 0;
    while (1) {
        int n;

        dequeue(q, &fname, &flen);

        if (strncmp(fname, END_MARK_FNAME, END_MARK_FLENGTH + 1) == 0)
            break;

	printf("worker2 %s \n", fname);
	//cout << "worker2:" << fname << endl;
        n = traverse_file2(fname, srchstr, thread_id);
    }

#endif
    return;
}

void print_result(thread_arg_t* arg) {
    if (result.num) {
        printf("Total %d files\n", arg->filenum);
        printf("Max include file: %s[include %d]\n", result.fname, result.num);	
    }
    return;
}

int main(int argc, char* argv[]) {
    int i;
    int thread_num = 1 + WORKER_THREAD_NUM;
    unsigned int t, travdirtime;
    queue_t q;
    thread_arg_t targ[thread_num];
    pthread_t master;
    pthread_t worker[thread_num];
    pthread_t worker2[thread_num];
    int cpu_num;

    map<string, string> myAddrPair;
    int map_counter = 0;
    
    if (argc != 2) {
        printf("Usage: ./traverse7 [DIR] \n"); return 0;
    }
    cpu_num = sysconf(_SC_NPROCESSORS_CONF);

    initqueue(&q);

    for (i = 0; i < thread_num; ++i) {
        targ[i].q = &q;
        targ[i].dirname = argv[1];
        targ[i].filenum = 0;
        targ[i].cpuid = i%cpu_num;
    }
    result.fname = NULL;

    start_timer(&t);

    pthread_mutex_init(&result.mutex, NULL);

    /* first scatter */
    
    pthread_create(&master, NULL, (void*)master_func, (void*)&targ[0]);
    for (i = 1; i < thread_num; ++i)
      { 
        targ[i].id = i;
        pthread_create(&worker[i], NULL, (void*)worker_func, (void*)&targ[i]);
      }

    for (i = 1; i < thread_num; ++i) 
        pthread_join(worker[i], NULL);

    /* second scatter */
    
    pthread_create(&master, NULL, (void*)master_func, (void*)&targ[0]);
    for (i = 1; i < thread_num; ++i)
      { 
        targ[i].id = i;
        pthread_create(&worker2[i], NULL, (void*)worker_func2, (void*)&targ[i]);
      }

    for (i = 1; i < thread_num; ++i) 
        pthread_join(worker2[i], NULL);

    map<int, int>::iterator itr2;
    std::vector<int> vcount;
    std::vector<int> vtlen;
    std::vector<int> vttl;
    std::vector<int> vsport;
    std::vector<int> vdport;       

    for (itr2 = reduced.count.begin(); itr2 != reduced.count.end(); itr2++)
      {
	vcount.push_back(itr2->second);
	// counter = counter + 1;
      }

    for (itr2 = reduced.tlen.begin(); itr2 != reduced.tlen.end(); itr2++)
      {
	vtlen.push_back(itr2->second);
	// counter = counter + 1;
      }

    for (itr2 = reduced.ttl.begin(); itr2 != reduced.ttl.end(); itr2++)
      {
	vttl.push_back(itr2->second);
	// counter = counter + 1;
      }

    for (itr2 = reduced.sport.begin(); itr2 != reduced.sport.end(); itr2++)
      {
	vsport.push_back(itr2->second);
	// counter = counter + 1;
      }

    for (itr2 = reduced.dport.begin(); itr2 != reduced.dport.end(); itr2++)
      {
	vdport.push_back(itr2->second);
	// counter = counter + 1;
      }               
    
    pthread_mutex_lock(&result.mutex);
    myAddrPair = addrpair.m;
    pthread_mutex_unlock(&result.mutex);      

    pthread_mutex_lock(&addrpair.mutex);  

    ofstream outputfile("reduced"); 
    
    map<string, string>::iterator itr;
    map_counter = 0;
    for (itr = myAddrPair.begin(); itr != myAddrPair.end(); itr++)
      {
	/*
	printf("%s,%s,", itr->first.c_str(), itr->second.c_str());
	*/

	std::cout << itr->first.c_str() << "," << itr->second.c_str() << "," << vcount[map_counter] << "," << vtlen[map_counter] << "," << vttl[map_counter] << "," << vsport[map_counter] << "," << vdport[map_counter] << std::endl;
	
	outputfile << itr->first.c_str() << "," << itr->second.c_str() << "," << vcount[map_counter] << "," << vtlen[map_counter] << "," << vttl[map_counter] << "," << vsport[map_counter] << "," << vdport[map_counter] << std::endl;
	
	map_counter++;
      }

    outputfile.close(); 
    pthread_mutex_unlock(&addrpair.mutex);  
    
    printf("map_counter:%d\n", map_counter);
    
    travdirtime = stop_timer(&t);
    print_timer(travdirtime);

    print_result(&targ[0]);
    for (i = 1; i < thread_num; ++i) {
        if ((targ[i].q)->fname[i] != NULL) free((targ[i].q)->fname[i]);
    }
    if(result.fname != NULL) free(result.fname);
    
    return 0;
}
