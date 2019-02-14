Currently, Asura can process 76,835,550 packets in 200-400 minutes.

<img src="asura.png" width=200 height=200>
PLAN1: Cocurrent hash map (Intel TBB) is going to be adopted: 2018.10-2018.11
2019.01.22: directory "1", "2" and "3" newly added which is going to be adopt concurrent hashmap (lock-free) of Intel TBB. 

PLAN2: NVidia Cuda Thrust is going to be adopted: 2019.02-2019.03
2019.02.14: directory "4" newly added which is work with Cuda Thrust.

<pre>
1: stl wrapped by mutex
   pthread --- worker_func1
		|
		|--- traverse_file1
			|
			|--- analyseIP
				|
				|--- ProcIPHeader

2: hash table by Intel TBB

3: stl wrapped by mutex
   pthread ---- worker_func1
	    |	|
	    |	|--- traverse_file1
	    |		|
            |		|--- analyseIP
	    |			|
	    |			|--- ProcIPHeader
            |
	    |-- worker_func2
	        |
		|--- ...
</pre>

# Quick test

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