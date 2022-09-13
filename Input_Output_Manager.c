
#include <stdlib.h>
#include "Input_Output_Manager.h"
#include "Tools.h"
#include "Mpi_functions.h"


void make_input_file_if_needed(){
	FILE *fp;

	if ((fp = fopen(INPUT_FILE_NAME, "r")) != NULL) {
		printf("\n===========\t MASTER -MSG-: file %s is exists, no need to create one...\n", INPUT_FILE_NAME);
		return;
	}
	printf("\n===========\t MASTER -MSG-: file %s is not exists, start to creating one...\n", INPUT_FILE_NAME);

	make_input_file();
}
void make_input_file(){
	FILE *fp;

	if ((fp = fopen(INPUT_FILE_NAME, "w")) == NULL) {
		printf("cannot create file %s\n", INPUT_FILE_NAME);
		exit(0);
	}
	fprintf(fp ,"%f\n", (get_rnd_between_one_to_zero()/1000));

	write_structs_to_file(fp , NUM_OF_PICTURES_MIN , NUM_OF_PICTURES_MAX , 1);

	write_structs_to_file(fp , NUM_OF_OBJECTS_MIN , NUM_OF_OBJECTS_MAX, 0);

	fclose(fp);

	return;
}
void write_structs_to_file(FILE* fp, int min , int max,int isPicture){
	int num_of_structs = get_rnd(min, max);
	fprintf(fp ,"%d\n", num_of_structs);
	if(ELEMENT_VALUE_MAX > 255){
		printf("ELEMENT_VALUE_MAX must to be less then 255 (1 byte only) \n");
		fclose(fp);
		MPI_Abort(MPI_COMM_WORLD, __LINE__);
	}

	int min_element_size = isPicture == 1 ? SIZE_OF_ELEMENT_MIN : (SIZE_OF_ELEMENT_MIN/PICTURE_OBJECT_SIZE_RELATION);
	int max_element_size = isPicture == 1 ? SIZE_OF_ELEMENT_MAX : (SIZE_OF_ELEMENT_MAX/PICTURE_OBJECT_SIZE_RELATION);

	for (int myStruct = 0; myStruct < num_of_structs; myStruct++) {
		fprintf(fp ,"%d\n", myStruct);
		int dimention = get_rnd(min_element_size, max_element_size);
		fprintf(fp ,"%d\n", dimention);
		for (int element = 0; element < (dimention*dimention-1); element++) {
			fprintf(fp ,"%d ", get_rnd(ELEMENT_VALUE_MIN, ELEMENT_VALUE_MAX));
		}
		fprintf(fp ,"%d\n", get_rnd(ELEMENT_VALUE_MIN, ELEMENT_VALUE_MAX));
	}
}
void read_input_file(Search_Data* search_data){
	FILE *fp;

	if ((fp = fopen(INPUT_FILE_NAME, "r")) == NULL) {
		printf("cannot open file %s for reading\n", INPUT_FILE_NAME);
		MPI_Abort(MPI_COMM_WORLD, __LINE__);
	}

	read_data_from_file(fp, search_data);

	fclose(fp);
}

void allocate_memory_status(FILE* fp,void* ptr){
	if (ptr == NULL) {
		printf("cannot Allocate Memory \n");
		fclose(fp);
		MPI_Abort(MPI_COMM_WORLD, __LINE__);
	}
}
