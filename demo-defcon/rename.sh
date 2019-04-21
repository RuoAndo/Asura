COUNTER=0
for line in `cat ${1}`
do
    echo "now processing CHUNK" $line ":" $COUNTER " ..."
    python trans.py $line > $COUNTER
    COUNTER=`expr $COUNTER + 1`
done
