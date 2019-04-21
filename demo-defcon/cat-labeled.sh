rm -rf all-labeled
touch all-labeled
for line in `cat ${1}`
do
    #echo $line
    cat $line >> all-labeled
done
