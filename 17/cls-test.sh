nLines=`wc -l tmp-asura-1-reduced`
echo $nLines
time ./km-thrust tmp-asura-1-reduced tmp-asura-2-reduced $nLines
