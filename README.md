2018.08 Currently, Asura can process 76,835,550 packets in 200-400 minutes.<br>
2019.03 Currently, Asura can process 3,959,137,279 packets in 851m24.777s.

<img src="asura.png" width=200 height=200>

PLAN1(2018.10-2018.11): Cocurrent hash map (Intel TBB) is going to be adopted.
<br>
PLAN2(2019.02-2019.03): NVidia Cuda Thrust is going to be adopted.
<br>
PLAN3(2019.04-2019.06): NVLink is going to be adopted.

<b>2019.04.21: Asura 0 is newly released (DEFCON 26)</b>

<img src="asura0.jpeg" width=200 height=200>
https://github.com/RuoAndo/Asura/tree/master/0
<br><br>

<b>2019.04.26: Asura 1 is released </b>
<img src="asura1.jpeg" width=200 height=200>
A portable anomaly detector for 100GB PCAP file in laptop using highly concurrent container<br>
https://github.com/RuoAndo/Asura/tree/master/1
<br><br>

<pre>
0: original - DEFCON 26
1: concurrent hash map with hashcompare()
2: 
3: 
4: Broken
5: Broken
6: Broken
7: 
8: concurrent hash map -> thrust::host_vector()
9: Broken
10: Broken
11: 
12: 
13: 
14: 
15: 
16: 
17: concurrent vector without hashcompare() and CUDA Thrust
18:
19:
20: 
</pre>
