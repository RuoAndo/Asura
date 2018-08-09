if [ "$6" = "" ]
then
    echo "argument required: ./first file nThreads nDimensions nClusters nItemss TAG"
    exit
fi

allnLines=`wc -l $1 | cut -d " " -f 1`
echo "the number of lines: "$allnLines
nThreads=$2

nLines=`expr $allnLines / $2`
#echo $nLines
#nLines=1000000
nDimensions=$3
nClusters=$4

nItems=$5 # nDimensions-2 / items: src dst n[* * *] 
TAG=$6

./clean.sh

grep THREAD_NUM init-label.cpp | grep define
echo "the numbers of threads\:"$nThreads

echo "STEP1: building executables ..."

# [1] init-label.cpp
# [2] count.cpp
# [3] avg.cpp
# [4] fill2.cpp
# [5] relabel.cpp

# [1] init-label.cpp
cat init-label.cpp | sed "s/#define THREAD_NUM 15/#define THREAD_NUM $nThreads/" > init-label.tmp.cpp
cat init-label.tmp.cpp | sed "s/#define CLUSTER_NUM 20/#define CLUSTER_NUM $nClusters/" > init-label.tmp.2.cpp
cat init-label.tmp.2.cpp | sed "s/res.rightCols(6)/res.rightCols($nDimensions)/" > init-label.re.cpp 
./build.sh init-label.re

rm -rf init-label.tmp.cpp
rm -rf init-label.tmp.2.cpp

# [2] count.cpp 
cat count.cpp | sed "s/#define THREAD_NUM N/#define THREAD_NUM $nThreads/" > count.tmp.cpp
cat count.tmp.cpp | sed "s/#define CLUSTER_NUM N/#define CLUSTER_NUM $nClusters/" > count.tmp.2.cpp
cat count.tmp.2.cpp | sed "s/res.rightCols(6)/res.rightCols($nDimensions)/" > count.re.cpp 
./build.sh count.re

rm -rf count.tmp.cpp
rm -rf count.tmp.2.cpp

# [3] avg.cpp
cat avg.cpp | sed "s/#define THREAD_NUM N/#define THREAD_NUM $nThreads/" > avg.tmp.cpp
cat avg.tmp.cpp | sed "s/#define CLUSTER_NUM N/#define CLUSTER_NUM $nClusters/" > avg.tmp.2.cpp
cat avg.tmp.2.cpp | sed "s/#define ITEM_NUM N/#define ITEM_NUM $nItems/" > avg.tmp.3.cpp
cat avg.tmp.3.cpp | sed "s/rightCols(N)/rightCols($nItems)/" > avg.re.cpp
./build.sh avg.re

rm -rf avg.tmp.cpp
rm -rf avg.tmp.2.cpp
rm -rf avg.tmp.3.cpp

# [4] fill2.cpp
cat fill2.cpp | sed "s/#define THREAD_NUM N/#define THREAD_NUM $nThreads/" > fill2.tmp.cpp
cat fill2.tmp.cpp | sed "s/#define CLUSTER_NUM N/#define CLUSTER_NUM $nClusters/" > fill2.tmp.2.cpp
cat fill2.tmp.2.cpp | sed "s/#define ITEM_NUM N/#define ITEM_NUM $nItems/" > fill2.tmp.3.cpp
cat fill2.tmp.3.cpp | sed "s/ avg(CONST);/ avg($nItems);/" > fill2.re.cpp
./build.sh fill2.re

rm -rf fill2.tmp.cpp
rm -rf fill2.tmp.2.cpp
rm -rf fill2.tmp.3.cpp

# [5] relabel.cpp
cat relabel.cpp | sed "s/#define THREAD_NUM N/#define THREAD_NUM $nThreads/" > relabel.tmp.cpp
cat relabel.tmp.cpp | sed "s/#define CLUSTER_NUM N/#define CLUSTER_NUM $nClusters/" > relabel.tmp.2.cpp
cat relabel.tmp.2.cpp | sed "s/rightCols(N)/rightCols($nItems)/" > relabel.re.cpp
./build.sh relabel.re

rm -rf relabel.tmp.cpp
rm -rf relabel.tmp.2.cpp

echo "STEP2: now spliting files ..".
rm -rf hout*
headLine=`expr $nLines \* $nThreads`

head -n $headLine $1 > $1.headed
split -l $nLines $1.headed hout

pyenv local system

ls hout* > list
./rename.sh list # calling trans.py

echo "STEP3: now initlializing labels ..."
./init-label.re $nLines $nDimensions

