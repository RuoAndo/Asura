Asura One

<img src="asura1.jpeg" width=200 height=200>

1.Lisence: Asura One is now released as Open Source under MIT license in github.com. 

2.Compiler and libraries: gcc version 7.3.0 (Ubuntu 7.3.0), Posix Pthreads and Intel TBB.

3.Input: PCAP files in the directory 

4.Usage: ./a.out DIRECTORY_NAME

<pre>
	# mkdir pcap
        # cd pcap 
        # wget https://download.netresec.com/pcap/maccdc-2012/maccdc2012_*.pcap.gz
        # cd ..
	# ./build-asura.sh 
        # ./a.out pcap
</pre>

5.Output: <sourceIP, destinationIP>, anomaly_rate

<pre>
	sourceIP,destIP->clusterNo (length, counts), clusterSize, AllSize, Anomaly_score(%)
	X.X.X.X,Y.Y.Y.Y -> 7 (275912,48),46,118644,0.0387715%
　　　	X.X.X.X,Z.Z.Z.Z -> 5 (73445,48),288,118644,0.242743%
</pre>
　
*Packet capture data is drastically reduced from 83GB to about 7MB with about 110,000 flow vectors which is worth for the further careful examination by Wireshark.

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

7.Procedure 2: calculating anomaly score with flow vector {<srcIP, dstIP>, Anomaly_Score}
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

8. Performance test

