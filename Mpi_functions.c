/*
 * Mpi_functions.c
 *
 *  Created on: 13 Jul 2022
 *      Author: linuxu
 */

#include <stdlib.h>
#include <string.h>
#include "Omp_functions.h"
#include "Mpi_functions.h"
#include "Cuda_functions.h"
#include "Run_Options.h"
#include "Input_Output_Manager.h"
#include "Tools.h"


int my_rank;
int p;
int is_slave_send_result = 0;

void init_my_mpi(int* argc, char** argv[]){
	/* start up MPI */
	MPI_Init(argc, argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	if(p != NUM_OF_P){
		printf("please make %d processes", NUM_OF_P);
		MPI_Abort(MPI_COMM_WORLD, __LINE__);
	}
}

/*Master make a input file if needed + read file data using Input_Output_Manager*/
void master_prepared_data(Search_Data* search_data){
	if(my_rank != MASTER)
		return;
	double t1,t2;

	printf("\n===========\t MASTER -Work On- : Create File %s If Needed\n",INPUT_FILE_NAME);
	t1 = MPI_Wtime();
	make_input_file_if_needed();
	t2 = MPI_Wtime();
	printf("\n===========\t MASTER -FINISHED- : Create File %s If Needed, took  %1.3f seconds\n",INPUT_FILE_NAME,t2-t1);

	printf("\n===========\t MASTER -Work On- : Read File Data\n");
	t1 = MPI_Wtime();
	read_input_file(search_data);
	t2 = MPI_Wtime();
	printf("\n===========\t MASTER -FINISHED- : Read File Data, took  %1.3f seconds\n",t2-t1);
}
/*check which P is call the function,
 * Master send data and slave  get data,
 defined a new MPI data types*/
void shared_search_data(Search_Data* search_data){
	MPI_Datatype Picture_mpi;
	MPI_Datatype Search_Data_mpi;
	MPI_Status status;

	defined_search_data_struct(&Picture_mpi, &Search_Data_mpi);

	if(my_rank == MASTER)
		master_send_shared_search_data(search_data, &Picture_mpi, &Search_Data_mpi);

	else if(my_rank == SLAVE)
		slave_get_shared_search_data(search_data, &Picture_mpi, &Search_Data_mpi);

	else{
		printf("Wrong rank -> rank = %d",my_rank);
		MPI_Abort(MPI_COMM_WORLD, __LINE__);
	}
}
void slave_get_shared_search_data(Search_Data* search_data, MPI_Datatype* Picture_mpi, MPI_Datatype* Search_Data_mpi){
	MPI_Status status;

	int data_details[3];
	MPI_Recv(data_details, 3 ,  MPI_INT, MASTER, 0, MPI_COMM_WORLD, &status);

	set_if_need_to_sychronize_cuda_between_p(data_details[0]);
	set_picture_max_dim(data_details[1]);
	set_object_max_dim(data_details[2]);

	/*slave get search data struct*/
	MPI_Recv(search_data, 1 ,  *Search_Data_mpi, MASTER, 0, MPI_COMM_WORLD, &status);

	/*slave will get num_of_pictures / NUM_OF_P */
	search_data->num_of_pictures = search_data->num_of_pictures / NUM_OF_P;

	search_data->pictures =  (Picture*) malloc(search_data->num_of_pictures * sizeof(Picture));
	if(verify_allocate_memory_succeded(search_data->pictures) == ALLOCATE_MEMORY_FAILED) MPI_Abort(MPI_COMM_WORLD, __LINE__);


	search_data->objects =  (Picture*) malloc(search_data->num_of_objects * sizeof(Picture));
	if(verify_allocate_memory_succeded(search_data->objects) == ALLOCATE_MEMORY_FAILED) MPI_Abort(MPI_COMM_WORLD, __LINE__);

	/*slave get pictures struct*/
	MPI_Recv(search_data->pictures, search_data->num_of_pictures ,  *Picture_mpi, MASTER, 0, MPI_COMM_WORLD, &status);

	/*slave get objects struct*/
	MPI_Recv(search_data->objects, search_data->num_of_objects ,  *Picture_mpi, MASTER, 0, MPI_COMM_WORLD, &status);

	/*Will get all Pictures elements (send in separate because you cannot send struct pointers variables )*/
	long max_elements = ((search_data->num_of_pictures) * (get_picture_max_dim() * get_picture_max_dim()));
	BYTE* all_picture_elements_data_max = (BYTE*) malloc(sizeof(BYTE)*max_elements); //All pictures element
	if(verify_allocate_memory_succeded(all_picture_elements_data_max) == ALLOCATE_MEMORY_FAILED) MPI_Abort(MPI_COMM_WORLD, __LINE__);
	MPI_Recv(all_picture_elements_data_max, max_elements ,  MPI_UNSIGNED_CHAR, MASTER, 0, MPI_COMM_WORLD, &status);

	/*every picture takes his own data*/
	long all_data_index = 0;
	for (int picture = 0; picture < search_data->num_of_pictures; ++picture) {
		int pic_dim = search_data->pictures[picture].dimention;
		search_data->pictures[picture].elements = (BYTE*) malloc(pic_dim * pic_dim * sizeof(BYTE));
		if(verify_allocate_memory_succeded(search_data->pictures[picture].elements) == ALLOCATE_MEMORY_FAILED) MPI_Abort(MPI_COMM_WORLD, __LINE__);
		for (int element = 0; element < pic_dim * pic_dim; ++element){
			search_data->pictures[picture].elements[element] = all_picture_elements_data_max[all_data_index++];
		}
	}

	free(all_picture_elements_data_max);

	/*Will get all objects elements (send in separate because you cannot send struct pointers variables )*/
	max_elements = search_data->num_of_objects * ( get_object_max_dim() * get_object_max_dim() );
	BYTE* all_objects_elements_data_max = (BYTE*) malloc(sizeof(BYTE)*max_elements);
	if(verify_allocate_memory_succeded(all_objects_elements_data_max) == ALLOCATE_MEMORY_FAILED) MPI_Abort(MPI_COMM_WORLD, __LINE__);
	MPI_Recv(all_objects_elements_data_max, max_elements ,  MPI_UNSIGNED_CHAR, MASTER, 0, MPI_COMM_WORLD, &status);

	/*every object takes his own data*/
	all_data_index = 0;
	for (int obj = 0; obj < search_data->num_of_objects; ++obj) {
		int obj_dim = search_data->objects[obj].dimention;
		search_data->objects[obj].elements = (BYTE*) malloc(obj_dim * obj_dim * sizeof(BYTE));
		if(verify_allocate_memory_succeded(search_data->objects[obj].elements) == ALLOCATE_MEMORY_FAILED) MPI_Abort(MPI_COMM_WORLD, __LINE__);
		for (int element = 0; element < obj_dim * obj_dim; ++element){
			search_data->objects[obj].elements[element] = all_objects_elements_data_max[all_data_index++];
		}
	}


	free(all_objects_elements_data_max);


}
void master_send_shared_search_data(Search_Data* search_data, MPI_Datatype* Picture_mpi, MPI_Datatype* Search_Data_mpi){

	int data_details[3] = {get_if_need_to_sychronize_cuda_between_p(),get_picture_max_dim() , get_object_max_dim()};
	MPI_Send(data_details, 3 ,  MPI_INT, SLAVE, 0, MPI_COMM_WORLD);

	/*Master send search data struct*/
	MPI_Send(search_data, 1 ,  *Search_Data_mpi, SLAVE, 0, MPI_COMM_WORLD);
	/*Master send Pictures struct*/
	MPI_Send(search_data->pictures, (search_data->num_of_pictures / NUM_OF_P) ,  *Picture_mpi, SLAVE, 0, MPI_COMM_WORLD);
	/*Master send Objects struct*/
	MPI_Send(search_data->objects, search_data->num_of_objects ,  *Picture_mpi, SLAVE, 0, MPI_COMM_WORLD);

	/*Send all pictures data in one array (it's saving time, less sends action)*/
	BYTE* all_picture_elements_data_max = (BYTE*) malloc(sizeof(BYTE)*
			((search_data->num_of_pictures/NUM_OF_P) * (get_picture_max_dim() * get_picture_max_dim())));
	if(verify_allocate_memory_succeded(all_picture_elements_data_max) == ALLOCATE_MEMORY_FAILED) MPI_Abort(MPI_COMM_WORLD, __LINE__);

	long num_of_elements = 0;

	for (int picture = 0; picture < (search_data->num_of_pictures / NUM_OF_P); picture++) {
		int pic_dim = search_data->pictures[picture].dimention;
		for (int element = 0; element < (pic_dim * pic_dim); element++) {
			all_picture_elements_data_max[num_of_elements++] = search_data->pictures[picture].elements[element];
		}
	}

	MPI_Send(all_picture_elements_data_max, num_of_elements ,  MPI_UNSIGNED_CHAR, SLAVE, 0, MPI_COMM_WORLD);

	free(all_picture_elements_data_max);

	/*Send all objects data in one array (it's saving time, less sends action)*/
	BYTE* all_objects_elements_data_max = (BYTE*) malloc(sizeof(BYTE)*
				(search_data->num_of_objects * (get_object_max_dim() * get_object_max_dim())));
	if(verify_allocate_memory_succeded(all_objects_elements_data_max) == ALLOCATE_MEMORY_FAILED) MPI_Abort(MPI_COMM_WORLD, __LINE__);

	num_of_elements = 0;
	for (int obj = 0; obj < search_data->num_of_objects; obj++) {
		int pic_dim = search_data->objects[obj].dimention;
		for (int element = 0; element < (pic_dim * pic_dim); element++) {
			all_objects_elements_data_max[num_of_elements++] = search_data->objects[obj].elements[element];
		}
	}

	MPI_Send(all_objects_elements_data_max, num_of_elements ,  MPI_UNSIGNED_CHAR, SLAVE, 0, MPI_COMM_WORLD);

	free(all_objects_elements_data_max);

}
/*Defined Search data and Picture structs to MPI
 define an empty size for pointer (we can't really send pointers)*/
void defined_search_data_struct(MPI_Datatype* Picture_mpi, MPI_Datatype* Search_Data_mpi){
	Picture picture;

	MPI_Datatype picture_vals_types[3] = {MPI_INT , MPI_INT, MPI_UNSIGNED_CHAR }; //struct Data types
	int vals_size[3] = {1 , 1, 8};  //struct each variable  amount
	MPI_Aint picture_disp[3]; //struct each variable  distance
	picture_disp[0] = (char*) &picture.id - (char*)&picture;
	picture_disp[1] = (char*) &picture.dimention - (char*)&picture;
	picture_disp[2] = (char*) &picture.elements - (char*)&picture;
	MPI_Type_create_struct(3, vals_size, picture_disp, picture_vals_types, Picture_mpi);
	MPI_Type_commit(Picture_mpi);

	Search_Data search_data;

	MPI_Datatype search_vals_types[3] = {MPI_FLOAT, MPI_INT , MPI_INT };
	int search_vals_size[3] = {1, 1, 1};
	MPI_Aint search_disp[3];
	search_disp[0] = (char*) &search_data.matching - (char*)&search_data;
	search_disp[1] = (char*) &search_data.num_of_pictures - (char*)&search_data;
	search_disp[2] = (char*) &search_data.num_of_objects - (char*)&search_data;
	MPI_Type_create_struct(3 , search_vals_size, search_disp, search_vals_types, Search_Data_mpi);
	MPI_Type_commit(Search_Data_mpi);
}

void parse_all_data_and_save_to_file(int* master_results, ResultArrDetails* master_resultArr_details, int** slave_results , ResultArrDetails* slave_resultArr_details,Search_Data* search_data ){
	FILE *fp;

	if ((fp = fopen(OUTPUT_FILE_NAME, "w")) == NULL) {
		printf("cannot open file %s for writing\n", OUTPUT_FILE_NAME);
		MPI_Abort(MPI_COMM_WORLD, __LINE__);
	}
	int* pictures_need_more_searching_master = NULL;
	int* pictures_need_more_searching_slave = NULL;

	/*Master results - Parse & Save*/
	pictures_need_more_searching_master = parse_part_of_data_and_save_to_file(fp,master_results, master_resultArr_details, search_data);
	/*wait slave send result data*/
	while(is_slave_send_result == 0);
	/*Slave results - Parse & Save*/
	pictures_need_more_searching_slave = parse_part_of_data_and_save_to_file(fp,*slave_results, slave_resultArr_details, search_data);

	/*In case master have pictures dim > 1024*1024 and they didn't found object in the indexes < 1024*1024, Calculate the rest using OpenMP*/
	if(pictures_need_more_searching_master != NULL){
		 printf("\n===========\t MASTER -TASK MSG-: Parsing Data & Write To File, -MSG- need to using OpenMP for searching last indexes in Master big pictures \n");
		 find_big_pictures_last_checks(fp, pictures_need_more_searching_master,master_resultArr_details ,search_data);
		 free(pictures_need_more_searching_master);
	}
	/*In case slave have pictures dim > 1024*1024 and they didn't found object in the indexes < 1024*1024, Calculate the rest using OpenMP*/
	if(pictures_need_more_searching_slave != NULL){
		printf("\n===========\t MASTER -TASK MSG-: Parsing Data & Write To File, -MSG- need to using OpenMP for searching last indexes in Slave big pictures \n");
		find_big_pictures_last_checks(fp, pictures_need_more_searching_slave,slave_resultArr_details ,search_data);
		free(pictures_need_more_searching_slave);

	}
	fclose(fp);

}

int* parse_part_of_data_and_save_to_file(FILE* fp,int* results, ResultArrDetails* resultArr_details,Search_Data* search_data){
	int is_match = 0;
	int cuda_parse_style = resultArr_details->elements_max_size;
	int* pictures_need_more_searching = NULL;
	//In case We may be need use OpenMP for big Pictures
	if(get_picture_max_dim() > CUDA_MAX_THREAD){
		cuda_parse_style = CUDA_MAX_THREAD;
		pictures_need_more_searching = (int*)calloc(resultArr_details->num_of_pictures, sizeof(int));
		if(verify_allocate_memory_succeded(pictures_need_more_searching) == ALLOCATE_MEMORY_FAILED) MPI_Abort(MPI_COMM_WORLD, __LINE__);
	}
	// results jumps Picture
	int size_of_picture_block = resultArr_details->num_of_objects * cuda_parse_style;
	int obj = 0,x = 0, y = 0;
#pragma omp parallel for firstprivate(obj, x, y, is_match)
	for (int picture = 0; picture < resultArr_details->num_of_pictures; picture++) {
		for (int res = 0; res < size_of_picture_block; res++) {
			if(results[(picture * size_of_picture_block) + res] & ITS_MATCH == ITS_MATCH ){
				parse_cuda_result(results[(picture * size_of_picture_block) + res], &obj , &x, &y); //Parse result
				is_match = ITS_MATCH;
				break;
			}
		}
		#pragma omp critical
		{
			if(is_match == ITS_MATCH)
				fprintf(fp,"Picture %d found Object %d in Position(%d,%d)\n",(picture + resultArr_details->jump_picture_index), obj, x ,y);
			else if(search_data->pictures[picture + resultArr_details->jump_picture_index].dimention <= CUDA_MAX_THREAD)
				fprintf(fp,"Picture %d No Objects were found\n",(picture + resultArr_details->jump_picture_index));
			else
				pictures_need_more_searching[picture] = NEED_TO_CHECK;

		}
		is_match = 0;
	}
	return pictures_need_more_searching;
}
/*Shared results between Master and Slave*/
void shared_results(int** results, ResultArrDetails* slave_resultArr_details){
	if(my_rank == MASTER)
		master_get_results(results, slave_resultArr_details);
	else if(my_rank == SLAVE)
		slave_send_result(results, slave_resultArr_details);
	else{
		printf("Wrong rank -> rank = %d",my_rank);
		MPI_Abort(MPI_COMM_WORLD, __LINE__);
	}
}
/*master get results from slave*/
void master_get_results(int** results,ResultArrDetails* slave_resultArr_details){
	MPI_Status status;
	long size = slave_resultArr_details->num_of_pictures *
				slave_resultArr_details->num_of_objects *
				slave_resultArr_details->elements_max_size;

	*results = (int*) calloc(size , sizeof(int));
	if(verify_allocate_memory_succeded(*results) == ALLOCATE_MEMORY_FAILED) MPI_Abort(MPI_COMM_WORLD, __LINE__);
	MPI_Recv(*results, size ,  MPI_INT, SLAVE, 0, MPI_COMM_WORLD, &status);

	is_slave_send_result = 1;
}
/*send send results to master*/
void slave_send_result(int** results,ResultArrDetails* slave_resultArr_details){
	long size = slave_resultArr_details->num_of_pictures *
				slave_resultArr_details->num_of_objects *
				slave_resultArr_details->elements_max_size;

	MPI_Send(*results, size ,  MPI_INT, MASTER, 0, MPI_COMM_WORLD);

}
//In case Master need to tell slave he finished with Cuda work
void tell_slave_master_finished_with_cuda(){
	MPI_Status status;
	char empty = 0;
	if(my_rank == MASTER)
		MPI_Send(&empty, 1 ,  MPI_CHAR, SLAVE, 0, MPI_COMM_WORLD);
	else if(my_rank == SLAVE)
		MPI_Recv(&empty, 1 ,  MPI_CHAR, MASTER, 0, MPI_COMM_WORLD, &status);
	else{
		printf("Wrong rank -> rank = %d",my_rank);
		MPI_Abort(MPI_COMM_WORLD, __LINE__);
	}
}
void close_mpi(){
	MPI_Finalize();
}
int get_my_rank(){
	return my_rank;
}
