#nvcc -g -c $1.cpp csv.cpp -std=c++11 -ltbb -Xcompiler="-fpermissive, -lpthread"
#nvcc -g -o $1 $1.o csv.o kernel.o  -std=c++11 -ltbb --generate-code arch=compute_60,code=sm_60
nvcc -g -c $1.cpp -std=c++11 -ltbb -Xcompiler="-fpermissive, -lpthread"
nvcc -g -c kernel.cu  -std=c++11
nvcc -g -o $1 $1.o kernel.o  -std=c++11 -ltbb --generate-code arch=compute_60,code=sm_60

