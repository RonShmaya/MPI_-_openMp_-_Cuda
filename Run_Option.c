#include <stdio.h>
#include <time.h>
#include "Mpi_functions.h"
#include "Omp_functions.h"
#include "Data_Manager.h"
#include "Tools.h"
#include "Input_Output_Manager.h"


/*------- Parallel Option ------*/
void run_parallel(){

	Search_Data search_data;

	/*Create file if needed + Read file*/
	master_prepared_data(&search_data);

	int my_rank = get_my_rank();

	/*Run P tasks*/
	run_proccess_tasks(my_rank , &search_data);

	/*Free struct search_data*/
	free_search_data(&search_data);

}

/*------- Serial Option --------*/
void run_serial(){
	Search_Data search_data;

	make_input_file_if_needed();
	read_input_file(&search_data);

	clock_t start, end;
	start = clock();

	FILE *fp;

	if ((fp = fopen(OUTPUT_FILE_NAME, "w")) == NULL) {
		printf("cannot open file %s for writing\n", OUTPUT_FILE_NAME);
		return;
	}

	for (int picture = 0; picture < search_data.num_of_pictures; ++picture) {
		int is_match = 0;
		for (int object = 0; object < search_data.num_of_objects; ++object) {
			int pic_dim = search_data.pictures[picture].dimention;
			int obj_dim = search_data.objects[object].dimention;
			int num_of_searching_in_one_axis = pic_dim - obj_dim + 1;

			for (int x = 0; x < num_of_searching_in_one_axis; ++x) {
				for (int y = 0; y < num_of_searching_in_one_axis; ++y) {
					float delta = 0;
					float tmp = 0;
					for (int elem_x = x; elem_x < x+obj_dim; ++elem_x) {
						for (int elem_y = y; elem_y < y+obj_dim; ++elem_y) {
							tmp = (search_data.pictures[picture].elements[elem_x*pic_dim + elem_y]
									- search_data.objects[object].elements[(elem_x-x)*obj_dim + (elem_y-y)])
											/(float)(search_data.pictures[picture].elements[elem_x*pic_dim + elem_y]);
							delta += tmp < 0 ? tmp * -1 : tmp;

							if(delta > search_data.matching)
								break;
						}

					}
					if(delta <= search_data.matching){
						fprintf(fp,"Picture %d found Object %d in Position(%d,%d)\n",picture, object, x ,y);
						is_match = 1;
						break;

					}
				}
				if(is_match == ITS_MATCH)
					break;
			}
			if(is_match == ITS_MATCH)
				break;
		}
		if(is_match != ITS_MATCH)
			fprintf(fp,"Picture %d No Objects were found\n",picture);
	}

	free_search_data(&search_data);
	fclose(fp);

	end = clock();
	printf("\n\n\n===========\t SERIAL -FINISHED PROGRAM-: execute time took (not include create %s and reading %s)  -   %1.3f sec    \n\n",INPUT_FILE_NAME,INPUT_FILE_NAME,((double) (end - start)) / CLOCKS_PER_SEC);
}

