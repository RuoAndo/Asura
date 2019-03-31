./build-asura.sh asura
./build-gpu.sh reduce
./build-gpu.sh km-thrust

time ./asura $1
time ./reduce.sh tmp-asura-1
time ./reduce.sh tmp-asura-2

nLines=`wc -l tmp-asura-1-reduced`
time ./km-thrust tmp-asura-1-reduced tmp-asura-2-reduced $nLines
