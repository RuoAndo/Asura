#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

#include <string>
#include <iostream>
#include <fstream>
#include <eigen3/Eigen/Dense>

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/SVD>

#define THREAD_NUM N
#define CLUSTER_NUM N
static int cluster_no[CLUSTER_NUM];

using namespace Eigen;
using namespace std;

Eigen::MatrixXd avg;

Eigen::MatrixXd readCSV(std::string file, int rows, int cols) {
  
  std::ifstream in(file.c_str());

  std::string line;

  int row = 0;
  int col = 0;

  Eigen::MatrixXd res = Eigen::MatrixXd(rows, cols);

  if (in.is_open()) {

    while (std::getline(in, line)) {

      char *ptr = (char *) line.c_str();
      int len = line.length();

      col = 0;

      char *start = ptr;
      for (int i = 0; i < len; i++) {

	if (ptr[i] == ',') {
	  res(row, col++) = atof(start);
	  start = ptr + i + 1;
	}
      }
      res(row, col) = atof(start);

      row++;
    }

    in.close();
  }
  return res;
}

typedef struct _thread_arg {
    int id;
    int rows;
    int columns;
} thread_arg_t;

typedef struct _result {
  int cluster_no[CLUSTER_NUM];  
  pthread_mutex_t mutex;    
} result_t;
result_t result;


void thread_func(void *arg) {
    thread_arg_t* targ = (thread_arg_t *)arg;
    int i, j, k;
    int counter = 0;
    long tmpNo;
    int my_cluster_no[CLUSTER_NUM];
    
    string fname_label = std::to_string(targ->id) + ".lbl";      
    
    Eigen::MatrixXd res_label= readCSV(fname_label, targ->rows,targ->columns);

    for(i = 0; i < CLUSTER_NUM; i++)
      my_cluster_no[i] = 0; 
    
    for(i=0; i< res_label.rows(); i++)
	{
	  tmpNo = res_label.row(i)(0);
	  my_cluster_no[tmpNo]++;
	}

    pthread_mutex_lock(&result.mutex);

    for(i=0; i<CLUSTER_NUM; i++)
      {
	result.cluster_no[i] += my_cluster_no[i];
      }
            
    pthread_mutex_unlock(&result.mutex);
    
    return;
}

int main(int argc, char *argv[])
{
    pthread_t handle[THREAD_NUM];
    thread_arg_t targ[THREAD_NUM];
    int i;

    /* クラスタ初期化 */
    for(i = 0; i < CLUSTER_NUM; i++)
      result.cluster_no[i] = 0; 

    /* 処理開始 */
    for (i = 0; i < THREAD_NUM; i++) {
        targ[i].id = i;
        targ[i].rows = atoi(argv[1]);
	targ[i].columns = atoi(argv[2]);
        pthread_create(&handle[i], NULL, (void*)thread_func, (void*)&targ[i]);
    }
    
    /* 終了を待つ */
    for (i = 0; i < THREAD_NUM; i++) 
        pthread_join(handle[i], NULL);
    
    for(i=0; i<CLUSTER_NUM; i++)
      {
	std::cout << "CLUSTER," << i << "," << "SIZE," << result.cluster_no[i] << endl;
      }

}
