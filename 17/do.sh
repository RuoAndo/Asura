./build-asura.sh asura
./build-gpu.sh reduce

time ./asura $1
time ./reduce.sh tmp-asura-1
time ./reduce.sh tmp-asura-2
