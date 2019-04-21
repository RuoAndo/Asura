rm -rf all-relabeled
touch all-relabeled
for line in `cat ${1}`
do
    #echo $line
    cat $line >> all-relabeled
done
