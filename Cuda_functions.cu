#include <cuda_runtime.h>
#include <helper_cuda.h>
#include "Cuda_functions.h"
#include "Input_Output_Manager.h"
#include "Data_Manager.h"
#include "Tools.h"

/*Calculate one object match in picture on specific index (x,y)*/
__device__ int is_match(BYTE* picture_elements, int picture_dim, int x, int y,BYTE* object_elements ,int object_dim, float matching){
	float delta = 0;
	float tmp;
	for (int row = 0; row < object_dim; ++row) {
		for (int col = 0; col < object_dim; ++col) {
									//[x]                      //[y]
			tmp = (picture_elements[ ((x+row)*picture_dim)  +  y+col] - object_elements[row*object_dim+col]) /(float)
																(picture_elements[ ((x+row)*picture_dim)  +  y+col]);
			delta+= tmp < 0 ? tmp * -1 : tmp; //(add ABS)
			if(delta > matching)
				return NOT_MATCH;
		}
	}
	
	return ITS_MATCH;
}
/*Calclate all the pictures match until 1024*1024 indexes
blockIdx.x = picture index
blockIdx.y = object index
threadIdx.x = picture row 
gridDim.y = num if objects
*/
__global__  void find_objects_in_pictures(Search_Data search_data,BYTE* picturesElements,int picturesElementsJumps, BYTE* objectsElements,int objectsElementsJumps,int* results) {

	int pic_dim = search_data.pictures[blockIdx.x].dimention;
	int obj_dim = search_data.objects[blockIdx.y].dimention;
	
	int num_of_searching_in_one_axis = pic_dim - obj_dim + 1;
	
	/*Calculate result index*/
	int result_picture_size = (gridDim.y)*blockDim.x;
	int result_object_size = blockDim.x;
	long result_index = (blockIdx.x*result_picture_size)   +   (blockIdx.y*result_object_size)   +threadIdx.x;
	results[result_index] = 0;
	
	if(obj_dim  > pic_dim)
		return;
	if(threadIdx.x >=  num_of_searching_in_one_axis)
		return;

	int res = 0;
	for (int i = 0; i < num_of_searching_in_one_axis; ++i) { 
		res = is_match(
				picturesElements + (blockIdx.x * picturesElementsJumps),
				pic_dim, 
				threadIdx.x ,
				i,
				objectsElements + (blockIdx.y * objectsElementsJumps),
				obj_dim,
				search_data.matching);

			if(res == ITS_MATCH ){ //every match is x y and object id -> populate into one integer
				results[result_index] = blockIdx.y << ID_SHIFT;
				results[result_index] |= threadIdx.x << X_SHIFT;
				results[result_index] |= i << Y_SHIFT;
				results[result_index] |=  ITS_MATCH;
				return;
			}
	}

}

cudaError_t err = cudaSuccess;

/*Calclate all the pictures match until 1024*1024 indexes*/
int* find_matches_by_cuda(Search_Data* h_search_data,int picture_start ,int picture_end){

    
	Search_Data to_cuda_search_data;
	to_cuda_search_data.matching = h_search_data->matching;
	BYTE* host_pictures_elements; /*all the pictures elements*/
	BYTE* host_objects_elements; /*all the objects elements*/
	
	int picture_max_size = (get_picture_max_dim() * get_picture_max_dim());
	int object_max_size = (get_object_max_dim() * get_object_max_dim());
	long pictures_elements_size = ((picture_end - picture_start) * picture_max_size);
	long objects_elements_size = (h_search_data->num_of_objects * object_max_size);
	long result_size = (picture_end - picture_start) * h_search_data->num_of_objects * get_picture_max_dim();
	
	host_pictures_elements = (BYTE*) malloc(sizeof(BYTE)*pictures_elements_size);
	if(verify_allocate_memory_succeded(host_pictures_elements) == ALLOCATE_MEMORY_FAILED) exit(0);


	/*fill all the pictures elements*/
	for (int picture = picture_start; picture < picture_end; picture++) {
		int pic_dim = h_search_data->pictures[picture].dimention;
		memcpy(host_pictures_elements + ((picture-picture_start) * picture_max_size), h_search_data->pictures[picture].elements, sizeof(BYTE)*pic_dim*pic_dim);
	} 

	host_objects_elements = (BYTE*) malloc(sizeof(BYTE)*objects_elements_size);
	if(verify_allocate_memory_succeded(host_objects_elements) == ALLOCATE_MEMORY_FAILED) exit(0);


	/*fill all the objects elements*/
	for (int object = 0; object < h_search_data->num_of_objects; object++) {
		int obj_dim = h_search_data->objects[object].dimention;
		memcpy(host_objects_elements + (object * object_max_size), h_search_data->objects[object].elements, sizeof(BYTE)*obj_dim*obj_dim);
	}


	BYTE* to_cuda_pictures_elements;
	BYTE* to_cuda_objects_elements;
	
	err = cudaMalloc((void **)&(to_cuda_pictures_elements), sizeof(BYTE)*pictures_elements_size);
	verify_cuda_succedded(__LINE__);

	// Copy data from host to the GPU memory
	err = cudaMemcpy(to_cuda_pictures_elements, host_pictures_elements, sizeof(BYTE)*pictures_elements_size, cudaMemcpyHostToDevice);
	verify_cuda_succedded(__LINE__);


	err = cudaMalloc((void **)&(to_cuda_objects_elements), sizeof(BYTE)*objects_elements_size);
	verify_cuda_succedded(__LINE__);

	// Copy data from host to the GPU memory
	err = cudaMemcpy(to_cuda_objects_elements, host_objects_elements, sizeof(BYTE)*objects_elements_size, cudaMemcpyHostToDevice);
	verify_cuda_succedded(__LINE__);


	// Will contain all the results
	int* to_cuda_results;
	err = cudaMalloc((void **)&(to_cuda_results), sizeof(int)*(result_size));
	verify_cuda_succedded(__LINE__);


	err = cudaMalloc((void **)&(to_cuda_search_data.pictures), sizeof(Picture) * (picture_end - picture_start));
	verify_cuda_succedded(__LINE__);

	// Copy data from host to the GPU memory
	err = cudaMemcpy(to_cuda_search_data.pictures, h_search_data->pictures + picture_start, sizeof(Picture) * (picture_end - picture_start), cudaMemcpyHostToDevice);
	verify_cuda_succedded(__LINE__);


	err = cudaMalloc((void **)&(to_cuda_search_data.objects), sizeof(Picture) * (h_search_data->num_of_objects));
	verify_cuda_succedded(__LINE__);

	// Copy data from host to the GPU memory
	err = cudaMemcpy(to_cuda_search_data.objects, h_search_data->objects, sizeof(Picture) * (h_search_data->num_of_objects), cudaMemcpyHostToDevice);
	verify_cuda_succedded(__LINE__);
	
	
	dim3 DimGrid((picture_end - picture_start),h_search_data->num_of_objects);
	if(get_picture_max_dim() > CUDA_MAX_THREAD){
		printf("\n----------- PICTURE MAX SIZE is the cuda dim blocks and it's > %d, cut the the picture to avoid Cuda error\n",CUDA_MAX_THREAD);
		dim3 DimBlock(CUDA_MAX_THREAD);
		find_objects_in_pictures<<<DimGrid, DimBlock>>>(to_cuda_search_data,to_cuda_pictures_elements,picture_max_size,to_cuda_objects_elements,object_max_size,to_cuda_results);
	}
	else{
		dim3 DimBlock(get_picture_max_dim());
		find_objects_in_pictures<<<DimGrid, DimBlock>>>(to_cuda_search_data,to_cuda_pictures_elements,picture_max_size,to_cuda_objects_elements,object_max_size,to_cuda_results);
	}
	err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf( "Failed in cuda action -  %s\n", cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}


	
	int* results = (int*) malloc(sizeof(int)*(result_size));
	if(verify_allocate_memory_succeded(results) == ALLOCATE_MEMORY_FAILED) exit(0);
	
	// Copy data from GPU memory to host
	err = cudaMemcpy(results, to_cuda_results, sizeof(int)*(result_size), cudaMemcpyDeviceToHost);
	verify_cuda_succedded(__LINE__);

	//Free all cuda allocate
	cudaDeviceReset();


	free(host_pictures_elements);
	free(host_objects_elements);

	return results;

}
/*verify Cuda err*/
void verify_cuda_succedded(int line){
	if (err != cudaSuccess) {
		printf("Failed in cuda action - %s in Line %d\n", cudaGetErrorString(err),line);
		exit(0);
	}
}
