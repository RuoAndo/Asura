#include "kmeans.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <alloca.h>
#include "timer.h"

#include <regex.h>

#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<linux/if.h>
#include<net/ethernet.h>
#include<netpacket/packet.h>
#include<netinet/if_ether.h>
#include<netinet/ip.h>
#include<netinet/ip6.h>
#include<netinet/ip_icmp.h>
#include<netinet/icmp6.h>
#include<netinet/tcp.h>
#include<netinet/udp.h>

#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>

#include <cstdio>
#include <cctype>
#include <iostream>

#include <boost/tokenizer.hpp>

using namespace std;
// using namespace tbb;

// Square root of number of intended centroids
/*
const size_t SQRT_K = 4;
const size_t K = SQRT_K*SQRT_K;
const size_t N = 533;

point points[N];
point centroid[K];
cluster_id id[N];
*/

std::vector < std::vector< std::string > > parse_csv(const char* filepath)
{
    std::vector< std::vector< std::string > > cells;
    std::string line;
    std::ifstream ifs(filepath);

    // csvを走査
    while (std::getline(ifs, line)) {

        std::vector< std::string > data;

        // 1行を走査
        boost::tokenizer< boost::escaped_list_separator< char > > tokens(line);
        for (const std::string& token : tokens) {
            data.push_back(token);
        }

        // 1行読み込んだ結果を入れる
        cells.push_back(data);
    }

    return cells;
}

/*
int main(int argc, char* argv[]) {

  const size_t SQRT_K = 4;
  const size_t K = SQRT_K*SQRT_K;
  const size_t N = 533;

  point* points;
  points = (struct point *)malloc(N*sizeof(struct point));

  point* centroid;
  centroid = (struct point *)malloc(N*sizeof(struct point));

  cluster_id* id;
  id = (unsigned short *)malloc(N*sizeof(unsigned short));

  std::vector<string> pair; 
    
    printf("Testing TBB kmeans algorithm...\n");

    const auto cells = parse_csv(argv[1]);
  
    int counter = 0;
    for (const auto& rows : cells) {

      point& p = points[counter];
      
      p.x = std::stof(rows[1]);
      p.y = std::stof(rows[2]);      
      pair.push_back(rows[0]);
      
      counter = counter + 1;
    }
    
    tbb_example::compute_k_means( N, points, K, id, centroid );

#if 1

    int* counts;
    counts = (int *)malloc(K*sizeof(int));

    for( size_t i=0; i<N; ++i ) {
      counts[id[i]] = 0;
    }
    
    for( size_t i=0; i<N; ++i ) {
      counts[id[i]]++;
    }
      
    for( size_t i=0; i<N; ++i ) {

	    unsigned long pair_long = std::stoul(pair[i]);
      
	    std::bitset<64> bset_pair = std::bitset<64>(pair_long);   
	    string bset_pair_string = bset_pair.to_string();
	    
	    std::string ip1 = bset_pair_string.substr(0, 8);
	    std::string ip2 = bset_pair_string.substr(8, 8);
	    std::string ip3 = bset_pair_string.substr(16, 8);
	    std::string ip4 = bset_pair_string.substr(24, 8);

	    std::bitset<8> ip1_bset = std::bitset<8>(ip1);
	    std::bitset<8> ip2_bset = std::bitset<8>(ip2);
	    std::bitset<8> ip3_bset = std::bitset<8>(ip3);
	    std::bitset<8> ip4_bset = std::bitset<8>(ip4);

	    std::cout << ip1_bset.to_ulong() << "." << ip2_bset.to_ulong() << "." << ip3_bset.to_ulong() << "." << ip4_bset.to_ulong() << ",";
	    
	    std::string ip5 = bset_pair_string.substr(32, 8);
	    std::string ip6 = bset_pair_string.substr(40, 8);
	    std::string ip7 = bset_pair_string.substr(48, 8);
	    std::string ip8 = bset_pair_string.substr(56, 8);

	    std::bitset<8> ip5_bset = std::bitset<8>(ip5);
	    std::bitset<8> ip6_bset = std::bitset<8>(ip6);
	    std::bitset<8> ip7_bset = std::bitset<8>(ip7);
	    std::bitset<8> ip8_bset = std::bitset<8>(ip8);

	    std::cout << ip5_bset.to_ulong() << "." << ip6_bset.to_ulong() << "." << ip7_bset.to_ulong() << "." << ip8_bset.to_ulong() << " -> ";

	    float percent = (float)counts[id[i]]/(float)N;
	    printf("%d (%g %g) counts %d / %d [%f%] \n",id[i],points[i].x,points[i].y, counts[id[i]], N, percent); 
    }
#endif
#if 1
    printf("centroids = ");
    for( size_t j=0; j<K; ++j ) {
        printf("(%g %g)",centroid[j].x,centroid[j].y);
    }
    printf("\n");
#endif

    return 0;
}
*/
