2018.08 Currently, Asura can process 76,835,550 packets in 200-400 minutes.<br>
2019.03 Currently, Asura can process 3,959,137,279 packets in 851m24.777s.

<img src="asura.png" width=200 height=200>

PLAN1: Cocurrent hash map (Intel TBB) is going to be adopted: 2018.10-2018.11
2019.01.22: directory "1", "2" and "3" newly added which is going to be adopt concurrent hashmap (lock-free) of Intel TBB. 
<br>
PLAN2: NVidia Cuda Thrust is going to be adopted: 2019.02-2019.03
2019.02.14: directory "4" newly added which is work with Cuda Thrust.

<pre>
0: original - DEFCON 26
1: stl wrapped by mutex
2: hash table by Intel TBB
3: stl wrapped by mutex
4: Broken
5: Broken
6: Broken
7: concurrent hash map with hashcompare()
8: concurrent hash map -> thrust::host_vector()
9: Broken
10: Broken
11: concurrent hash map with hashcompare()
12: stl wrapped by mutex
13: stl wrapped by mutex
14: concurrent vector without hashcompare()
15: 
16: 
17: concurrent vector without hashcompare() writing files
18:
19:
20: thrust without vector
</pre>

<img src="asura0.jpeg" width=200 height=200>

2019.04: Asura 0 is released (DEFCON 26)