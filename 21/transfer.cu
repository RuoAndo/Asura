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

int main(int argc, const char* argv[])
{
  int N = atoi(argv[2]);
  
  int counter = 0;
  int ngpus = 4;
  const size_t iBytes = N * sizeof(float);  

  static int RATIO = 1000000;

  unsigned int t, travdirtime; 

  float **d_A = (float **)malloc(sizeof(float *) * ngpus);
  float **d_B = (float **)malloc(sizeof(float *) * ngpus);
  // float **d_C = (float **)malloc(sizeof(float *) * ngpus);

  float **h_A = (float **)malloc(sizeof(float *) * ngpus);
  float **h_B = (float **)malloc(sizeof(float *) * ngpus);    

  cudaStream_t *stream = (cudaStream_t *)malloc(sizeof(cudaStream_t) * ngpus); 

  for (int i = 0; i < ngpus; i++)
  {
	cudaSetDevice(i);
        cudaMalloc((void **) &d_A[i], iBytes);
	cudaMalloc((void **) &d_B[i], iBytes);
	// cudaMalloc((void **) &d_C[i], iBytes);

        cudaMallocHost((void **) &h_A[i], iBytes);
	cudaMallocHost((void **) &h_B[i], iBytes);

        cudaStreamCreate(&stream[i]);
  }                  

  /*
  thrust::host_vector<float> h_x(N);
  thrust::host_vector<float> h_y(N);
  */

  const auto cells = parse_csv(argv[1]);
  
  cudaSetDevice(0);

  counter = 0;
  for (const auto& rows : cells) {
      h_A[0][counter] = std::stof(rows[0]);

      if(counter % RATIO == 0)
	{
		cout << counter / RATIO << "stored... " << endl;
        }

      counter = counter + 1;
  }

  cudaDeviceEnablePeerAccess(0, 1);
  // printf("> GPU%d enabled direct access to GPU%d\n", , j); 

  cout << "host to device" << endl;
  start_timer(&t);
  cudaMemcpy(d_A[0], h_A[0], iBytes, cudaMemcpyHostToDevice);
  travdirtime = stop_timer(&t);
  print_timer(travdirtime);  

  cout << "device to device" << endl;
  start_timer(&t);
  cudaMemcpy(d_A[1], d_A[0], iBytes, cudaMemcpyDeviceToDevice);
  travdirtime = stop_timer(&t);
  print_timer(travdirtime);  

  /*
  const auto cells2 = parse_csv(argv[2]);
  
  counter = 0;
  for (const auto& rows : cells2) {
      h_y[counter] = std::stof(rows[1]);
      counter = counter + 1;
  }
  */

}
