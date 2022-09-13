/*
 * Omp_fuctions.h
 *
 * Goal:
 */

#ifndef OMP_FUNCTIONS_H_
#define OMP_FUNCTIONS_H_
#include "Data_Manager.h"

#define MASTER_OPEN_THREAD 2
#define SLAVE_OPEN_THREAD 4

/*
 * Verify if it's master or slave and call his task function
 */
void run_proccess_tasks(int my_rank ,Search_Data*  search_data);

/*
 * Execute master tasks
 */
void run_master_tasks(Search_Data*  search_data);

/*
 * Execute slave tasks
 */
void run_slave_tasks(Search_Data*  search_data);


void find_big_pictures_last_checks(FILE* fp,int* pictures_to_check, ResultArrDetails* resultArr_details ,Search_Data* search_data );



/*----------------   Deprecated -----------
All the functions below together make a full calculate for Pictures and Objects and return a int* with all the data
but was deprecated because of a lower efficiency
the program didn't use them
-------------------------------------------*/
int* find_matches_by_omp(Search_Data* search_data,ResultArrDetails* resultArr_details);
int is_picture_match_object_by_omp(Picture* picture, Picture* object , float matching);
int is_match_by_omp(BYTE* picture_elements, int picture_dim, int x, int y,BYTE* object_elements ,int object_dim, float matching);

#endif /* OMP_FUNCTIONS_H_ */
