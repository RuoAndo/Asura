rm -rf all
cp reduced all

date_now=`date "+%Y%m%d-%H%M%S"`
date_start=`date "+%s"`
echo "started at:"$date_now 
echo "started at:"$date_now > procTime

time ./first.sh all $1 $2 $3 $4 $5
time ./second.sh all $1 $2 $3 $4 $5
sleep 5s

date_now=`date "+%Y%m%d-%H%M%S"`
echo "finished at:"$date_now
echo "finished at:"$date_now >> procTime
date_end=`date "+%s"`

#echo $date_start
#echo $date_end

diff=`echo $((date_end-date_start))`
div=`echo $((diff/60))`

echo "proc time:"$diff"sec" 
echo "proc time:"$div"min" 

echo "proc time:"$diff"sec" >> procTime
echo "proc time:"$div"min" >> procTime
