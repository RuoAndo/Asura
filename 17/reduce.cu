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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tbb/concurrent_hash_map.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/tick_count.h"
#include "tbb/task_scheduler_init.h"
//  #include "tbb/tbb_allocator.hz"
#include "utility.h"

#include "csv.hpp"
typedef std::basic_string<char,std::char_traits<char>,tbb::tbb_allocator<char> > MyString;

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/generate.h>
#include <thrust/sort.h>
#include <thrust/copy.h>
#include <algorithm>
#include <cstdlib>
#include "util.h"

using namespace tbb;
using namespace std;

static bool verbose = false;
static bool silent = false;

// const int size_factor = 2;
// typedef concurrent_hash_map<MyString,int> StringTable;
typedef concurrent_hash_map<MyString,std::vector<string>> StringTable;
std::vector<string> v_pair;
std::vector<string> v_count;
static MyString* Data;

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

int main( int argc, char* argv[] ) {

  int counter = 0;
  int N = atoi(argv[2]);  
  char* tmpchar;

  struct in_addr inaddr;
  char *some_addr;

  std::string timestamp;

  thrust::host_vector<unsigned long long> h_vec_1(N);
  thrust::host_vector<long> h_vec_2(N);   

    try {
        tbb::tick_count mainStartTime = tbb::tick_count::now();
        srand(2);

        utility::thread_number_range threads(tbb::task_scheduler_init::default_num_threads,0);

        if ( silent ) verbose = false;

        Data = new MyString[N];

	const string csv_file = std::string(argv[1]); 
	vector<vector<string>> data; 

	try {
	  Csv objCsv(csv_file);
	  if (!objCsv.getCsv(data)) {
	    cout << "read ERROR" << endl;
	    return 1;
	  }

	 std::remove("tmp-reduce");
	 ofstream outputfile("tmp-reduce");

	 //outputfile << "timestamp,src,dest,counted" << endl;

      	  for (int row = 0; row < data.size(); row++) {
	    vector<string> rec = data[row]; 

	    h_vec_1[row] = stoull(rec[0]);
	    h_vec_2[row] = stol(rec[1]);
	    }

	  for(long i=0; i <10; i++)
	  	   cout << h_vec_1[i] << "," << h_vec_2[i] << endl;
	  
	  thrust::device_vector<unsigned long long> key_in(N); // = h_vec_1;
	  thrust::device_vector<unsigned long long> value_in(N); // = h_vec_2; 

	  thrust::copy(h_vec_1.begin(), h_vec_1.end(), key_in.begin());
	  thrust::copy(h_vec_2.begin(), h_vec_2.end(), value_in.begin());

	  thrust::device_vector<unsigned long long> dkey_out(h_vec_1.size());
	  thrust::device_vector<unsigned long long> dvalue_out(h_vec_2.size());

	  thrust::sort(key_in.begin(), key_in.end());
	  // thrust::sort_by_key(value_in.begin(), value_in.end(), key_in.begin()); 

	  auto new_end = thrust::reduce_by_key(key_in.begin(),
					       key_in.end(),
					       value_in.begin(),
					       dkey_out.begin(),
	  				       dvalue_out.begin());

	  long new_size = new_end.first - dkey_out.begin();
	  cout << "size:" << key_in.size() << "," << new_size << endl;

	  thrust::host_vector<unsigned long long> hkey_out_2(new_size);
	  thrust::host_vector<unsigned long long> hvalue_out_2(new_size);

	  for(int i=0;i<new_size;i++)
	  {
		hkey_out_2[i] = dkey_out[i];
		hvalue_out_2[i] = dvalue_out[i];
	  }
	  
          // thrust::sort_by_key(hvalue_out_2.begin(), hvalue_out_2.end(), hkey_out_2.begin(), thrust::greater<long>());
          thrust::sort_by_key(hkey_out_2.begin(), hkey_out_2.end(), hvalue_out_2.begin(), thrust::greater<unsigned long long>());

	  for(long i=0; i <new_size; i++)
	  {
	    outputfile << hkey_out_2[i] << "," << hvalue_out_2[i] << endl;
          }

	  /*
	  thrust::host_vector<long> hkey_out_2(N,0);
	  thrust::host_vector<long> hvalue_out_2(N,0);

	  for(int i=0;i<new_size;i++)
	  {
		hkey_out_2[i] = dkey_out[i];
		hvalue_out_2[i] = dvalue_out[i];
	  }
	  */

	  // thrust::sort_by_key(hvalue_out_2, hvalue_out_2 + (int)new_size, hvalue_out_2);
          // thrust::sort_by_key(hvalue_out_2.begin(), hvalue_out_2.end(), hkey_out_2.begin(), thrust::greater<long>());

	  /*
	  for(long i=0; i <new_size; i++)
	  {
	    outputfile << hkey_out_2[i] << "," << hvalue_out_2[i] << endl;
          }
	  */

	  outputfile.close();
	}
	
	catch (...) {
	  cout << "EXCEPTION!" << endl;
	  return 1;
	}
	
        delete[] Data;
        utility::report_elapsed_time((tbb::tick_count::now() - mainStartTime).seconds());
       
        return 0;
	
    } catch(std::exception& e) {
        std::cerr<<"error occurred. error text is :\"" <<e.what()<<"\"\n";
    }
}
