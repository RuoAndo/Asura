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
#include <bitset>

#include <cstdio>
#include <cctype>
#include <iostream>

#include "tbb/task_scheduler_init.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include <tbb/concurrent_vector.h>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/generate.h>
#include <thrust/sort.h>
#include <thrust/copy.h>
#include <algorithm>
#include <cstdlib>
#include "timer.h" 

using namespace std;
using namespace tbb;

#define N 100
#define WORKER_THREAD_NUM_PHASE1 80
#define WORKER_THREAD_NUM_PHASE2 3

#define MAX_QUEUE_NUM N
#define END_MARK_FNAME   "///"
#define END_MARK_FLENGTH 3

#define DISP_FREQ 100000

static long all_packet_counts;

/* srcIP, destIP */
typedef struct _addrpair {
  map<string, string> m;
  pthread_mutex_t mutex;
} addrpair_t;
addrpair_t addrpair;

struct HashCompare {
  static size_t hash( unsigned long long x ) {
    return (size_t)x;
    }
  static bool equal( unsigned long long x, unsigned long long y ) {
        return x==y;
    }
};

extern void transfer(unsigned long long *key, long *value, unsigned long long *key_out, long *value_out, int kBytes, int vBytes, size_t data_size, int* new_size, int thread_id);    

typedef tbb::concurrent_vector<unsigned long long> iTbb_Vec1;
typedef tbb::concurrent_vector<long> iTbb_Vec2;

iTbb_Vec1 TbbVec1_thread_1;
iTbb_Vec2 TbbVec2_thread_1;

iTbb_Vec1 TbbVec1_thread_2;
iTbb_Vec2 TbbVec2_thread_2;

iTbb_Vec1 TbbVec1_thread_3;
iTbb_Vec2 TbbVec2_thread_3;

typedef tbb::concurrent_hash_map<unsigned long, int, HashCompare> iTbb_Vec_bytes;
static iTbb_Vec_bytes TbbVec_bytes; 

typedef tbb::concurrent_hash_map<unsigned long, int, HashCompare> iTbb_Vec_counts;
static iTbb_Vec_counts TbbVec_counts;

typedef tbb::concurrent_hash_map<unsigned long, std::vector<int>> iTbb_Vec_counts_thread_1;
static iTbb_Vec_counts_thread_1 TbbVec_counts_thread_1;

typedef tbb::concurrent_hash_map<unsigned long, std::vector<int>> iTbb_Vec_counts_thread_2;
static iTbb_Vec_counts_thread_2 TbbVec_counts_thread_2;

typedef tbb::concurrent_hash_map<unsigned long, std::vector<int>> iTbb_Vec_counts_thread_3;
static iTbb_Vec_counts_thread_3 TbbVec_counts_thread_3;

typedef tbb::concurrent_hash_map<unsigned long, std::vector<int>> iTbb_Vec_counts_thread_4;
static iTbb_Vec_counts_thread_4 TbbVec_counts_thread_4; 

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

std::vector<std::string> split_string_2(std::string str, char del) {
  int first = 0;
  int last = str.find_first_of(del);

  std::vector<std::string> result;

  while (first < str.size()) {
    std::string subStr(str, first, last - first);

    result.push_back(subStr);

    first = last + 1;
    last = str.find_first_of(del, first);

    if (last == std::string::npos) {
      last = str.size();
    }
  }

  return result;
}

int ProcIpHeader(struct iphdr *iphdr,u_char *option,int optionLen,FILE *fp,u_char*ptr,int thread_id)
{
  int i;
  char buf[80];
  
  char src[80];
  char dst[80];

  string saddr;
  string daddr;

  int tlen;

  struct tcphdr *tcp;
  struct udphdr *udp;
  
  /*
  fprintf(fp,"protocol=%u, ",iphdr->protocol);
  fprintf(fp,"saddr=%s,",ip_ip2str(iphdr->saddr,buf,sizeof(buf)));
  fprintf(fp,"daddr=%s\n",ip_ip2str(iphdr->daddr,buf,sizeof(buf)));
  fprintf(fp,"length=%u\n",ntohs(iphdr->tot_len));
 */

  sprintf(src, ip_ip2str(iphdr->saddr,buf,sizeof(buf)));
  sprintf(dst, ip_ip2str(iphdr->daddr,buf,sizeof(buf)));

  tlen = (signed int)ntohs(iphdr->tot_len);
  
  saddr = string(src);
  daddr = string(dst);

  int sport = 0;
  int dport = 0;
  
  char del = '.';

  std::string stringIP;
  std::string IPstring;
	    	    
  stringIP = saddr;	    
  for (const auto subStr : split_string_2(stringIP, del)) {
    unsigned long ipaddr_src;
    ipaddr_src = atoi(subStr.c_str());
    std::bitset<8> trans =  std::bitset<8>(ipaddr_src);
    std::string trans_string = trans.to_string();
    IPstring = IPstring + trans_string;
  }
	    
  stringIP = daddr;
  for (const auto subStr : split_string_2(stringIP, del)) {
    unsigned long ipaddr_src;
    ipaddr_src = atoi(subStr.c_str());
    std::bitset<8> trans =  std::bitset<8>(ipaddr_src);
    std::string trans_string = trans.to_string();
    IPstring = IPstring + trans_string;
  }
	  
  unsigned long long n = bitset<64>(IPstring).to_ullong();

  if(thread_id % 3 == 1)
    {  
      TbbVec1_thread_1.push_back(n);
      TbbVec2_thread_1.push_back(tlen);

      iTbb_Vec_counts_thread_1::accessor cnt_thread_1;
      TbbVec_counts_thread_1.insert(cnt_thread_1, n);
      cnt_thread_1->second.push_back(tlen);    
    }

  if(thread_id %3 == 2)
    {  
      TbbVec1_thread_2.push_back(n);
      TbbVec2_thread_2.push_back(tlen);

      iTbb_Vec_counts_thread_2::accessor cnt_thread_2;
      TbbVec_counts_thread_2.insert(cnt_thread_2, n);
      cnt_thread_2->second.push_back(tlen);    
      
    }

  if(thread_id %3 == 0)
    {  
      TbbVec1_thread_3.push_back(n);
      TbbVec2_thread_3.push_back(tlen);
      
      iTbb_Vec_counts_thread_3::accessor cnt_thread_3;
      TbbVec_counts_thread_3.insert(cnt_thread_3, n);
      cnt_thread_3->second.push_back(tlen);    
      
    }
      
  if(iphdr->protocol == IPPROTO_TCP)
    {
      tcp = (struct tcphdr *) ptr;
      ptr += ((int) (tcp->th_off) << 2);   

      sport = ntohs(tcp->th_sport);
      dport = ntohs(tcp->th_dport);
    }
  
  return(0);
}

int AnalyzeIp(u_char *data, int size, int thread_id)
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
      // return(-1);
    }
    option=ptr;
    ptr+=optionLen;
    lest-=optionLen;
  }

  if(checkIPchecksum(iphdr,option,optionLen)==0){
    chksum++;
  }
  
  ProcIpHeader(iphdr,option,optionLen,stdout,ptr,thread_id);
  
  return(0);
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
    int disp_counter = 0;
	
    fp = fopen(filename, "rb");
    if(fp == NULL){
      printf("file cannot be opened. \n");
      return 0;
    }

    for(;;){
      
      if((data = getc(fp) ) == EOF){
	fclose(fp); return 1;}
      
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
		  //if(thread_id % WORKER_THREAD_NUM == 0)
		  if(counter % DISP_FREQ == 0)
		    printf("worker@1stPhase:threadID:%d:filename:%s IP 080045:counter:%d\n", thread_id, filename, counter);
		  
		  fseek(fp,-1.5L,SEEK_CUR);
		  if (fgets(buf, sizeof(struct iphdr)+8, fp) != NULL)
		    {
		      ptr = buf;
		      AnalyzeIp(ptr, sizeof(struct iphdr), thread_id);
		      counter = counter + 1;
		    }
		  fseek(fp,-3L,SEEK_CUR);
		}
	      
	    } // if(strcmp(tmp,s2)
	  disp_counter++;
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
    for (i = 0; i < WORKER_THREAD_NUM_PHASE1; ++i) 
        enqueue(q, END_MARK_FNAME, END_MARK_FLENGTH);
    return;
}

void worker_func(thread_arg_t* arg) {
    int flen;
    char* fname = NULL;
    queue_t* q = arg->q;
    char* srchstr = arg->srchstr;

    int thread_id = arg->id;

    printf("worker func %d launched \n", thread_id);
    
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


void worker_func_2(thread_arg_t* arg) {
    int flen;
    char* fname = NULL;
    queue_t* q = arg->q;
    char* srchstr = arg->srchstr;

    int thread_id = arg->id;
    int counter = 0;

    unsigned int t, travdirtime; 
    
    printf("worker func_2 %d launched \n", thread_id);

    if(thread_id == 1)
      {
	std::cout << "Thread1: TbbVec1 size:" << TbbVec1_thread_1.size() << endl;
	std::cout << "Thread1: TbbVec2 size:" << TbbVec2_thread_1.size() << endl;

	size_t kBytes = TbbVec1_thread_1.size() * sizeof(unsigned long long);
	unsigned long long *key;
	key = (unsigned long long *)malloc(kBytes);

	size_t vBytes = TbbVec1_thread_1.size() * sizeof(long);
	long *value;
	value = (long *)malloc(vBytes);

	unsigned long long *key_out;
	key_out = (unsigned long long *)malloc(kBytes);
    
	long *value_out;
	value_out = (long *)malloc(vBytes);

	int new_size = 0;
    	
	counter = 0;
	for(  iTbb_Vec_counts_thread_1::iterator i=TbbVec_counts_thread_1.begin(); i!=TbbVec_counts_thread_1.end(); ++i )
	  {
	      for(auto itr = i->second.begin(); itr != i->second.end(); ++itr) {
		key[counter] = (unsigned long long)i->first;
		value[counter] = (long)*itr;
		
		counter++;
	      }
	  }

	/*

	tbb::concurrent_vector<unsigned long long>::iterator start1;
	tbb::concurrent_vector<unsigned long long>::iterator end1 = TbbVec1_thread_1.end();

	tbb::concurrent_vector<long>::iterator start2;
	tbb::concurrent_vector<long>::iterator end2 = TbbVec2_thread_1.end();

	for(start1 = TbbVec1_thread_1.begin();start1 != end1;++start1)
	  {
	    unsigned long long s = (unsigned long long)*start1;
	  }

	counter = 0;
	start2 = TbbVec2_thread_1.begin();
	for(start1 = TbbVec1_thread_1.begin();start1 != end1;++start1)
	  {
	    unsigned long long s = (unsigned long long)*start1;
	    long v = (unsigned long long)*start2;
	    
	    key[counter] = s;
	    value[counter] = v;

	    counter++;
	    start2++;
	  }
	*/

	cout << "thread:" << thread_id << ":" << TbbVec1_thread_1.size() << " lines - read done." << endl;
	all_packet_counts = all_packet_counts +  TbbVec1_thread_1.size();
	
	start_timer(&t);
	cout << "transfer..." << endl;
	transfer(key, value, key_out, value_out, kBytes, vBytes, TbbVec1_thread_1.size(), &new_size, thread_id);
	cout << "done." << endl;
	travdirtime = stop_timer(&t);
	print_timer(travdirtime);
	
	for(int i = 0; i < new_size; i++)
	  {
	    iTbb_Vec_bytes::accessor tms;
	    TbbVec_bytes.insert(tms, key_out[i]);
	    tms->second += value_out[i];
	  }

	for(int i = 0; i < new_size; i++)
	  {
	    iTbb_Vec_counts::accessor tcnt;
	    TbbVec_counts.insert(tcnt, key_out[i]);
	    tcnt->second++;
	  }       
      }

    if(thread_id == 2)
      {
	std::cout << "Thread2: TbbVec1 size:" << TbbVec1_thread_2.size() << endl;
	std::cout << "Thread2: TbbVec2 size:" << TbbVec2_thread_2.size() << endl;

	size_t kBytes = TbbVec1_thread_2.size() * sizeof(unsigned long long);
	unsigned long long *key;
	key = (unsigned long long *)malloc(kBytes);

	size_t vBytes = TbbVec1_thread_2.size() * sizeof(long);
	long *value;
	value = (long *)malloc(vBytes);

	unsigned long long *key_out;
	key_out = (unsigned long long *)malloc(kBytes);
    
	long *value_out;
	value_out = (long *)malloc(vBytes);

	int new_size = 0;

	counter = 0;
	for(  iTbb_Vec_counts_thread_2::iterator i=TbbVec_counts_thread_2.begin(); i!=TbbVec_counts_thread_2.end(); ++i )
	  {
	      for(auto itr = i->second.begin(); itr != i->second.end(); ++itr) {
		key[counter] = (unsigned long long)i->first;
		value[counter] = (long)*itr;
		
		counter++;
	      }
	  }

	/*
	tbb::concurrent_vector<unsigned long long>::iterator start1;
	tbb::concurrent_vector<unsigned long long>::iterator end1 = TbbVec1_thread_2.end();

	tbb::concurrent_vector<long>::iterator start2;
	tbb::concurrent_vector<long>::iterator end2 = TbbVec2_thread_2.end();

	for(start1 = TbbVec1_thread_2.begin();start1 != end1;++start1)
	  {
	    unsigned long long s = (unsigned long long)*start1;
	  }

	counter = 0;
	start2 = TbbVec2_thread_2.begin();
	for(start1 = TbbVec1_thread_2.begin();start1 != end1;++start1)
	  {
	    unsigned long long s = (unsigned long long)*start1;
	    long t = (unsigned long long)*start2;
	    
	    key[counter] = s;
	    value[counter] = t;

	    counter++;
	    start2++;
	  }
	*/

	cout << "thread:" << thread_id << ":" << TbbVec1_thread_2.size() << " lines - read done." << endl;
	all_packet_counts = all_packet_counts +  TbbVec1_thread_2.size();
	
	start_timer(&t);
	cout << "transfer..." << endl;
	transfer(key, value, key_out, value_out, kBytes, vBytes, TbbVec1_thread_2.size(), &new_size, thread_id);
	cout << "done." << endl;
	travdirtime = stop_timer(&t);
	print_timer(travdirtime);
	
	for(int i = 0; i < new_size; i++)
	  {
	    iTbb_Vec_bytes::accessor tms;
	    TbbVec_bytes.insert(tms, key_out[i]);
	    tms->second += value_out[i];
	  }
	
	for(int i = 0; i < new_size; i++)
	  {
	    iTbb_Vec_counts::accessor tcnt;
	    TbbVec_counts.insert(tcnt, key_out[i]);
	    tcnt->second++;
	  }       
	
      }
    
    if(thread_id == 3)
      {
	std::cout << "Thread3: TbbVec1 size:" << TbbVec1_thread_3.size() << endl;
	std::cout << "Thread3: TbbVec2 size:" << TbbVec2_thread_3.size() << endl;

	size_t kBytes = TbbVec1_thread_3.size() * sizeof(unsigned long long);
	unsigned long long *key;
	key = (unsigned long long *)malloc(kBytes);

	size_t vBytes = TbbVec1_thread_3.size() * sizeof(long);
	long *value;
	value = (long *)malloc(vBytes);

	unsigned long long *key_out;
	key_out = (unsigned long long *)malloc(kBytes);
    
	long *value_out;
	value_out = (long *)malloc(vBytes);

	int new_size = 0;

	counter = 0;
	for(  iTbb_Vec_counts_thread_3::iterator i=TbbVec_counts_thread_3.begin(); i!=TbbVec_counts_thread_3.end(); ++i )
	  {
	      for(auto itr = i->second.begin(); itr != i->second.end(); ++itr) {
		key[counter] = (unsigned long long)i->first;
		value[counter] = (long)*itr;
		
		counter++;
	      }
	  }

	/*
	tbb::concurrent_vector<unsigned long long>::iterator start1;
	tbb::concurrent_vector<unsigned long long>::iterator end1 = TbbVec1_thread_3.end();

	tbb::concurrent_vector<long>::iterator start2;
	tbb::concurrent_vector<long>::iterator end2 = TbbVec2_thread_3.end();

	for(start1 = TbbVec1_thread_3.begin();start1 != end1;++start1)
	  {
	    unsigned long long s = (unsigned long long)*start1;
	  }

	counter = 0;
	start2 = TbbVec2_thread_3.begin();
	for(start1 = TbbVec1_thread_3.begin();start1 != end1;++start1)
	  {
	    unsigned long long s = (unsigned long long)*start1;
	    long t = (unsigned long long)*start2;
	    
	    key[counter] = s;
	    value[counter] = t;

	    counter++;
	    start2++;
	  }
	*/

	cout << "thread:" << thread_id << ":" << TbbVec1_thread_3.size() << " lines - read done." << endl;
	all_packet_counts = all_packet_counts +  TbbVec1_thread_3.size();
	  
	start_timer(&t);
	cout << "transfer..." << endl;
	transfer(key, value, key_out, value_out, kBytes, vBytes, TbbVec1_thread_3.size(), &new_size, thread_id);
	cout << "done." << endl;
	travdirtime = stop_timer(&t);
	print_timer(travdirtime);
	
	for(int i = 0; i < new_size; i++)
	  {
	    iTbb_Vec_bytes::accessor tms;
	    TbbVec_bytes.insert(tms, key_out[i]);
	    tms->second += value_out[i];
	  }
	
	for(int i = 0; i < new_size; i++)
	  {
	    iTbb_Vec_counts::accessor tcnt;
	    TbbVec_counts.insert(tcnt, key_out[i]);
	    tcnt->second++;
	  }       
      }

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
    unsigned int t, travdirtime;
    queue_t q;

    int thread_num = 1 + WORKER_THREAD_NUM_PHASE1;
    thread_arg_t targ[thread_num];
    int cpu_num;

    int counter = 0;

    struct in_addr inaddr;
    char *some_addr;
    
    if (argc != 2) {
        printf("Usage: ./asura [DIR] \n"); return 0;
    }
    cpu_num = sysconf(_SC_NPROCESSORS_CONF);

    initqueue(&q);

    start_timer(&t);

    /* first stage */
    pthread_t master;
    pthread_t worker[thread_num];

    for (i = 0; i < thread_num; ++i) {
        targ[i].q = &q;
        targ[i].dirname = argv[1];
        targ[i].filenum = 0;
        targ[i].cpuid = i%cpu_num;
	targ[i].id = i;
    }
    result.fname = NULL;
    pthread_mutex_init(&result.mutex, NULL);
    
    pthread_create(&master, NULL, (void*)master_func, (void*)&targ[0]);
    for (i = 1; i < thread_num; ++i)
      { 
        targ[i].id = i;
        pthread_create(&worker[i], NULL, (void*)worker_func, (void*)&targ[i]);
      }
    for (i = 1; i < thread_num; ++i) 
        pthread_join(worker[i], NULL);

    /* second stage */
    thread_num = 1 + WORKER_THREAD_NUM_PHASE2;
    pthread_t worker2[thread_num];
    thread_arg_t targ2[thread_num];

    cout << "second stage: # of threads:" <<  WORKER_THREAD_NUM_PHASE2 << endl;
    
    for (i = 0; i < thread_num; ++i) {
        targ2[i].q = &q;
        targ2[i].dirname = argv[1];
        targ2[i].filenum = 0;
        targ2[i].cpuid = i%cpu_num;
	targ2[i].id = i;
    }
    
    for (i = 1; i < thread_num; ++i)
      { 
        targ[i].id = i;
        pthread_create(&worker2[i], NULL, (void*)worker_func_2, (void*)&targ2[i]);
      }
    for (i = 1; i < thread_num; ++i) 
        pthread_join(worker2[i], NULL);

    cout << "all - done." << endl;

    std::remove("tmp-asura-1");
    ofstream outputfile1("tmp-asura-1");
    
    counter = 0;
    cout << "TbbVec_bytes size: " << TbbVec_bytes.size() << endl;
    
    for(  iTbb_Vec_bytes::iterator i=TbbVec_bytes.begin(); i!=TbbVec_bytes.end(); ++i )
      {

	bitset<64> addr((unsigned long long)i->first);
	std::string addr_string = addr.to_string();
	string addr_src = addr_string.substr(0,32);
	string addr_dst = addr_string.substr(32,32);

	bitset<32> bs(addr_src);
	bitset<32> ds(addr_dst);
	unsigned long long int s = bs.to_ullong();
	unsigned long long int d = ds.to_ullong();

	inaddr = { htonl(s) };
	some_addr = inet_ntoa(inaddr);
	string src_string = string(some_addr);   
	
	inaddr = { htonl(d) };
	some_addr = inet_ntoa(inaddr);
	string dst_string = string(some_addr);

	outputfile1 << src_string << "," << dst_string << "," << (long)i->second << endl; 

	counter = counter + 1;
      }

    outputfile1.close();

    std::remove("tmp-asura-2");
    ofstream outputfile2("tmp-asura-2");

    cout << "TbbVec_counts size: " << TbbVec_counts.size() << endl;
    
    counter = 0;
    for(  iTbb_Vec_counts::iterator i=TbbVec_counts.begin(); i!=TbbVec_counts.end(); ++i )
      {

	bitset<64> addr((unsigned long long)i->first);
	std::string addr_string = addr.to_string();
	string addr_src = addr_string.substr(0,32);
	string addr_dst = addr_string.substr(32,32);

	bitset<32> bs(addr_src);
	bitset<32> ds(addr_dst);
	unsigned long long int s = bs.to_ullong();
	unsigned long long int d = ds.to_ullong();

	inaddr = { htonl(s) };
	some_addr = inet_ntoa(inaddr);
	string src_string = string(some_addr);   
	
	inaddr = { htonl(d) };
	some_addr = inet_ntoa(inaddr);
	string dst_string = string(some_addr);

	outputfile2 << src_string << "," << dst_string << "," << (long)i->second << endl; 

	if(counter < 10)
	  cout << src_string << "," << dst_string << "," << (long)i->second << endl; 
    
	counter = counter + 1;
      }

    outputfile2.close();

    cout << "# of all packets processed: " << all_packet_counts << endl;
    
    return 0;
}
