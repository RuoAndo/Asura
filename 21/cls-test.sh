nLines=`wc -l tmp-asura-1`
echo $nLines
time ./km-thrust tmp-asura-1 tmp-asura-2 $nLines
