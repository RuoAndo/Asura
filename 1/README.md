Asura One: A portable anomaly detector for 100GB PCAP file in laptop using highly concurrent container 

<img src="asura1.jpeg" width=200 height=200>

With the rapid increase of network traffic, we are suffering the long processing time in coping with large packet capture file in many aspects such as network diagnosis, incidents response and CTF. Wireshark and Linux grep command are basically run in single-process mode and are thus not good at analyzing tens or hundreds gigabytes of packet dump in reasonable processing time. On the other hand, high performance devices such as GPGPU are still bulky and expensive for running on our laptop.

Asura One is a portable anomaly detector of large packet capture file (around 100GB) leveraged by raw thread (Posix Pthreads) and highly concurrent container of Intel TBB (Threading Building Block). Asura One is designed for the lightweight anomaly detection of large PCAP file ranging from tens to hundreds million packets with the parallel clustering algorithm. Asura One takes two steps: extracting the flow vector {(srcIP,dstIP), X,Y..} and the clustering of which the outputs are {(srcIP,dstIP), Anomaly_Score}. The flow vector {(srcIP,dstIP), X,Y.. } is implemented as the concurrent hashmap of Intel TBB offering a much higher level of concurrency. Compared with the conventional C++ or Java hashmap such as STL, Intel TBB concurrent hashmap adopts lock-free techniques and is thus so faster. Another thrust of Asura One is leveraging task-decomposition by raw threads. Asura's task decomposition based multithreading can handle various types and lengths of IP packets and transforms these into well-organized input of {(srcIP,dstIP), X,Y..}. Asura One with its outputs of {(srcIP,dstIP), Anomaly_Score} should be a good partner (as pre-filter) of Wireshark. 

In experiment, Asura One has processed about 400,000,000 packets with 83GB PCAP file size in about 30 minutes on laptop PC. Packet dump was drastically reduced from 83GB to 7MB with about 110,000 aggregated flow vectors which are feasible for the further careful inspection by Wireshark.

1.Lisence: Asura One is now released as open source under MIT license. 

2.Compiler and libraries: gcc version 7.3.0 (Ubuntu 7.3.0), Posix Pthreads and Intel TBB.

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
	sourceIP,destIP->clusterNo (length, counts), clusterSize, AllSize, Anomaly_score(%)
	X.X.X.X,Y.Y.Y.Y -> 7 (275912,48),46,118644,0.0387715%
　　　   X.X.X.X,Z.Z.Z.Z -> 5 (73445,48),288,118644,0.242743%
</pre>
　
*Packet dump was drastically reduced from 83GB to 7MB with about 110,000 extracted flow vectors which are useful for the further careful inspection by Wireshark.

6.Procedure 1: extracting flow vector {<srcIP, dstIP>, X, Y}
<pre>
	Container: typedef concurrent_hash_map<unsigned long long, int, HashCompare>
</pre>

<pre>
	Main loop: 
  	pthread_create(&master, NULL, (void*)master_func, (void*)&targ[0]);
    	  for (i = 1; i < thread_num; ++i) { 
        	targ[i].id = i;
       		pthread_create(&worker[i],NULL,(void*)worker_func,(void*)&targ[i]); }
    	  for (i = 1; i < thread_num; ++i) 
 	       pthread_join(worker[i], NULL);
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

8. Processing time: about 30 minutes for about 400,000,000 packets

