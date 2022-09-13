build:
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -c Data_Manager.c -o Data_Manager.o
	mpicxx -fopenmp -c Input_Output_Manager.c -o Input_Output_Manager.o
	mpicxx -fopenmp -c Omp_functions.c -o Omp_functions.o
	mpicxx -fopenmp -c Mpi_functions.c -o Mpi_functions.o
	mpicxx -fopenmp -c Run_Option.c -o Run_Option.o
	mpicxx -fopenmp -c Tools.c -o Tools.o
	nvcc -I./inc -c Cuda_functions.cu -o Cuda_functions.o
	mpicxx -fopenmp -o mpiCudaOpemMP main.o Data_Manager.o Input_Output_Manager.o Omp_functions.o Mpi_functions.o Run_Option.o Tools.o Cuda_functions.o /usr/local/cuda/lib64/libcudart_static.a -ldl -lrt

clean:
	rm -f *.o ./mpiCudaOpemMP
	
runSerial:
	./mpiCudaOpemMP "SERIAL"
run:
	mpiexec -np 2 ./mpiCudaOpemMP "ONE_PC"

runOn2:
	mpiexec -np 2 -machinefile  mf  -map-by  node  ./mpiCudaOpemMP "TWO_PC"
