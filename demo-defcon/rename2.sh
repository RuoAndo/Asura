COUNTER=0
for line in `cat ${1}`
do
    #cut=`echo $line | cut -d "." -f1`
    #echo $cut
    echo "now relabeling " $COUNTER ".rlbl to " $COUNTER ".lbl" 
    \cp $COUNTER.rlbl $COUNTER.lbl
    COUNTER=`expr $COUNTER + 1`
done
