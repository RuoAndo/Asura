if [ "$4" = "" ]
then
    echo "argument required: ./do.sh nThreads nDimensions nClusters nItems"
    echo "./do.sh 10 7 10 5"
    exit
fi

# build
make

# reduction
date_now=`date "+%Y%m%d-%H%M%S"`
date_start=`date "+%s"`
echo "started at:"$date_now 
echo "started at:"$date_now > procTime_group

./asura ./packets2

date_now=`date "+%Y%m%d-%H%M%S"`
echo "finished at:"$date_now
echo "finished at:"$date_now >> procTime_group
date_end=`date "+%s"`

# clutering
# reduced -> all
./auto.sh $1 $2 $3 $4
