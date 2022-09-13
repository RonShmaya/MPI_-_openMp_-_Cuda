#include <string.h>
#include "Run_Options.h"
#include "Mpi_functions.h"
#include "Data_Manager.h"

/*All Program Options Expected From Argv - Defined In Data Manager*/

/*Option Serial*/
extern const char OPTION_PROGRAM_RUN_SERIAL[];
/*Option Parallel One PC*/
extern const char OPTION_PROGRAM_RUN_PARALLEL_ONE_PC[];
/*Option Parallel Two PC*/
extern const char OPTION_PROGRAM_RUN_PARALLEL_TWO_PC[];

int main(int argc, char* argv[]){

	/*Verify Argc Size As expected*/
	if(argc != ARGC_EXPECTED_SIZE){
		printf("Expected to have %d parametrs in ARGC but get %d\n",ARGC_EXPECTED_SIZE,argc);
		return 0;
	}

	/*Run Program Serial Option*/
	if(strcmp(argv[1],OPTION_PROGRAM_RUN_SERIAL) == 0){
		set_program_option(OPTION_PROGRAM_RUN_SERIAL); /*Save The Program Option*/
		run_serial(); 								   /*Execute The Serial Option*/
		return 0;
	}/*Run Program Parallel Option - One PC*/
	else if(strcmp(argv[1],OPTION_PROGRAM_RUN_PARALLEL_ONE_PC) == 0)
		set_program_option(OPTION_PROGRAM_RUN_PARALLEL_ONE_PC);
	/*Run Program Parallel Option - Two PC*/
	else if(strcmp(argv[1],OPTION_PROGRAM_RUN_PARALLEL_TWO_PC) == 0)
		set_program_option(OPTION_PROGRAM_RUN_PARALLEL_TWO_PC);
	/*Case of Wrong Argv*/
	else{
		printf("Wrong ARGV parameter , Expected to %s or %s or %s , but get %s\n",OPTION_PROGRAM_RUN_SERIAL,OPTION_PROGRAM_RUN_PARALLEL_ONE_PC,OPTION_PROGRAM_RUN_PARALLEL_TWO_PC,argv[1]);
		return 0;
	}

	init_my_mpi(&argc, &argv);

	run_parallel();

	close_mpi();

	return 0;
}
