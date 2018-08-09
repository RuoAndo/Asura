<img src="asura.png" width=50 height=50>

# Quick test

Put any PCAP files in the directory:
<pre>
# mkdir demo
# mv *.pcap ./demo
# ls demo/
1.pcap 2.pcap 3.pcap
</pre>

or
<pre>
# cd demo
# wget http://.../*.pcap
# cd ..
</pre>

run this command:
<pre>
# ./auto.sh 10 7 10 5 demo
</pre>

Output is like this:
<pre>
sourceIP, destIP, clusterSize, anomaly rate
*.*.*.*,*.*.*.*,2,1.17647%
*.*.*.*,*.*.*.*,4,2.35294%
*.*.*.*,*.*.*.*,9,5.29412%
</pre>

Browse all results:
<pre>
# cat result
</pre>