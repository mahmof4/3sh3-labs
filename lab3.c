#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#define DATA_SIZE 255

pthread_mutex_t completed;
pthread_cond_t t_sig;

int shearsort = 0;
int phase = 0;
int matrix[DATA_SIZE];
int size;
int ptr = 0;
int complete_phase = 0;

// This function is used for opening, reading and storing information from the file
void readFile(){
	FILE *input = fopen("input.txt", "r");
    int i = 0;
    int check = 0;
    
    if (input == NULL){
        printf("File is empty.");
    }
	
    // Store all the numbers in a buffer before reaching the end of the input file
	while(i < sizeof(matrix) && check != -1){
        // Check for the end of file
		check = fscanf(input, "%d", &matrix[i++]);
	}
    
    // Close the input file
	fclose(input);
    
    // The size of the matrix
	size = sqrt(i);
}

// This function prints the matrix
void printMatrix(){
	int row, col;
	for (row = 0; row < size; row++){
		for (col = 0; col < size; col++){
			printf("%d\t", matrix[row*size + col]);
		}
		printf("\n");
	}
	printf("\n");
}

void swap(int i, int j){
    int temp;
    temp = i;
    i = j;
    j = temp;
}

void *sort(void *thread_id){
    int temp;
    int i, j;
    int pos;
    
    int t_id = (int)thread_id;
	int row = t_id;
	int col = t_id;
	int row_type = t_id % 2;
	
	while (phase < complete_phase){
		
		switch(shearsort){
            // Sorting the rows of the matrix
			case 0:
				pos = size*(row+1);
				while (pos != row*size){
					for (i = row*size; i < pos-1; i++){
                        // If the row is even
                        // Swap from the lowest to the greatest number
						if (row_type == 0 && matrix[i] > matrix[i+1]){
							temp = matrix[i+1];
							matrix[i+1] = matrix[i];
							matrix[i] = temp;
                            /*
                            //swap(matrix[i+1], matrix[i]);
                            printf("matrix[i+1] even = %d.\n", matrix[i+1]);
                            printf("matrix[i] even = %d.\n", matrix[i]);
                            printf("Even Row\n\n");
                            */
						}
                        // If the row is odd
                        // Swap from the greatest to the lowest number
                        else if (row_type == 1 && matrix[i] < matrix[i+1]){
							temp = matrix[i+1];
							matrix[i+1] = matrix[i];
							matrix[i] = temp;
                            /*
                            //swap(matrix[i+1], matrix[i]);
                            printf("matrix[i+1] odd = %d.\n", matrix[i+1]);
                            printf("matrix[i] odd = %d.\n", matrix[i]);
                            printf("Odd Row\n\n");
                            */
						}
					}
					pos--;
				}
				break;
            
            // Sorting the columns of the matrix
			case 1:
				pos = size;
				while (pos != 0){
                    for (i = col; i < (size*size-(size-col)); i+=size){
                        // Swapping from the lowest to the greatest number
						if (matrix[i] > matrix[i+size]){
							temp = matrix[i+size];
							matrix[i+size] = matrix[i];
							matrix[i] = temp;
                            /*
                            //swap(matrix[i+n], matrix[i]);
                            printf("matrix[i+size] = %d.\n", matrix[i+size]);
                            printf("matrix[i] = %d.\n", matrix[i]);
                            printf("Column\n\n");
                            */
						}
					}
					pos--;
				}
				break;
		}
		
        // This acts like a sem wait to only allow one thread for the completed state
        // Locks the mutex before updating value in the shared memory
		pthread_mutex_lock(&completed);
		
        // Increments when the rows or the columns sorting completes
		ptr++;
		
        // When one phase is completed
		if (ptr == size){
			ptr = 0;
			shearsort = !shearsort;
            phase++;
			printf("Phase: %d\n", phase);
			printMatrix();
			pthread_cond_broadcast(&t_sig); // Unblocks all the threads currently blocked 
		}
        else {
			pthread_cond_wait(&t_sig, &completed); //  Blocks the calling thread until the specified condition is signalled
		}
		// Unlock the mutex upon updating the value
		pthread_mutex_unlock(&completed);
	}
	pthread_exit(NULL);
}

int main(){
	readFile();
	printf("Initial (%dx%d):\n", size, size);
	printMatrix();
	complete_phase = ceil(2*(log(size) / log(2))+1);
    
	pthread_t threads[16];
	// Initializing mutex
	pthread_mutex_init(&completed, NULL);
	pthread_cond_init(&t_sig, NULL);
	
    long i = 0;
	for (i = 0; i < size; i++){
		// Creating threads depending upon the size
		if (pthread_create(&threads[i], NULL, sort, (void *)i)){
			printf("ERROR: Thread creation failed.");
			return 1;
		}
	}
    
    for (i = 0; i < size; i++){
    	// Terminates the threads if successful
        pthread_join(threads[i], NULL);
    }
	
	// Destroys the threads
	pthread_mutex_destroy(&completed);
	pthread_cond_destroy(&t_sig);
	return 0;
}
