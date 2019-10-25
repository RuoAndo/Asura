#include <algorithm>
#include <cfloat>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <boost/tokenizer.hpp>

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

#include <string>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <bitset>
#include <random>
#include "timer.h"
#include <time.h>

using namespace std;

void transfer(unsigned long long *key, long *value, unsigned long long *key_out, long *value_out, int kBytes, int vBytes, size_t data_size, int *new_size)
{
    // unsigned long long *d_A;
    // long *d_B;
    unsigned int t, travdirtime;
    struct timespec startTime, endTime, sleepTime;

    cout << "[GPU kernel]data size:" << data_size << ":kBytes:" << kBytes << ":vBytes:" << endl;

    int GPU_number = 0;

    thrust::host_vector<unsigned long long> h_vec_key(data_size);
    thrust::host_vector<unsigned long long> h_vec_value(data_size);

    for(int i=0; i < data_size; i++)
    {
	h_vec_key[i] = key[i];
	h_vec_value[i] = value[i];
    }

    // start_timer(&t);
    
    thrust::device_vector<unsigned long long> d_vec_key(data_size);
    thrust::device_vector<long> d_vec_value(data_size);
    
    thrust::copy(h_vec_key.begin(), h_vec_key.end(), d_vec_key.begin());
    thrust::copy(h_vec_value.begin(), h_vec_value.end(), d_vec_value.begin());

    clock_gettime(CLOCK_REALTIME, &startTime);
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 123;

    thrust::sort_by_key(d_vec_key.begin(), d_vec_key.end(), d_vec_value.begin());

    clock_gettime(CLOCK_REALTIME, &endTime);
    printf("[GPU kernel] sort_by_key done: ");
    if (endTime.tv_nsec < startTime.tv_nsec) {
       printf("%10ld.%09ld", endTime.tv_sec - startTime.tv_sec - 1 ,endTime.tv_nsec + 1000000000 - startTime.tv_nsec);
	} else {
       printf("%10ld.%09ld", endTime.tv_sec - startTime.tv_sec, endTime.tv_nsec - startTime.tv_nsec);
    }
    printf(" sec\n");

    thrust::device_vector<unsigned long long> d_vec_key_out(data_size);
    thrust::device_vector<long> d_vec_value_out(data_size);

    clock_gettime(CLOCK_REALTIME, &startTime);
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 123;

    auto new_end = thrust::reduce_by_key(d_vec_key.begin(), d_vec_key.end(), d_vec_value.begin(),
       	       	 		       d_vec_key_out.begin(), d_vec_value_out.begin());

    int new_size_r = new_end.first - d_vec_key_out.begin();

    clock_gettime(CLOCK_REALTIME, &endTime);
    printf("[GPU kernel] reduce_by_key done: ");
    if (endTime.tv_nsec < startTime.tv_nsec) {
       printf("%10ld.%09ld", endTime.tv_sec - startTime.tv_sec - 1 ,endTime.tv_nsec + 1000000000 - startTime.tv_nsec);
	} else {
       printf("%10ld.%09ld", endTime.tv_sec - startTime.tv_sec, endTime.tv_nsec - startTime.tv_nsec);
    }
    printf(" sec\n");

    thrust::host_vector<unsigned long long> h_vec_key_2(data_size);
    thrust::host_vector<long> h_vec_value_2(data_size);

    thrust::copy(d_vec_value_out.begin(),d_vec_value_out.end(),h_vec_value_2.begin());
    thrust::copy(d_vec_key_out.begin(),d_vec_key_out.end(),h_vec_key_2.begin());

    for(int i = 0; i < new_size_r; i++)
    {
    	key_out[i] =  h_vec_key_2[i];
	value_out[i] =  h_vec_value_2[i];
    }

    cout << "[GPU kernel] transfer(rev) done with new_size " << new_size_r << endl;

    (*new_size) = new_size_r;

}


