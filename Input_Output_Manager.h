
#ifndef INPUT_OUTPUT_MANAGER_H_
#define INPUT_OUTPUT_MANAGER_H_

#include <stdio.h>
#include "Data_Manager.h"

#define INPUT_FILE_NAME "input.txt"
#define OUTPUT_FILE_NAME "output.txt"

//write
void make_input_file_if_needed();
void make_input_file();
void write_structs_to_file(FILE* fp, int min , int max,int isPicture);



//read
void read_input_file(Search_Data* search_data);

void allocate_memory_status(FILE* fp, void* ptr);

#endif /* INPUT_OUTPUT_MANAGER_H_ */
