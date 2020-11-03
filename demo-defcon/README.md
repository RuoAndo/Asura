# Demo at DEFCON 26

Memo is updated! 2020/11/03

Put 10 pcap files in pcap directory and run auto.sh (10 threads in this case).

<pre>
~/tmp/Asura/demo-defcon# mkdir pcap                                                                                                                                                          
~/tmp/Asura/demo-defcon# cd pcap/    

~/tmp/Asura/demo-defcon# wget https://download.netresec.com/pcap/maccdc-2012/maccdc2012_00000.pcap.gz                                                                                        
~/tmp/Asura/demo-defcon# gunzip maccdc2012_00000.pcap.gz 
~/tmp/Asura/demo-defcon# split -n 10 maccdc2012_00000.pcap 

~/tmp/Asura/demo-defcon# cd ..
~/tmp/Asura/demo-defcon# ls pcap/
xaa  xab  xac  xad  xae  xaf  xag  xah  xai  xaj

~/tmp/Asura/demo-defcon# time ./auto.sh 10 7 10 5 pcap
</pre>

And you will get..

<pre>

CLUSTER,0,SIZE,0
CLUSTER,1,SIZE,15
CLUSTER,2,SIZE,2
CLUSTER,3,SIZE,7
CLUSTER,4,SIZE,10
CLUSTER,5,SIZE,15
CLUSTER,6,SIZE,12
CLUSTER,7,SIZE,35
CLUSTER,8,SIZE,0
CLUSTER,9,SIZE,214
Name "main::OUT1" used only once: possible typo at ./sort-percent.pl line 10.
 
### RESULT ###
sourceIP, destIP, clusterSize, anomaly rate
192.168.21.100,192.168.204.45,2,0.64516%
192.168.21.25,192.168.204.45,2,0.64516%
192.168.202.94,192.168.24.100,7,2.25806%
192.168.21.103,192.168.202.102,7,2.25806%
192.168.26.100,192.168.202.81,7,2.25806%
192.168.27.100,192.168.202.81,7,2.25806%
192.168.28.100,192.168.202.81,7,2.25806%
192.168.28.25,192.168.202.81,7,2.25806%
192.168.28.254,192.168.202.76,7,2.25806%
192.168.202.102,192.168.22.203,10,3.22581%
192.168.202.90,192.168.21.101,10,3.22581%
192.168.203.45,192.168.204.45,10,3.22581%
192.168.22.102,192.168.204.45,10,3.22581%
192.168.22.103,192.168.202.102,10,3.22581%
192.168.23.202,192.168.204.45,10,3.22581%
192.168.23.203,192.168.204.45,10,3.22581%
192.168.23.253,192.168.204.45,10,3.22581%
192.168.26.103,192.168.202.102,10,3.22581%
192.168.26.152,192.168.204.45,10,3.22581%

real    0m19.591s
user    0m6.700s
sys     0m9.901s
finished at:20201103-175202
proc time:529sec
proc time:8min

real    8m49.169s
user    46m43.290s
sys     0m47.023s

</pre>

<HR>
  
↓ Below is obsolete...

Put any PCAP files in the directory:
<pre>
# mkdir demo
# mv *.pcap ./demo
</pre>

or
<pre>
# cd demo
# wget http://.../*.pcap
# cd ..
</pre>

(recommended) Publicly available PCAP files
<pre>
https://www.netresec.com/?page=MACCDC
https://github.com/chrissanders/packets
</pre>

PCAP files under the directory:
<pre>
# ls demo/
1.pcap 2.pcap 3.pcap
</pre>

run this command:
<pre>
# ./auto.sh
./auto.sh nThread nDimensions nClusters nItems DIR

# ./auto.sh 10 7 10 5 demo
</pre>

Output is like this. The low percentage means that it is worth inspecting in detail.
<pre>
sourceIP, destIP, clusterSize, anomaly rate
*.*.*.*,*.*.*.*,2,1.17647%
*.*.*.*,*.*.*.*,4,2.35294%
*.*.*.*,*.*.*.*,9,5.29412%
</pre>

Browse all results:
<pre>
# cat result
</pre>

Main loop is in asura.cpp
<pre>
WORKER_THREAD_NUM   37 asura.cpp        #define WORKER_THREAD_NUM N
MAX_QUEUE_NUM      38 asura.cpp        #define MAX_QUEUE_NUM N
END_MARK_FNAME     39 asura.cpp        #define END_MARK_FNAME   "///"
END_MARK_FLENGTH   40 asura.cpp        #define END_MARK_FLENGTH 3
_addrpair          43 asura.cpp        typedef struct _addrpair {
addrpair_t         46 asura.cpp        } addrpair_t;
_reduced           50 asura.cpp        typedef struct _reduced {
reduced_t          57 asura.cpp        } reduced_t;
pseudo_ip          63 asura.cpp        struct pseudo_ip{
pseudo_ip6_hdr     71 asura.cpp        struct pseudo_ip6_hdr{
checksum           80 asura.cpp        u_int16_t checksum(u_char *data,int len)
checksum2         110 asura.cpp        u_int16_t checksum2(u_char *data1,int len1,u_char *data2,int len2)
checkIPchecksum   159 asura.cpp        int checkIPchecksum(struct iphdr *iphdr,u_char *option,int optionLen)
checkIPDATAchecksum  183 asura.cpp        int checkIPDATAchecksum(struct iphdr *iphdr,unsigned char *data,int len)
checkIP6DATAchecksum  203 asura.cpp        int checkIP6DATAchecksum(struct ip6_hdr *ip,unsigned char *data,int len)
_result           224 asura.cpp        typedef struct _result {
result_t          228 asura.cpp        } result_t;
_queue            231 asura.cpp        typedef struct _queue {
queue_t           239 asura.cpp        } queue_t;
_thread_arg       241 asura.cpp        typedef struct _thread_arg {
thread_arg_t      248 asura.cpp        } thread_arg_t;
ip_ip2str         250 asura.cpp        char *ip_ip2str(u_int32_t ip,char *buf,socklen_t size)
ProcIpHeader      260 asura.cpp        int ProcIpHeader(struct iphdr *iphdr,u_char *option,int optionLen,FILE *fp)
AnalyzeIp         291 asura.cpp        int AnalyzeIp(u_char *data,int size)
PrintTcp          336 asura.cpp        int PrintTcp(struct tcphdr *tcphdr,FILE *fp)
AnalyzeTcp        358 asura.cpp        int AnalyzeTcp(u_char *data,int size)
tcp_ftoa          381 asura.cpp        char *tcp_ftoa(int flag)
TCP_FLG_MAX       385 asura.cpp        #define TCP_FLG_MAX (sizeof f / sizeof f[0])
AnalyzeIp2        403 asura.cpp        int AnalyzeIp2(u_char *data,int size)
traverse_buffer2  556 asura.cpp        int traverse_buffer2(char* buf, int thread_id, char* filename)
traverse_file     574 asura.cpp        int traverse_file(char* filename, char* srchstr, int thread_id) {
traverse_file2    649 asura.cpp        int traverse_file2(char* filename, char* srchstr, int thread_id) {
initqueue         723 asura.cpp        void initqueue(queue_t* q) {
enqueue           733 asura.cpp        void enqueue(queue_t* q, char* path, int size) {
dequeue           753 asura.cpp        void dequeue(queue_t* q, char** fname, int* flen) {
traverse_dir_thread  771 asura.cpp        int traverse_dir_thread(queue_t* q, char* dirname) {
master_func       823 asura.cpp        void master_func(thread_arg_t* arg) {
worker_func       834 asura.cpp        void worker_func(thread_arg_t* arg) {
worker_func2      903 asura.cpp        void worker_func2(thread_arg_t* arg) {
print_result      953 asura.cpp        void print_result(thread_arg_t* arg) {
main              961 asura.cpp        int main(int argc, char* argv[]) {
</pre>
