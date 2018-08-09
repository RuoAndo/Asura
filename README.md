# Asura: A huge PCAP file analyzer with massive multithreading

# Quick test
<pre>
# time  ./auto.sh 100 7 10 5 dns
real    5m13.159s
user    0m22.096s
sys     0m45.072s
</pre>

Asura generates five files.

* iplist sorted by anomaly rate (iplist-[machine name]-date)
iplist-flare-Precision-Tower-7910-20180602
iplist-flare-Precision-Tower-7910-20180602-all-192-percent

* clutering results
count-result
result-all

* elapsed time
procTime

<pre>
# more count-percent-flare-Precision-Tower-7910-20180602
CLUSTER0,3,(3.0%)
CLUSTER1,2,(2.0%)
CLUSTER2,2,(2.0%)
CLUSTER3,2,(2.0%)
CLUSTER4,1,(1.0%)
CLUSTER5,2,(2.0%)
CLUSTER6,7,(7.0%)
CLUSTER7,57,(57.0%)
CLUSTER8,19,(19.0%)
CLUSTER9,5,(5.0%)

# more iplist-flare-Precision-Tower-7910-20180602-all-192-percent
192.168.205.253,1.0,(%)
172.16.16.128,(1.0%)
192.168.202.69,(2.0%)
192.168.202.91,(2.0%)
192.168.202.83,(2.0%)
192.168.203.45,(2.0%)
192.168.202.109,(2.0%)
192.168.202.100,(2.0%)
192.168.202.73,(2.0%)
192.168.206.44,(2.0%)
192.168.202.79,(2.0%)
192.168.202.74,(2.0%)
192.168.202.68,(4.0%)
192.168.22.100,(4.0%)
192.168.21.100,(4.0%)
192.168.202.76,(4.0%)
192.168.213.1,(25.0%)
192.168.211.1,(25.0%)

# head -n 5 result-all
6,2130706433,3232242685,2,168,256,0
3,2886733952,2886733974,37,3482,1817,1672439
6,2886733974,2886733952,11,822,1408,6437
6,2886926914,2886927103,9,2121,1152,0

# more count-result
CLUSTER0,2
CLUSTER1,4
CLUSTER2,2
CLUSTER3,1
CLUSTER4,2
CLUSTER5,25
CLUSTER6,59
CLUSTER7,2
CLUSTER8,1
CLUSTER9,2

# more procTime
started at:20180602-191836
finished at:20180602-192049
proc time:133sec
proc time:2min
</pre>

# test

<pre>
# mkdir packets
# cd packets

# wget http://www.chrissanders.org/captures/aurora.pcap
# wget https://download.netresec.com/pcap/maccdc-2012/maccdc2012_00000.pcap.gz
# gunzip maccdc2012_00000.pcap.gz
# wget https://github.com/kbandla/pcaps/blob/master/malicious/scanning/activeosfingerprinting.pcap

# cd ..
# ./asura.sh 10 7 10 5 dns
</pre># Asura
