
#ifndef MPI_FUNCTIONS_H_
#define MPI_FUNCTIONS_H_

#include <mpi.h>
#include "Data_Manager.h"

#define MASTER 0
#define SLAVE 1

extern const char OPTION_PROGRAM_RUN_SERIAL[];
extern const char OPTION_PROGRAM_RUN_PARALLEL_ONE_PC[];
extern const char OPTION_PROGRAM_RUN_PARALLEL_TWO_PC[];
extern char program_option[ARGV_STR_SIZE];

void init_my_mpi(int* argc, char** argv[]);

/*Master make a input file if needed + read file data using Input_Output_Manager*/
void master_prepared_data(Search_Data* search_data);

/*send Master to send data and slave to get data*/
void shared_search_data(Search_Data* search_data);

void slave_get_shared_search_data(Search_Data* search_data, MPI_Datatype* Picture_mpi, MPI_Datatype* Search_Data_mpi);
void master_send_shared_search_data(Search_Data* search_data, MPI_Datatype* Picture_mpi, MPI_Datatype* Search_Data_mpi);

/*Will parse the results we get from Cuda and save to file
 after the master finish to check his data the function wait until it get the slave data */
void parse_all_data_and_save_to_file(int* master_results, ResultArrDetails* master_resultArr_details, int** slave_results , ResultArrDetails* slave_resultArr_details,Search_Data* search_data );
/*Get result from Cuda and Parse and save to file */
int* parse_part_of_data_and_save_to_file(FILE* fp, int* results, ResultArrDetails* resultArr_details,Search_Data* search_data);

/*Shared results between Master and Slave -> Send each P to his function*/
void shared_results(int** results,ResultArrDetails* slave_resultArr_details);
/*master get results from slave*/
void master_get_results(int** results,ResultArrDetails* slave_resultArr_details);
/*send send results to master*/
void slave_send_result(int** results,ResultArrDetails* slave_resultArr_details);

/*Define search data and Picture structs*/
void defined_search_data_struct(MPI_Datatype* Picture_mpi, MPI_Datatype* Search_Data_mpi);
/*In case Master need to tell slave he finished with Cuda work*/
void tell_slave_master_finished_with_cuda();

void close_mpi();
int get_my_rank();




#endif /* MPI_FUNCTIONS_H_ */
