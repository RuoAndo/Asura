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

using namespace std;

__device__ float
squared_l2_distance(float x_1, float y_1, float x_2, float y_2) {
  return (x_1 - x_2) * (x_1 - x_2) + (y_1 - y_2) * (y_1 - y_2);
}

float
squared_l2_distance_h(float x_1, float y_1, float x_2, float y_2) {
  return (x_1 - x_2) * (x_1 - x_2) + (y_1 - y_2) * (y_1 - y_2);
}

__global__ void assign_clusters(const thrust::device_ptr<float> data_x,
                                const thrust::device_ptr<float> data_y,
                                int data_size,
                                const thrust::device_ptr<float> means_x,
                                const thrust::device_ptr<float> means_y,
                                thrust::device_ptr<float> new_sums_x,
                                thrust::device_ptr<float> new_sums_y,
                                int k,
                                thrust::device_ptr<int> counts,
				thrust::device_ptr<int> d_clusterNo) {
				
  const int index = blockIdx.x * blockDim.x + threadIdx.x;
  if (index >= data_size) return;

  // Make global loads once.
  const float x = data_x[index];
  const float y = data_y[index];

  float best_distance = FLT_MAX;
  int best_cluster = 0;
  for (int cluster = 0; cluster < k; ++cluster) {
    const float distance =
        squared_l2_distance(x, y, means_x[cluster], means_y[cluster]);
    if (distance < best_distance) {
      best_distance = distance;
      best_cluster = cluster;
    }
  }

  // d_clusterNo[index] = best_cluster;
  //  __syncthreads();

  atomicAdd(thrust::raw_pointer_cast(new_sums_x + best_cluster), x);
  atomicAdd(thrust::raw_pointer_cast(new_sums_y + best_cluster), y);
  atomicAdd(thrust::raw_pointer_cast(counts + best_cluster), 1);
}

__global__ void compute_new_means(thrust::device_ptr<float> means_x,
                                  thrust::device_ptr<float> means_y,
                                  const thrust::device_ptr<float> new_sum_x,
                                  const thrust::device_ptr<float> new_sum_y,
                                  const thrust::device_ptr<int> counts) {
  const int cluster = threadIdx.x;
  const int count = max(1, counts[cluster]);
  means_x[cluster] = new_sum_x[cluster] / count;
  means_y[cluster] = new_sum_y[cluster] / count;
}

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
  int N = atoi(argv[3]);
  
  int k = 10;
  int number_of_iterations = 1000;
  int counter = 0;

  std::vector<string> h_src(N);
  std::vector<string> h_dst(N);

  thrust::host_vector<float> h_x(N);
  thrust::host_vector<float> h_y(N);

  /*
  std::string line;
  std::ifstream ifs(argv[1]);
  
  counter = 0;
  while (std::getline(ifs, line)) {
  	counter = counter + 1;
  }
  N = counter;
  
  std::cout << N << std::endl;
  */

  const auto cells = parse_csv(argv[1]);
  
  counter = 0;
  for (const auto& rows : cells) {

      h_src[counter] = std::string(rows[0]);
      h_dst[counter] = std::string(rows[1]);

      h_x[counter] = std::stof(rows[2]);      
      counter = counter + 1;
  }

  const auto cells2 = parse_csv(argv[2]);
  
  counter = 0;
  for (const auto& rows : cells2) {

      h_y[counter] = std::stof(rows[2]);      
      counter = counter + 1;
  }

  const size_t number_of_elements = h_x.size();

  thrust::device_vector<float> d_x = h_x;
  thrust::device_vector<float> d_y = h_y;

  thrust::device_vector<int> d_clusterNo(h_x.size());

  std::mt19937 rng(std::random_device{}());
  std::shuffle(h_x.begin(), h_x.end(), rng);
  std::shuffle(h_y.begin(), h_y.end(), rng);
  thrust::device_vector<float> d_mean_x(h_x.begin(), h_x.begin() + k);
  thrust::device_vector<float> d_mean_y(h_y.begin(), h_y.begin() + k);

  thrust::device_vector<float> d_sums_x(k);
  thrust::device_vector<float> d_sums_y(k);
  thrust::device_vector<int> d_counts(k, 0);

  const int threads = 1024;
  const int blocks = (number_of_elements + threads - 1) / threads;

  const auto start = std::chrono::high_resolution_clock::now();
  for (size_t iteration = 0; iteration < number_of_iterations; ++iteration) {
    thrust::fill(d_sums_x.begin(), d_sums_x.end(), 0);
    thrust::fill(d_sums_y.begin(), d_sums_y.end(), 0);
    thrust::fill(d_counts.begin(), d_counts.end(), 0);

    assign_clusters<<<blocks, threads>>>(d_x.data(),
                                         d_y.data(),
                                         number_of_elements,
                                         d_mean_x.data(),
                                         d_mean_y.data(),
                                         d_sums_x.data(),
                                         d_sums_y.data(),
                                         k,
                                         d_counts.data(),
					 d_clusterNo.data());
					 
    cudaDeviceSynchronize();

    compute_new_means<<<1, k>>>(d_mean_x.data(),
                                d_mean_y.data(),
                                d_sums_x.data(),
                                d_sums_y.data(),
                                d_counts.data());
    cudaDeviceSynchronize();
  }
  
  const auto end = std::chrono::high_resolution_clock::now();
  const auto duration =
      std::chrono::duration_cast<std::chrono::duration<float>>(end - start);
  std::cerr << "Took: " << duration.count() << "s" << std::endl;

  thrust::host_vector<float> h_mean_x = d_mean_x;
  thrust::host_vector<float> h_mean_y = d_mean_y;
  thrust::host_vector<int> h_counts = d_counts;

  for (size_t cluster = 0; cluster < k; ++cluster) {
    std::cout << h_mean_x[cluster] << " " << h_mean_y[cluster] << std::endl;
    std::cout << h_counts[cluster] << std::endl;
  }

  // cudaMemcpy(h_clusterNo, d_clusterNo, N * sizeof(int), cudaMemcpyDeviceToHost);

  thrust::host_vector<int> h_clusterNo(d_clusterNo.size());
  // thrust::copy(d_clusterNo.begin(), d_clusterNo.end(), h_clusterNo.begin());

  float distance;
  int best_cluster;

  for(int i = 0; i < N; i++)
  {
	float best_distance = FLT_MAX;
	for (int cluster = 0; cluster < k; ++cluster) {
	
    	    distance = squared_l2_distance_h(h_x[i], h_y[i], h_mean_x[cluster], h_mean_y[cluster]);
	    // std::cout << h_x[i] << "," << h_y[i] << "," << cluster << "," << distance << endl;
	    
	    if (distance < best_distance) {
      	      	 best_distance = distance;
      		 best_cluster = cluster;
             }
 
	}
	
	// std::cout << "*" << h_x[i] << "," << h_y[i] << "," << best_cluster << "," << distance << endl;
	h_clusterNo[i] = best_cluster;
  }

  std::remove("clustered");
  ofstream outputfile("clustered");  

  int sum;

  int nBytes = k * sizeof(float);

  float *percent;
  percent = (float *)malloc(nBytes);


  
  for (size_t cluster = 0; cluster < k; ++cluster) {
    sum = sum + h_counts[cluster];
  }

  for (size_t cluster = 0; cluster < k; ++cluster) {
    percent[cluster] = (float)h_counts[cluster] / (float)sum;
  }

  for(int i=0; i < N; i++)
  {
	outputfile << h_src[i] << "," << h_dst[i] << "," << h_x[i] << "," << h_y[i] << ", cluster" << h_clusterNo[i] << ",(" << percent[h_clusterNo[i]] << "%)" << std::endl;
  }

  outputfile.close();

}
