#if __linux__ && defined(__INTEL_COMPILER)
#define __sync_fetch_and_add(ptr,addend) _InterlockedExchangeAdd(const_cast<void*>(reinterpret_cast<volatile void*>(ptr)), addend)
#endif
#include <string>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <bitset>
#include <locale>
#include <codecvt>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>   

#include "tbb/concurrent_hash_map.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/tick_count.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/concurrent_vector.h"
//  #include "tbb/tbb_allocator.hz"
#include "utility.h"

#include "csv.hpp"

using namespace tbb;
using namespace std;

concurrent_vector < string > IPpair;
std::vector<string> sv;
std::vector<string> sourceIP;
std::vector<string> destinationIP;
std::vector<string> timestamp;

std::vector<string> IPstring_src;
std::vector<string> IPstring_dst;

std::vector<std::string> split_string_2(std::string str, char del) {
  int first = 0;
  int last = str.find_first_of(del);

  std::vector<std::string> result;

  while (first < str.size()) {
    std::string subStr(str, first, last - first);

    result.push_back(subStr);

    first = last + 1;
    last = str.find_first_of(del, first);

    if (last == std::string::npos) {
      last = str.size();
    }
  }

  return result;
}

wstring widen( const string& str )
{
  wostringstream wstm ;
  const ctype<wchar_t>& ctfacet =
    use_facet< ctype<wchar_t> >( wstm.getloc() ) ;
  for( size_t i=0 ; i<str.size() ; ++i )
    wstm << ctfacet.widen( str[i] ) ;
  return wstm.str() ;
}

string narrow( const wstring& str )
{
  ostringstream stm ;
  const ctype<char>& ctfacet =
    use_facet< ctype<char> >( stm.getloc() ) ;
  for( size_t i=0 ; i<str.size() ; ++i )
    stm << ctfacet.narrow( str[i], 0 ) ;
  return stm.str() ;
}

int main( int argc, char* argv[] ) {

  int counter = 0;
  int N = atoi(argv[2]);  

  struct in_addr inaddr;
  char *some_addr;
  //long long pair_long;
  
    try {

        tbb::tick_count mainStartTime = tbb::tick_count::now();
        utility::thread_number_range threads(tbb::task_scheduler_init::default_num_threads,0);

        // Data = new MyString[N];
	const string csv_file = std::string(argv[1]); 
	vector<vector<string>> data; 
	
	// std::remove("trans");
	// ofstream outputfile("trans");
	
	try {
	  Csv objCsv(csv_file);
	  if (!objCsv.getCsv(data)) {
	    cout << "read ERROR" << endl;
	    return 1;
	  }

	  for (unsigned int row = 0; row < data.size(); row++) {
	    vector<string> rec = data[row]; 

	    std::cout << rec[0] << ",";
	    
	    unsigned long pair_long = std::stol(rec[1]);
      
	    std::bitset<32> bset_pair = std::bitset<32>(pair_long);   
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

	    pair_long = std::stol(rec[2]);
      
	    bset_pair = std::bitset<32>(pair_long);   
	    bset_pair_string = bset_pair.to_string();
	    
	    std::string ip5 = bset_pair_string.substr(0, 8);
	    std::string ip6 = bset_pair_string.substr(8, 8);
	    std::string ip7 = bset_pair_string.substr(16, 8);
	    std::string ip8 = bset_pair_string.substr(24, 8);

	    std::bitset<8> ip5_bset = std::bitset<8>(ip5);
	    std::bitset<8> ip6_bset = std::bitset<8>(ip6);
	    std::bitset<8> ip7_bset = std::bitset<8>(ip7);
	    std::bitset<8> ip8_bset = std::bitset<8>(ip8);

	    std::cout << ip5_bset.to_ulong() << "." << ip6_bset.to_ulong() << "." << ip7_bset.to_ulong() << "." << ip8_bset.to_ulong() << endl;
	    
	  }
	}
	catch (...) {
	  cout << "EXCEPTION!" << endl;
	  return 1;
	}

	/*
	counter = 0;
	for(auto itr = sv.begin(); itr != sv.end(); ++itr) {
	     std::string tmp_string = *itr;
	     std::bitset<64> trans_tmp (tmp_string);
	     
	     unsigned long long int n = bitset<64>(tmp_string).to_ullong();
	     unsigned long long int s = bitset<32>(IPstring_src[counter]).to_ullong();
	     unsigned long long int d = bitset<32>(IPstring_dst[counter]).to_ullong();
	                                                                                    
             inaddr = { htonl(s) };
	     some_addr = inet_ntoa(inaddr);
	     string src_string = string(some_addr);

	     inaddr = { htonl(d) };
	     some_addr = inet_ntoa(inaddr);
	     string dst_string = string(some_addr);

	     // std::cout << srcIP << "," << destIP << "," << tmp_string << "," << n << endl;
	     // std::cout << timestamp[counter] << "," << sourceIP[counter] << "," << s << "," << src_string << "," << destinationIP[counter] << "," << d << "," << dst_string << "," << tmp_string << "," << n << endl;

	     // outputfile << timestamp[counter] << "," << sourceIP[counter] << "," << s << "," << src_string << "," << destinationIP[counter] << "," << d << "," << dst_string << "," << tmp_string << "," << n << endl;
	     
	     outputfile << timestamp[counter] << "," << n << endl;
	     counter = counter + 1;
	}
	*/

	// outputfile.close();
	
        utility::report_elapsed_time((tbb::tick_count::now() - mainStartTime).seconds());
       
        return 0;
	
    } catch(std::exception& e) {
        std::cerr<<"error occurred. error text is :\"" <<e.what()<<"\"\n";
    }
}
