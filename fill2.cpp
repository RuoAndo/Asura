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
#define ITEM_NUM N

static int cluster_no[CLUSTER_NUM];
Eigen::VectorXd avg(CONST);

using namespace Eigen;
using namespace std;

typedef struct _thread_arg {
  int id;
  int rows;
  int columns;
} thread_arg_t;

typedef struct _result {
  double distance;
  double items[ITEM_NUM];
  pthread_mutex_t mutex;    
} result_t;
result_t result;

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

void thread_func(void *arg) {
    thread_arg_t* targ = (thread_arg_t *)arg;
    int id;
    int i,j;
    int counter = 0;
    double distance_tmp = 0;

    long sum_item[ITEM_NUM];

    srand((unsigned int)time(NULL));

    id = targ->id;
    string fname = std::to_string(targ->id);
    
    Eigen::MatrixXd res = readCSV(fname, targ->rows,targ->columns);
    Eigen::MatrixXd res2 = res.rightCols(ITEM_NUM);
    double my_items[ITEM_NUM];
    
    for(i=0; i< res2.rows(); i++)
	{
	  if((rand()*1000) %100 > 95) 
	    {

	      Eigen::VectorXd res2vec = res2.row(i);

	      counter = 0;
	      for(j=0; j < ITEM_NUM; j++)
		{
		  sum_item[j] =+ res2vec(j);
		  counter =+ 1;
		}

	      // Eigen::VectorXd distance = (res2vec - avg).colwise().squaredNorm();

		  distance_tmp = 0;
		  my_items[0] = sum_item[0]/counter;
		  my_items[1] = sum_item[1]/counter;
		  my_items[2] = sum_item[2]/counter;

	    }
	}

    pthread_mutex_lock(&result.mutex);

            result.distance = distance_tmp;
	    int x = rand() % 201 - 100;
	    result.items[0] = my_items[0] + (my_items[0] / x);
	    x = rand() % 201 - 100;
	    result.items[1] = my_items[1] + (my_items[1] / x);
	    result.items[2] = my_items[2];
	    
    pthread_mutex_unlock(&result.mutex);

    /*
    std::cout << distance_tmp << "," << point_max_distance << std::endl;
    */

    return;
}

int main(int argc, char *argv[])
{
    pthread_t handle[THREAD_NUM];
    thread_arg_t targ[THREAD_NUM];
    int i,j;

    /* ˆ—ŠJŽn */
    for (i = 0; i < THREAD_NUM; i++) {
        targ[i].id = i;

	for (j = 0; j < ITEM_NUM; j++) {
	  // std::cout << i << ":" << atoi(argv[j]) << std::endl;
	  avg(j) = atof(argv[j+1]);
	}

        targ[i].rows = atoi(argv[4]);
	targ[i].columns = atoi(argv[5]);

        pthread_create(&handle[i], NULL, (void*)thread_func, (void*)&targ[i]);

    }

    /*
    for (i = 0; i < THREAD_NUM; i++) {
        targ[i].id = i;
        targ[i].rows = atoi(argv[4]);
	targ[i].columns = atoi(argv[5]);
        pthread_create(&handle[i], NULL, (void*)thread_func, (void*)&targ[i]);
    }
    */    

    /* I—¹‚ð‘Ò‚Â */
    for (i = 0; i < THREAD_NUM; i++) 
        pthread_join(handle[i], NULL);

    /*
    std::cout << "done" << std::endl;
    std::cout << "distance:" << result.distance << std::endl;
    std::cout << "0:" << result.items[0] << std::endl;
    std::cout << "1:" << result.items[1] << std::endl;
    std::cout << "2:" << result.items[2] << std::endl;
    */

    std::cout << result.items[0] << "," << result.items[1] << "," << result.items[2] << std::endl; 
}
