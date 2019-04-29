rm -rf $1
rm -rf a.out
g++ -o asura asura.cpp csv.cpp test_kmeans.cpp kmeans.h kmeans_tbb.cpp repair_empty_clusters.cpp -std=c++11 -std=c++11 -ltbb -fpermissive -lpthread
