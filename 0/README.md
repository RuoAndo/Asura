README is going to be relased on 2019.04.27.

<img src="../asura0.jpeg" width=200 height=200>

Recently, the inspection of huge traffic log is imposing a great burden on security analysts. Unfortunately, there have been few research efforts focusing on scalablility in analyzing very large PCAP file with reasonable computing resources. Asura is a portable and scalable PCAP file analyzer for detecting anomaly packets using massive multithreading. Asura's parallel packet dump inspection is based on task-based decomposition and therefore can handle massive threads for large PCAP file without considering tidy parameter selection in adopting data decomposition. Asura is designed to scale out in processing large PCAP file by taking as many threads as possible. 

Asura takes two steps. First, Asura extracts feature vector represented by associative containers of <sourceIP, destIP> pair. By doing this, the feature vector can be drastically small compared with the size of original PCAP files. In other words, Asura can reduce packet dump data into the size of unique <sourceIP, destIP> pairs (for example, in experiment, Asura's output which is reduced in first step is about 2% compared with the size of original libpcap files). Second, a parallel clustering algorithm is applied for the feature vector which is represented as {<sourceIP, destIP>, V[i]} where V[i] is aggregated flow vector. In second step, Asura adopts an enhanced Kmeans algorithm. Concretely, two functions of Kmeans which are (1)calculating distance and (2)relabeling points are improved for parallel processing. 

In experiment, in processing public PCAP datasets, Asura can identified 750 packets which are labeled as malicious from among 70 million (about 18GB) normal packets. In a nutshell, Asura successfully found 750 malicious packets in about 18GB packet dump. For Asura to inspect 70 million packets, it took reasonable computing time of around 350-450 minutes with 1000-5000 multithreading by running commodity workstation. Asura will be released under MIT license and available at author's GitHub site on the first day of DEF CON 26.

1.Lisence: Asura One is now released as open source under MIT license.

2.Compiler and libraries: gcc version 7.3.0 (Ubuntu 7.3.0), Posix Pthreads and Intel TBB.
