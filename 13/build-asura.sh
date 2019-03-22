#rm -rf $1
#rm -rf a.out
nvcc -g -o $1 $1.cpp -std=c++11 -ltbb -Xcompiler="-fpermissive, -lpthread"
