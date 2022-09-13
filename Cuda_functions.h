
#ifndef CUDA_FUNCTIONS_H_
#define CUDA_FUNCTIONS_H_
#include "Data_Manager.h"
#include "Input_Output_Manager.h"

/*Calclate all the pictures match until 1024*1024 indexes*/
int* find_matches_by_cuda(Search_Data* search_data,int picture_start ,int picture_end);
/*verify Cuda err*/
void verify_cuda_succedded(int line);


#endif /* CUDA_FUNCTIONS_H_ */

