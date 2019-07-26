Asura 0 with STL (Standard Template Library)

<img src="../asura0.jpeg" width=200 height=200>

Recently, the inspection of huge traffic log is imposing a great burden on security analysts. Unfortunately, there have been few research efforts focusing on scalablility in analyzing very large PCAP file with reasonable computing resources. Asura is a portable and scalable PCAP file analyzer for detecting anomaly packets using massive multithreading. Asura's parallel packet dump inspection is based on task-based decomposition and therefore can handle massive threads for large PCAP file without considering tidy parameter selection in adopting data decomposition. Asura is designed to scale out in processing large PCAP file by taking as many threads as possible. 

Asura takes two steps. First, Asura extracts feature vector represented by associative containers of <sourceIP, destIP> pair. By doing this, the feature vector can be drastically small compared with the size of original PCAP files. In other words, Asura can reduce packet dump data into the size of unique <sourceIP, destIP> pairs (for example, in experiment, Asura's output which is reduced in first step is about 2% compared with the size of original libpcap files). Second, a parallel clustering algorithm is applied for the feature vector which is represented as {<sourceIP, destIP>, V[i]} where V[i] is aggregated flow vector. In second step, Asura adopts an enhanced Kmeans algorithm. Concretely, two functions of Kmeans which are (1)calculating distance and (2)relabeling points are improved for parallel processing. 

In experiment, in processing public PCAP datasets, Asura can identified 750 packets which are labeled as malicious from among 70 million (about 18GB) normal packets. In a nutshell, Asura successfully found 750 malicious packets in about 18GB packet dump. For Asura to inspect 70 million packets, it took reasonable computing time of around 350-450 minutes with 1000-5000 multithreading by running commodity workstation. Asura will be released under MIT license and available at author's GitHub site on the first day of DEF CON 26.

1.Lisence: Asura One is now released as open source under MIT license.

2.Compiler and libraries: gcc version 7.3.0 (Ubuntu 7.3.0), Posix Pthreads.

3.Input: PCAP files in the directory 

4.Usage: ./asura DIRECTORY_NAME

<pre>
	# mkdir pcap
        # cd pcap 
        # wget https://download.netresec.com/pcap/maccdc-2012/maccdc2012_*.pcap.gz
        # cd ..
	# ./build-asura.sh 
        # ./asura pcap
</pre>

5.Output: {<sourceIP, destinationIP>, Anomaly_Score}

<pre>
	sourceIP,destIP->clusterNo (centroids coords: length, counts), clusterSize, AllSize, Anomaly_score(%)
	X.X.X.X,Y.Y.Y.Y -> 7 (275912,48),46,118644,0.0387715%
　　　   X.X.X.X,Z.Z.Z.Z -> 5 (73445,48),288,118644,0.242743%
</pre>
　
6.Procedure 1: extracting flow vector {<srcIP, dstIP>, X, Y}
<pre>
Container: 
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
</pre>

7.Procedure 2: calculating anomaly score {<srcIP, dstIP>, Anomaly_Score}
<pre>
	  Main loop (K-Means):
	  tbb::parallel_for(
          tbb::blocked_range<size_t>(0,n),
          [=,&tls,&global]( tbb::blocked_range<size_t> r ) {
              view& v = tls.local();
              for( size_t i=r.begin(); i!=r.end(); ++i ) {
                  cluster_id j = calc_shortest_index(centroid, k , points[i]); 
                  if( j!=id[i] ) {
                      id[i] = j;
                      ++v.change;
                  }
                  v.array[j].tally(points[i]);
              }
            }
</pre>
