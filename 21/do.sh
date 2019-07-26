./build-asura-2.sh asura
./build-gpu.sh km-thrust

time ./asura $1 
nLines=`wc -l tmp-asura-1`
echo $nLines
time ./km-thrust tmp-asura-1 tmp-asura-2 $nLines
