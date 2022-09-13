
#ifndef DATA_MANAGER_H_
#define DATA_MANAGER_H_
#include <stdio.h>
#define NUM_OF_P 2


#define NUM_OF_PICTURES_MAX 100
#define NUM_OF_PICTURES_MIN 85
#define NUM_OF_OBJECTS_MAX 100
#define NUM_OF_OBJECTS_MIN 85
#define SIZE_OF_ELEMENT_MAX 1000
#define SIZE_OF_ELEMENT_MIN 600
#define ELEMENT_VALUE_MAX 100
#define ELEMENT_VALUE_MIN 1
#define PICTURE_OBJECT_SIZE_RELATION 20


#define SIZE_OF_DATA_ONE_COMPUTER_CUDA_WAIT_BETWEEN_PROCESSES 625000
#define CUDA_MAX_THREAD 1024

#define NEED_TO_CHECK 1

#define ARGV_STR_SIZE 7
#define ARGC_EXPECTED_SIZE 2

typedef unsigned char BYTE;

typedef struct{
	int id;
	int dimention;
	BYTE* elements;
}Picture;

void read_picture_from_file(FILE* fp,Picture* picture, int isPicture);
void free_picture(Picture* picture);



typedef struct{

	float matching;

	int num_of_pictures;
	int num_of_objects;

	Picture* pictures;
	Picture* objects;

}Search_Data;
void read_data_from_file(FILE* fp,Search_Data* search_data);
void free_search_data(Search_Data* search_data);

typedef struct{
	int num_of_pictures;
	int num_of_objects;
	int jump_picture_index;
	int elements_max_size;

}ResultArrDetails;


void set_program_option(const char* option);
int get_if_need_to_sychronize_cuda_between_p();
void set_if_need_to_sychronize_cuda_between_p(int is_needed);

void update_picture_max_dim(int dim);
void update_object_max_dim(int dim);
int get_picture_max_dim();
void set_picture_max_dim(int dim);
int get_object_max_dim();
void set_object_max_dim(int dim);


#endif /* DATA_MANAGER_H_ */
