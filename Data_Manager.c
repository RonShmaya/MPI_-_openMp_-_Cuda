/*
 * Data_Manager.c
 *
 *  Created on: 11 Jul 2022
 *      Author: linuxu
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Data_Manager.h"
#include "Input_Output_Manager.h"
#include "Mpi_functions.h"

/*All Program Options Expected From Argv*/

/*Option Serial*/
const char OPTION_PROGRAM_RUN_SERIAL[] = "SERIAL";
/*Option Parallel One PC*/
const char OPTION_PROGRAM_RUN_PARALLEL_ONE_PC[] = "ONE_PC";
/*Option Parallel Two PC*/
const char OPTION_PROGRAM_RUN_PARALLEL_TWO_PC[] = "TWO_PC";

/*Contain program option from Argv*/
char program_option[ARGV_STR_SIZE];
/*only for one pc parallel oprion*/
int is_need_to_sychronize_cuda_between_p = 0;

int picture_max_dim = -1;
int object_max_dim = -1;

void read_picture_from_file(FILE* fp,Picture* picture, int isPicture){
	fscanf(fp ,"%d\n", &(picture->id));
	fscanf(fp ,"%d\n", &(picture->dimention));

	picture->elements = (BYTE*) malloc(picture->dimention * picture->dimention * sizeof(BYTE));
	allocate_memory_status(fp , picture->elements);

	for (int i = 0; i < (picture->dimention * picture->dimention)-1; i++) {
			fscanf(fp ,"%hhu ", &(picture->elements[i]));
	}
	fscanf(fp ,"%hhu\n", &(picture->elements[(picture->dimention * picture->dimention)-1]));

	if(isPicture == 1)
		update_picture_max_dim(picture->dimention);
	else
		update_object_max_dim(picture->dimention);
}



void read_data_from_file(FILE* fp,Search_Data* search_data){
	if(fp == NULL){
		printf("cannot get data from null pointer file\n");
		exit(0);
	}

	fscanf(fp, "%f\n", &(search_data->matching));
	fscanf(fp, "%d\n", &(search_data->num_of_pictures));


	search_data->pictures = (Picture*) malloc(search_data->num_of_pictures * sizeof(Picture));
	allocate_memory_status(fp , search_data->pictures);

	for (int myStruct = 0; myStruct < search_data->num_of_pictures; myStruct++) {
		read_picture_from_file(fp , &(search_data->pictures[myStruct]), 1);
	}

	fscanf(fp, "%d\n", &(search_data->num_of_objects));

	search_data->objects = (Picture*) malloc(search_data->num_of_objects * sizeof(Picture));
	allocate_memory_status(fp , search_data->pictures);
	for (int myStruct = 0; myStruct < search_data->num_of_objects; myStruct++) {
		read_picture_from_file(fp , &(search_data->objects[myStruct]), 0);
	}

	printf("\n===========\t MASTER -MSG-: File Data is\n\n\t\t\tMatching = %f\n\t\t\tNum Of Pictures = %d\n"
			"\t\t\tNum Of Objects = %d\n\t\t\tDim Picture Max = %d\n\t\t\tDim Object Max = %d\n\n"
			, search_data->matching, search_data->num_of_pictures, search_data->num_of_objects, picture_max_dim, object_max_dim);

	if(strcmp(OPTION_PROGRAM_RUN_PARALLEL_ONE_PC , program_option) == 0  &&
			(search_data->num_of_pictures * search_data->num_of_objects * picture_max_dim) > SIZE_OF_DATA_ONE_COMPUTER_CUDA_WAIT_BETWEEN_PROCESSES){
		printf("\n===========\t MASTER -CHANGE RUN-: One PC With a Big Data, Cuda work only in one processes every time, to Rise efficiency and Avoid Out Of Memory\n");
		is_need_to_sychronize_cuda_between_p = 1;
	}

}


void free_picture(Picture* picture){
	free(picture->elements);
}
void free_search_data(Search_Data* search_data){
	for (int i = 0; i < search_data->num_of_pictures; i++) {
		free_picture(&(search_data->pictures[i]));
	}
	free(search_data->pictures);

	for (int i = 0; i < search_data->num_of_objects; i++) {
		free_picture(&(search_data->objects[i]));
	}
	free(search_data->objects);
}

void set_program_option(const char* option){
	strcpy(program_option, option);
}
int get_if_need_to_sychronize_cuda_between_p(){
	return is_need_to_sychronize_cuda_between_p;
}
void set_if_need_to_sychronize_cuda_between_p(int is_needed){
	is_need_to_sychronize_cuda_between_p = is_needed;
}
void update_picture_max_dim(int dim){
	picture_max_dim = dim > picture_max_dim ? dim : picture_max_dim;
}
void update_object_max_dim(int dim){
	object_max_dim = dim > object_max_dim ? dim : object_max_dim;
}
int get_picture_max_dim(){
	return picture_max_dim;
}
void set_picture_max_dim(int dim){
	picture_max_dim = dim;
}
int get_object_max_dim(){
	return object_max_dim;
}
void set_object_max_dim(int dim){
	object_max_dim = dim;
}
