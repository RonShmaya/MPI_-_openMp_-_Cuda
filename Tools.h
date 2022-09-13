/*
 * Tools.h
 *
 *  Created on: 11 Jul 2022
 *      Author: linuxu
 */

#ifndef TOOLS_H_
#define TOOLS_H_
#define ALLOCATE_MEMORY_FAILED -1

#define ID_SHIFT  22
#define X_SHIFT 12
#define Y_SHIFT 2


#define ITS_MATCH 1
#define NOT_MATCH 0

#define ID_MASK 0xFFC00000
#define X_MASK 0x3FF000
#define Y_MASK 0xFFC

/*using for make input file*/
float get_rnd_between_one_to_zero();
/*using for make input file*/
int get_rnd(int min_limit,int max_limit);
/*using for verify malloc and calloc on read & make files actions*/
int verify_allocate_memory_succeded(void* ptr);
//parse int that is contain (obj_id , x ,y)
void parse_cuda_result(int result,int* obj ,int* x,int* y);
//build int to contain (obj_id , x ,y), (On the same why Cuda return results)
int fill_result_match(int obj_id ,int x,int y);
#endif /* TOOLS_H_ */


