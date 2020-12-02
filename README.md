“The universe is not complicated, there's just a lot of it.”  - Richard Feynman

2018.08 Currently, Asura can process 76,835,550 packets in 200-400 minutes.<br>
2019.03 Currently, Asura can process 3,959,137,279 packets in 851m24.777s.<br>
2019.07 Currently, Asuca 21 can process 465,913,560 (about 83G) packets in 41m4.667s with 20 CPU cores and 4 GPUs.
2019.08 Currently, Asuca 21 can process 1,050,676,809 (about 690G) packets in 297m36.604s with 20 CPU cores and 4 GPUs.

INFO:<br>
2020.11.03 defcon-demo/README.md is updated.<br>
2020.12.02 Asura23 leveraging Libtins are started.

PLAN1(2018.10-2018.11): Cocurrent hash map (Intel TBB) is going to be adopted.
<br>
PLAN2(2019.02-2019.03): NVidia Cuda Thrust is going to be adopted.
<br>
<strike>PLAN3(2019.04-): Multi-GPUs are going to be adopted.</strike>
<br>
<strike>PLAN4(2019.08-): NVLink is going to be adopted.</strike>
<br>
PLAN5(2020.12-): More accurate packet header parsing leveraging Libtins is going to be adopted.
<br>

<br>
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

<b>Asura 17 (with Single GPU)</b>

<img src="asura17.jpeg" width=200 height=200>

A GPU acclerated packets cluctering using highly concurrent container<br> 
https://github.com/RuoAndo/Asura/tree/master/17
<br><br>

<pre>
0: original - DEFCON 26
1: concurrent hash map (Intel TBB)
17: concurrent vector (Intel TBB) and CUDA Thrust
21: Multi-GPU
22: NVLink
</pre>
