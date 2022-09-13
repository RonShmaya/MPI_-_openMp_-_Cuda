#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include "Tools.h"

int is_rnd_already = 0;
/*using for make input file*/
float get_rnd_between_one_to_zero(){
	if(is_rnd_already == 0)
		srand(time(NULL));
	is_rnd_already = 1;

	return (float)rand()/RAND_MAX;
}
/*using for make input file*/
int get_rnd(int min_limit,int max_limit){
	if(is_rnd_already == 0)
		srand(time(NULL));
	is_rnd_already = 1;

	return min_limit + (rand()%(max_limit - min_limit +1));
}
/*using for verify malloc and calloc on read & make files actions*/
int verify_allocate_memory_succeded(void* ptr){
	if (ptr == NULL) {
		printf("cannot Allocate Memory \n");
		return ALLOCATE_MEMORY_FAILED;
	}
	return 1;
}
//parse int that is contain (obj_id , x ,y)
void parse_cuda_result(int result,int* obj ,int* x,int* y){
	*obj = (result & ID_MASK) >> ID_SHIFT;
	*x = (result & X_MASK) >> X_SHIFT;
	*y = (result & Y_MASK) >> Y_SHIFT;
}
//build int to contain (obj_id , x ,y), (On the same why Cuda return results)
int fill_result_match(int obj_id ,int x,int y){
	int result = 0;
	result = obj_id << ID_SHIFT;
	result |= x << X_SHIFT;
	result |= y << Y_SHIFT;
	result |=  ITS_MATCH;

	return result;
}

