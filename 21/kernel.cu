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

using namespace std;

void transfer(unsigned long long *key, long *value, unsigned long long *key_out, long *value_out, int kBytes, int vBytes, size_t data_size, int *new_size, int thread_id)
{
    // unsigned long long *d_A;
    // long *d_B;
    unsigned int t, travdirtime;

    int GPU_number = 0;

    thrust::host_vector<unsigned long long> h_vec_key(data_size);
    thrust::host_vector<unsigned long long> h_vec_value(data_size);

    for(int i=0; i < data_size; i++)
    {
	h_vec_key[i] = key[i];
	h_vec_value[i] = value[i];
    }

    start_timer(&t);

    GPU_number = thread_id - 1;

    cudaSetDevice(GPU_number);
    
    thrust::device_vector<unsigned long long> d_vec_key(data_size);
    thrust::device_vector<long> d_vec_value(data_size);
    
    thrust::copy(h_vec_key.begin(), h_vec_key.end(), d_vec_key.begin());
    thrust::copy(h_vec_value.begin(), h_vec_value.end(), d_vec_value.begin());
    
    cout << "thread:" << thread_id << " - transfer done." << endl;
    travdirtime = stop_timer(&t);
    print_timer(travdirtime);

    /* reduction */
    start_timer(&t);
    
    thrust::sort_by_key(d_vec_key.begin(), d_vec_key.end(), d_vec_value.begin());

    thrust::device_vector<unsigned long long> d_vec_key_out(data_size);
    thrust::device_vector<long> d_vec_value_out(data_size);

    auto new_end = thrust::reduce_by_key(d_vec_key.begin(), d_vec_key.end(), d_vec_value.begin(),
       	       	 		       d_vec_key_out.begin(), d_vec_value_out.begin());

    int new_size_r = new_end.first - d_vec_key_out.begin();

    cout << "thread:" << thread_id << " - reduction done." << endl;
    travdirtime = stop_timer(&t);
    print_timer(travdirtime);

    for(int i = 0; i < 5; i++)
    {
	cout << "threadID:" << thread_id << "-" << d_vec_key_out[i] << "," << d_vec_value_out[i] << endl;
    }

    /*
    start_timer(&t);
    for(int i = 0; i < new_size; i++)
    {
	key_out[i] =  d_vec_key_out[i];
	value_out[i] =  d_vec_value_out[i];
    }

    cout << "thread:" << thread_id << " - transfer(rev) done with new_size " << new_size << endl;
    travdirtime = stop_timer(&t);
    print_timer(travdirtime);
    */

    start_timer(&t);
    thrust::host_vector<unsigned long long> h_vec_key_2(data_size);
    thrust::host_vector<long> h_vec_value_2(data_size);

    thrust::copy(d_vec_value_out.begin(),d_vec_value_out.end(),h_vec_value_2.begin());
    thrust::copy(d_vec_key_out.begin(),d_vec_key_out.end(),h_vec_key_2.begin());

    for(int i = 0; i < new_size_r; i++)
    {
    	key_out[i] =  h_vec_key_2[i];
	value_out[i] =  h_vec_value_2[i];
    }

    cout << "thread:" << thread_id << " - transfer(rev) done with new_size " << new_size_r << endl;
    travdirtime = stop_timer(&t);
    print_timer(travdirtime);

    (*new_size) = new_size_r;

    /*
    for(int i = 0; i < 5; i++)
    {
	cout << "threadID:" << thread_id << "-" << h_vec_key_2[i] << "," << h_vec_value_2[i] << endl;
    }
    */

    /*
    start_timer(&t);

    cudaSetDevice(thread_id);
    cudaMalloc((unsigned long long**)&d_A, kBytes);
    cudaMalloc((long**)&d_B, vBytes);
    cudaMemcpy(d_A, key, kBytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, value, vBytes, cudaMemcpyHostToDevice);

    cout << "thread:" << thread_id << " - transfer done." << endl;
    travdirtime = stop_timer(&t);
    print_timer(travdirtime);
    */
}


