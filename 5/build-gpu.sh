rm -rf a.out
g++ -c csv.cpp -std=c++11 -fpermissive
nvcc -fpermissive -g -o $1 csv.o $1.cu -std=c++11 -lpthread
