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

// In the assignment step, each point (thread) computes its distance to each
// cluster centroid and adds its x and y values to the sum of its closest
// centroid, as well as incrementing that centroid's count of assigned points.
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

// Each thread is one cluster, which just recomputes its coordinates as the mean
// of all points assigned to it.
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

void kernel(unsigned long long *h_key, long *h_value_1, long *h_value_2, int size)
{
  int N = size;

  for(int i = 0; i < 5; i++)
  {
	cout << h_key[i] << "," << h_value_1[i] << endl;
  }

  thrust::host_vector<int> h_vec_1(N);
  std::generate(h_vec_1.begin(), h_vec_1.end(), rand); 

  thrust::device_vector<int> key_in(N);
  thrust::copy(h_vec_1.begin(), h_vec_1.end(), key_in.begin()); 

  thrust::host_vector<unsigned long long> h_vec_key_1(N);
  thrust::host_vector<unsigned long long> h_vec_key_2(N);

  thrust::host_vector<long> h_vec_value_1(N);
  thrust::host_vector<long> h_vec_value_2(N);

  cout << N << endl;

  for(int i=0; i < N; i++)
  {
	// cout << h_key[i] << endl;
	h_vec_key_1[i] = h_key[i];
	h_vec_key_2[i] = h_key[i];
	h_vec_value_1[i] = h_value_1[i];
	h_vec_value_2[i] = h_value_2[i];
  }

  /* 1 -> 3 */

  thrust::device_vector<unsigned long long> d_vec_key_1(N);
  thrust::device_vector<long> d_vec_value_1(N);
  thrust::copy(h_vec_key_1.begin(), h_vec_key_1.end(), d_vec_key_1.begin());
  thrust::copy(h_vec_value_1.begin(), h_vec_value_1.end(), d_vec_value_1.begin());

  thrust::sort_by_key(d_vec_key_1.begin(), d_vec_key_1.end(), d_vec_value_1.begin(), thrust::greater<unsigned long long>());

  thrust::host_vector<unsigned long long> h_vec_key_3(N);
  thrust::host_vector<long> h_vec_value_3(N);

  thrust::copy(d_vec_value_1.begin(),d_vec_value_1.end(),h_vec_value_3.begin());
  thrust::copy(d_vec_key_1.begin(),d_vec_key_1.end(),h_vec_key_3.begin());

  /* 2 -> 4 */

  thrust::device_vector<unsigned long long> d_vec_key_2(N);
  thrust::device_vector<long> d_vec_value_2(N);
  thrust::copy(h_vec_key_2.begin(), h_vec_key_2.end(), d_vec_key_2.begin());
  thrust::copy(h_vec_value_2.begin(), h_vec_value_2.end(), d_vec_value_2.begin());

  thrust::sort_by_key(d_vec_key_2.begin(), d_vec_key_2.end(), d_vec_value_2.begin(), thrust::greater<unsigned long long>());

  thrust::host_vector<unsigned long long> h_vec_key_4(N);
  thrust::host_vector<long> h_vec_value_4(N);

  thrust::copy(d_vec_value_2.begin(),d_vec_value_2.end(),h_vec_value_4.begin());
  thrust::copy(d_vec_key_2.begin(),d_vec_key_2.end(),h_vec_key_4.begin());

  cout << "1 -> 3" << endl;
  for(int i = 0; i < 5; i++)
  {
	cout << h_vec_key_3[i] << "," << h_vec_value_3[i] << endl;
  }

  cout << "2 -> 4" << endl;
  for(int i = 0; i < 5; i++)
  {
	cout << h_vec_key_4[i] << "," << h_vec_value_4[i] << endl;
  }

  thrust::device_vector<unsigned long long> d_vec_key_1_out(N);
  thrust::device_vector<long> d_vec_value_1_out(N);

  thrust::device_vector<unsigned long long> d_vec_key_2_out(N);
  thrust::device_vector<long> d_vec_value_2_out(N);

  auto new_end_1 = thrust::reduce_by_key(d_vec_key_1.begin(), d_vec_key_1.end(), d_vec_value_1.begin(),
       	       	 		       d_vec_key_1_out.begin(), d_vec_value_1_out.begin());

  int new_size_1 = new_end_1.first - d_vec_key_1_out.begin() + 1; 

  thrust::host_vector<unsigned long long> h_vec_key_3_out(N);
  thrust::host_vector<long> h_vec_value_3_out(N);

  thrust::copy(d_vec_value_1_out.begin(),d_vec_value_1_out.end(),h_vec_value_3_out.begin());
  thrust::copy(d_vec_key_1_out.begin(),d_vec_key_1_out.end(),h_vec_key_3_out.begin());

  auto new_end_2 = thrust::reduce_by_key(d_vec_key_2.begin(), d_vec_key_2.end(), d_vec_value_2.begin(),
       	       	 		       d_vec_key_2_out.begin(), d_vec_value_2_out.begin());      

  int new_size_2 = new_end_2.first - d_vec_key_2_out.begin() + 1; 

  thrust::host_vector<unsigned long long> h_vec_key_4_out(N);
  thrust::host_vector<long> h_vec_value_4_out(N);

  thrust::copy(d_vec_value_2_out.begin(),d_vec_value_2_out.end(),h_vec_value_4_out.begin());
  thrust::copy(d_vec_key_2_out.begin(),d_vec_key_2_out.end(),h_vec_key_4_out.begin());

  cout << "1 -> 3" << endl;
  for(int i = 0; i < 5; i++)
  {
	cout << h_vec_key_3_out[i] << "," << h_vec_value_3_out[i] << endl;
  }

  cout << "2 -> 4" << endl;
  for(int i = 0; i < 5; i++)
  {
	cout << h_vec_key_4_out[i] << "," << h_vec_value_4_out[i] << endl;
  }

}
