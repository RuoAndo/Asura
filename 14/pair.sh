DATE=`date --date '3 day ago' +%Y%m%d`
./build-gpu.sh reduce
./build-gpu.sh reduce2

echo $DATE
time nLines=`wc -l $1 | cut -d " " -f 1`
echo $nLines

rm -rf x*
split -l 100000000 $1

ls x* > list

rm -rf reduced-all
touch reduced-all

while read line; do
    echo $line
    time nLines=`wc -l $line | cut -d " " -f 1`
    echo $nLines
    comstr="./reduce $line $nLines"
    echo $comstr
    eval $comstr
    cat tmp-reduce >> reduced-all
done < list

nLines=`wc -l reduced-all | cut -d " " -f 1`
./reduce2 reduced-all $nLines

#time ./sort-pair.pl tmp-pair > tmp-pair-sorted
#cp tmp-pair-sorted tmp-pair-sorted-${DATE}
#scp tmp-pair-sorted-${DATE} 192.168.72.6:/mnt/sdc/splunk-ip-pair/
