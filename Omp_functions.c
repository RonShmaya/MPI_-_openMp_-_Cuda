/*
 * Omp_fuctions.c
 *
 *  Created on: 20 Jul 2022
 *      Author: linuxu
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <string.h>
#include "Omp_functions.h"
#include "Mpi_functions.h"
#include "Cuda_functions.h"
#include "Tools.h"

extern const char OPTION_PROGRAM_RUN_PARALLEL_ONE_PC[];

/*
 * Verify if it's master or slave and call his task function
 */
void run_proccess_tasks(int my_rank ,Search_Data*  search_data){
	if(my_rank == MASTER)
		run_master_tasks(search_data);
	else if(my_rank == SLAVE)
		run_slave_tasks(search_data);
	else{
		printf("Wrong rank -> rank = %d",my_rank);
		MPI_Abort(MPI_COMM_WORLD, __LINE__);
		}
}
void run_master_tasks(Search_Data*  search_data){
	double prog_start,prog_end;
	prog_start = MPI_Wtime();


	int* master_results; /*Master result details*/
	int* slave_results=NULL;/*Slave result details*/

	clock_t start, end;

	/*Master result details*/
	ResultArrDetails master_resultArr_details = {
			  (search_data->num_of_pictures - search_data->num_of_pictures/NUM_OF_P),
			   search_data->num_of_objects,
			   search_data->num_of_pictures/NUM_OF_P,
			   get_picture_max_dim()};

	  /*Slave result details*/
	  ResultArrDetails slave_resultArr_details = {
			  (search_data->num_of_pictures / NUM_OF_P),
			   search_data->num_of_objects,
			   0,
			   get_picture_max_dim()};

/*
 * Master make parallel tasks: Task1 shared data to slave, Task2: calculate his picture   ---> parallel
 * Wait they finished
 * Master make parallel tasks more tasks, Task2: parse his result & save to file, Task3: get data results from slave */
#pragma omp parallel
	{
		#pragma omp single
		{
			#pragma omp task private(start, end)
			{
		    	start = clock();
		    	printf("\n===========\t MASTER -TASK-: Shared Data With Slave\n");
				shared_search_data(search_data);
				end = clock();
				printf("\n===========\t MASTER -TASK FINISHED-: Shared Data With Slave, took %1.3f sec\n",((double) (end - start)) / CLOCKS_PER_SEC);

			}
			#pragma omp task private(start, end)
			{
				start = clock();
				printf("\n===========\t MASTER -TASK-: Calculate Matches By Cuda\n");
				master_results = find_matches_by_cuda(search_data, search_data->num_of_pictures/NUM_OF_P , search_data->num_of_pictures);
				end = clock();
				printf("\n===========\t MASTER -TASK FINISHED-: Calculate Matches By Cuda, took %1.3f sec\n",((double) (end - start)) / CLOCKS_PER_SEC);


			}

			#pragma omp taskwait

			#pragma omp task private(start, end)
			{
			/*In some cases of big data on one PC it's necessary to verify each P use Cuda kernel not on the same time*/
			if(get_if_need_to_sychronize_cuda_between_p() == 1)
					tell_slave_master_finished_with_cuda();
    		  start = clock();
			  printf("\n===========\t MASTER -TASK-: Parsing Data & Write To File\n");
			  parse_all_data_and_save_to_file(master_results, &master_resultArr_details, &slave_results , &slave_resultArr_details, search_data );
			  end = clock();
			  printf("\n===========\t MASTER -TASK FINISHED-: Parsing Data & Write To File, took %1.3f sec\n",((double) (end - start)) / CLOCKS_PER_SEC);
			}
			#pragma omp task private(start, end)
			{
			  start = clock();
			  printf("\n===========\t MASTER -TASK-: Waiting Get Data From Slave\n");
			  shared_results(&slave_results,&slave_resultArr_details);
			  end = clock();
			  printf("\n===========\t MASTER -TASK FINISHED-: Waiting Get Data From Slave, took %1.3f sec\n",((double) (end - start)) / CLOCKS_PER_SEC);
			}
		}
	}
	free(master_results);
	free(slave_results);
	prog_end = MPI_Wtime();
	printf("\n\n\n===========\t MASTER -FINISHED PROGRAM-: execute time took (not include create %s and reading %s)  -   %1.3f sec    \n\n",INPUT_FILE_NAME,INPUT_FILE_NAME,((double) (prog_end - prog_start)));
}
void run_slave_tasks(Search_Data*  search_data){
	int* slave_results=NULL;
		clock_t start, end;

		start = clock();
		printf("\n###########\t SLAVE -TASK-: Waiting Get Data From Master\n");
		shared_search_data(search_data);
		end = clock();
		printf("\n###########\t SLAVE -TASK FINISHED-: Waiting Get Data From Master, took %1.3f sec\n",((double) (end - start)) / CLOCKS_PER_SEC);

		/*Slave result details*/
		 ResultArrDetails slave_resultArr_details = {
				  (search_data->num_of_pictures),
				   search_data->num_of_objects,
				   0,
				   get_picture_max_dim()};

		start = clock();
		/*In some cases of big data on one PC it's necessary to verify each P use Cuda kernel not on the same time*/
		if(get_if_need_to_sychronize_cuda_between_p() == 1)
			tell_slave_master_finished_with_cuda();

		printf("\n###########\t SLAVE -TASK-: Calculate Matches By Cuda\n");
		slave_results = find_matches_by_cuda(search_data, 0 , search_data->num_of_pictures );
		end = clock();
		printf("\n###########\t SLAVE -TASK FINISHED-: Calculate Matches By Cuda, took %1.3f sec\n",((double) (end - start)) / CLOCKS_PER_SEC);


		start = clock();
		printf("\n###########\t SLAVE -TASK-: Shared Data With Master\n");
		shared_results(&slave_results,&slave_resultArr_details);
		end = clock();
		printf("\n###########\t SLAVE -TASK FINISHED-: Shared Data With Master, took %1.3f sec\n",((double) (end - start)) / CLOCKS_PER_SEC);

		free(slave_results);
}

/*In case master have pictures dim > 1024*1024 and they didn't found object in the indexes < 1024*1024, Calculate the rest using OpenMP*/
void find_big_pictures_last_checks(FILE* fp,int* pictures_to_check, ResultArrDetails* resultArr_details ,Search_Data* search_data ){
#pragma omp parallel for
	for (int picture = 0; picture < resultArr_details->num_of_pictures; ++picture) {
		if(pictures_to_check[picture] == NEED_TO_CHECK){
			int is_match = 0;
			for (int object = 0; object < search_data->num_of_objects; ++object) {
				int pic_dim = search_data->pictures[picture + resultArr_details->jump_picture_index].dimention;
				int obj_dim = search_data->objects[object].dimention;
				if(obj_dim > pic_dim)
					break;
				//the number of searching the Picture has
				int num_of_searching_in_one_axis = pic_dim - CUDA_MAX_THREAD;
				//the first index the picture will start to search for matches
				int index_searching = CUDA_MAX_THREAD - obj_dim + 1;

				for (int x = index_searching; x < index_searching + num_of_searching_in_one_axis; ++x) {
					for (int y = index_searching; y < index_searching + num_of_searching_in_one_axis; ++y) {
						float delta = 0;
						float tmp = 0;
						// check for match between Picture and Object
						for (int elem_x = x; elem_x < x+obj_dim; ++elem_x) {
							for (int elem_y = y; elem_y < y+obj_dim; ++elem_y) {
								tmp = (search_data->pictures[picture + resultArr_details->jump_picture_index].elements[elem_x*pic_dim + elem_y]
										- search_data->objects[object].elements[(elem_x-x)*obj_dim + (elem_y-y)])
												/(float)(search_data->pictures[picture + resultArr_details->jump_picture_index].elements[elem_x*pic_dim + elem_y]);
								delta += tmp < 0 ? tmp * -1 : tmp;

								if(delta > search_data->matching)
									break;
							}

						}
						//It's not match, no need to make more searching on this point x , y
						if(delta <= search_data->matching){
							fprintf(fp,"Picture %d found Object %d in Position(%d,%d)\n",(picture + resultArr_details->jump_picture_index), object, x ,y);
							is_match = 1;
							break;

						}
					}
					//It's match, no need to make more searching on Picture
					if(is_match == ITS_MATCH)
						break;
				}
				//It's match, no need to make more searching on Picture
				if(is_match == ITS_MATCH)
					break;
			}
			//It's not match
			if(is_match != ITS_MATCH)
				fprintf(fp,"Picture %d No Objects were found\n",(picture + resultArr_details->jump_picture_index));
		}
	}
}

/*----------------   Deprecated -----------
All the functions below together make a full calculate for Pictures and Objects and return a int* with all the data
but was deprecated because of a lower efficiency
the program didn't use its
-------------------------------------------*/
int* find_matches_by_omp(Search_Data* search_data,ResultArrDetails* resultArr_details){

	long result_size = resultArr_details->num_of_pictures * search_data->num_of_objects * get_picture_max_dim();

	int* results = (int*) calloc(result_size , sizeof(int));

	int size_of_picture_block = resultArr_details->num_of_objects * resultArr_details->elements_max_size;

	int num_thread = strcmp(OPTION_PROGRAM_RUN_PARALLEL_ONE_PC , program_option) == 0 ? 2 : 4;


#pragma omp parallel for num_threads(num_thread)
	for (int picture = 0; picture < resultArr_details->num_of_pictures; picture++) {
		for (int res = 0; res < size_of_picture_block; res++) {
			int ans = is_picture_match_object_by_omp(
					&(search_data->pictures[picture]) ,
					&(search_data->objects[res / resultArr_details->elements_max_size]),
					search_data->matching);

			if(ans &  ITS_MATCH == ITS_MATCH){
				results[(picture * size_of_picture_block)] = ans;
				break;
			}

		}

	}
	return results;
}
int is_picture_match_object_by_omp(Picture* picture, Picture* object , float matching){

	int pic_dim = picture->dimention;
	int obj_dim = object->dimention;
	if(obj_dim > pic_dim)
		return NOT_MATCH;

	int num_of_searching_in_one_axis = pic_dim - obj_dim + 1;

	for (int x = 0; x < num_of_searching_in_one_axis ; ++x)
		for (int y = 0; y < num_of_searching_in_one_axis ; ++y)
			if(is_match_by_omp(picture->elements, pic_dim, x, y, object->elements, obj_dim, matching) == ITS_MATCH)
				return fill_result_match(object->id, x, y);

	return NOT_MATCH;
}
int is_match_by_omp(BYTE* picture_elements, int picture_dim, int x, int y,BYTE* object_elements ,int object_dim, float matching){
	float delta = 0;
	float tmp;
	for (int row = 0; row < object_dim; ++row) {
		for (int col = 0; col < object_dim; ++col) {
			tmp = (picture_elements[ ((x+row)*picture_dim)  +  y+col] - object_elements[row*object_dim+col]) /(float)
																(picture_elements[ ((x+row)*picture_dim)  +  y+col]);
			delta+= tmp < 0 ? tmp * -1 : tmp;
			if(delta > matching)
				return NOT_MATCH;
		}
	}
	return ITS_MATCH;
}
