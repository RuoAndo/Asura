#rm -rf $1
#rm -rf a.out
#g++ -c csv.cpp -std=c++11 -ltbb
nvcc -g -o $1 $1.cpp -std=c++11 -ltbb -Xcompiler="-fpermissive, -lpthread"
