Asura 1 adopting concurrent hash map of Intel TBB.
This version is faster than verion 0 (DEFCON26) which uses mutex. 

<pre>
PLAN1: implementing second scatter in version 0. / 2019.01 - 2019.02
PLAN2: adopting bitset<> for faster reduction. / 2019.01 - 2019.02
</pre>

usage:

creating directory.
<pre>
# mkdir pcap
# cd pcap
</pre>

download pcap file. For example, 
<pre>
# wget https://download.netresec.com/pcap/maccdc-2012/maccdc2012_*****.pcap.gz
</pre>

execute.
<pre>
# cd ..
# make
# ./asura ./pcap
</pre>