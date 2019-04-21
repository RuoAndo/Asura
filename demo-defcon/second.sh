if [ "$6" = "" ]
then
    echo "argument required: ./second file nThreads nDimensions nClusters nItems TAG"
    exit
fi

allnLines=`wc -l $1 | cut -d " " -f 1`
echo "the number of lines: "$allnLines
nThreads=$2

nLines=`expr $allnLines / $2`
nDimensions=$3
nClusters=$4
nItems=$5 # nDimensions-2 / items: src dst n[* * *] 
TAG=$6

threshold=`expr $allnLines / 100`
echo "threshold: "$threshold

ssetail=10000000000000

while [ $ssetail -gt $threshold ]
do

echo "STEP4: [LABEL] concatenating label files ..." 
ls *.lbl > label_file_list
./sort.pl label_file_list > label_file_list_sorted
head -n $nThreads label_file_list_sorted > label_file_list.h
rm -rf all-labeled
touch all-labeled
./cat-labeled.sh label_file_list.h 
wc -l all-labeled # could be size of all data
sleep 2s

echo "STEP5: counting points per cluster..."
./count.re $nLines 1 | tee tmp-all-labeled 
sleep 2s

echo "STEP6: calculating centroid..."
rm -rf centroid
./avg.re $nLines $nDimensions #yields centroid
sleep 2s

echo "STEP7: filling blank centroid rows..."
python fill2.py tmp-all-labeled centroid $nLines $nDimensions > tmp-centroid

echo "##### modified centroid #####"
cat tmp-centroid
echo "##### #####"
\cp tmp-centroid centroid
sleep 4s

echo "STEP8: relabeling ..."
./relabel.re centroid $nClusters $nItems $nLines $nDimensions 

echo "STEP9: [LABEL] concatenating relabel files ..."
ls *.rlbl > list-relabeled
./sort.pl list-relabeled > list-relabeled-sorted
head -n $nThreads list-relabeled-sorted > list-relabeled.h
./cat-relabeled.sh list-relabeled.h # yields all-relabeled
wc -l all-relabeled # could be size of all data
sleep 2s

echo "STEP10: converting *.labeled to *.relabeled..."
./rename2.sh list-relabeled.h 

echo "STEP11: counting points per cluster..."
./count.re $nLines 1 | tee tmp-all-relabeled 
sleep 2s

echo "STEP12: calculating SSE..."
python sse.py centroid tmp-all-relabeled tmp-all-labeled
cat SSE

ssetail=`tail -n 1 SSE`
ssetail=`echo $ssetail`

echo "current sse:"$ssetail
sleep 5s

done

./count.re $nLines 1 

COUNT=0

rm -rf result-all
touch result-all

while [ $COUNT -lt $nThreads ]; do
    paste ${COUNT}.lbl ${COUNT} -d"," > ${COUNT}.result
    cat ${COUNT}.result >> result-all
    COUNT=$(( COUNT + 1 )) 
done

./count.re $nLines 1 > count-result                                  

# Generating results to files.
today=$(date "+%Y%m%d")
hostname=`hostname`

python count-percent.py count-result > count-percent
python count-grep.py count-percent result-all > tmp

rm -rf result
touch result

echo "sourceIP, destIP, clusterSize, anomaly rate" > result
./sort-percent.pl tmp | sed -e 's/$/%/' >> result

echo " "
echo "### RESULT ###"
head -n 20 result

