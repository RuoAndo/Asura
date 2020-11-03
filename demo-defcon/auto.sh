if [ "$5" = "" ]
then
    echo "./auto.sh nThread nDimensions nClusters nItems dir"
    echo "./auto.sh 100 7 10 5 packtes"
    exit
fi

date=`date --date '2 day ago' +%Y%m%d`
echo $date

date_now=`date "+%Y%m%d-%H%M%S"`
date_start=`date "+%s"`
echo "started at:"$date_now 
echo "started at:"$date_now > procTime

time ./asura.sh $1 $5

rm -rf all 
cp reduced all

time ./first.sh all 10 $2 $3 $4 $5
time ./second.sh all 10 $2 $3 $4 $5
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

