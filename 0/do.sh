./build-gpu.sh asura
./asura $1
g++ kmeans.h kmeans_tbb.cpp repair_empty_clusters.cpp test_kmeans.cpp -std=c++11 -ltbb #-pg -g
nLines=`wc -l tmp3 | cut -d " " -f 1`
./a.out tmp3 ${nLines}
