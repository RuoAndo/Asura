2018.08 Currently, Asura can process 76,835,550 packets in 200-400 minutes.<br>
2019.03 Currently, Asura can process 3,959,137,279 packets in 851m24.777s.<br>
2019.07 Currently, Asuca 21 can process 465,913,560 (about 83G) packets in 41m4.667s with 90 CPU cores and 4 GPUs.

PLAN1(2018.10-2018.11): Cocurrent hash map (Intel TBB) is going to be adopted.
<br>
PLAN2(2019.02-2019.03): NVidia Cuda Thrust is going to be adopted.
<br>
PLAN3(2019.04-): Multi-GPUs are going to be adopted.
<br>
PLAN4(2019.08-): NVLink is going to be adopted.

<b>DEFCON 26 Demo</b>

<img src="asura.png" width=200 height=200>

https://github.com/RuoAndo/Asura/tree/master/demo-defcon
<br><br>

<b>Asura 0 (DEFCON 26)</b>

<img src="asura0.jpeg" width=200 height=200>

A huge PCAP file analyzer for anomaly packets detection using massive multithreading (DEFCON 26)<br>
https://github.com/RuoAndo/Asura/tree/master/0
<br><br>

<b>Asura 1 (with Intel TBB)</b>

<img src="asura1.jpeg" width=200 height=200>

A portable anomaly detector for 100GB PCAP file in laptop using highly concurrent container<br>
https://github.com/RuoAndo/Asura/tree/master/1
<br><br>

<b>Asura 17 (with Intel TBB and CUDA)</b>

<img src="asura17.jpeg" width=200 height=200>

A GPU acclerated packets cluctering using highly concurrent container<br> 
https://github.com/RuoAndo/Asura/tree/master/17
<br><br>

<pre>
0: original - DEFCON 26
1: concurrent hash map (Intel TBB)
17: concurrent vector (Intel TBB) and CUDA Thrust
</pre>
