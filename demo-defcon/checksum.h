u_int16_t checksum(u_char *data,int len);
u_int16_t checksum2(u_char *data1,int len1,u_char *data2,int len2);
int checkIPchecksum(struct iphdr *iphdr,u_char *option,int optionLen);
int checkIPDATAchecksum(struct iphdr *iphdr,unsigned char *data,int len);
int checkIP6DATAchecksum(struct ip6_hdr *ip,unsigned char *data,int len);
