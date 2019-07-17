Asura 17: A GPU acclerated packets cluctering using highly concurrent container<br> 

<img src="asura17.jpeg" width=200 height=200>

<pre>
The rapid increase of security log has been imposing a great burden on security analysts and researchers. Particularly, in the recent several years, we are facing more and more the situation in which we cope with terabyte-scale traffic dump for various purposes such as network forensics, incident response and CTF. Unfortunately, the emergence of terabyte-scale PCAP file has nullified the effectiveness of the most of good old arts of concurrency. Many conventional techniques such as concurrent hashmap, atomic instruction and lock-based mechanism implemented on SIMD architecture are not so practical any longer in the struggle against terabyte-scale PCAP file.

  Asura 17 has a deep impact of "SMIT after SMID" parallelization over the anomaly detection of terabyte-scale traffic dump is reported. The good combination of SMIT and SMID parallelization outperforms the conventional art of concurrency, which enables us process a few terabyte PCAP file with the reasonable computing time.  For example, Asura can execute the anomaly detection of about 3,000,000,000 packets in around 800-1000 minutes (which is feasible for daily cron operation). Asura has two transformation combos (Pthreads→TBB, TBB→Thrust) for achieving drastically faster flow aggregation, feature vector extraction and clustering. SM
</pre>

1.Lisence: Asura 17 is now released as open source under MIT license. 

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

