if [ "$2" = "" ]
then
    echo "./asura.sh nThread DIR"
    echo "USAGE: ./asura.sh 1000 packets"
    exit
fi

nThreads=$1
DIR=$2

rm -rf asura.re
cat asura.cpp | sed "s/#define WORKER_THREAD_NUM N/#define WORKER_THREAD_NUM $nThreads/" > asura.tmp.cpp
cat asura.tmp.cpp | sed "s/#define MAX_QUEUE_NUM N/#define MAX_QUEUE_NUM 3000/" > asura.re.cpp
./build.sh asura.re 

date_now=`date "+%Y%m%d-%H%M%S"`
date_start=`date "+%s"`
echo "started at:"$date_now 
echo "started at:"$date_now > procTime

time ./asura.re $2

date_now=`date "+%Y%m%d-%H%M%S"`
echo "finished at:"$date_now
echo "finished at:"$date_now >> procTime
date_end=`date "+%s"`

diff=`echo $((date_end-date_start))`
div=`echo $((diff/60))`

echo "proc time:"$diff"sec" 
echo "proc time:"$div"min" 

echo "proc time:"$diff"sec" >> procTime
echo "proc time:"$div"min" >> procTime
