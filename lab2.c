#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main(void){
	int fd[2], fd1[2], nbytes;
	pid_t childpid;
	char readbuffer [80];
	pipe(fd); // Pipe 1
	pipe(fd1); // Pipe 2
	int Input = 0;
    int Value = 0;
    int Total_p = 0;
    int Total_c = 0;
    int i = 1;
    
    printf("--------------------------------------------\n");
    printf("                  Lab 2\n");
    printf("--------------------------------------------\n");
	
	// Error Handler
    // Fork function creates child process
	if((childpid = fork()) ==  -1){
		perror("fork");
		exit (0);
	}
	
	// If we are in the child process
	if(childpid  == 0){
	
		close(fd[0]);
		close(fd1[1]);
		
		while(1) {
			// Enter the input that gets added to the ended of the pipe
            printf("Please enter input %d or -1 to terminate: ", i);
            scanf("%d", &Input); // Scan the input and store in Input1
            i++;
            
            if (Input != -1) { // Loop until -1 is inputted
                write(fd[1], &Input, sizeof(Input)); // This value is sent to the parent
            }
           
            else {
                break; // Break the loop is -1 is inputted
            }
        }
        
        write(fd[1], &Input, sizeof(Input));
        
        // When -1 is received by the parent, the value is stored and printed here through pipe
        read(fd1[0], &Total_c, sizeof(Total_c));
        printf("Total= %d\n", Total_c);
        
        printf("--------------------------------------------\n");
        printf("--------------------------------------------\n");
        
        exit (0);
	}
	
	// If we are in the parent process
	else{
		close(fd[1]);
		close(fd1[0]);
		
		while(1) {
           	
           	// The value sent by the child is stored here
            read(fd[0], &Value, sizeof(Value));
            
            if (Value != -1) {
                Total_p = Total_p + Value;
            }
           
            else {
                break;
            }
        }
		
		write(fd1[1], &Total_p, sizeof(Total_p)); // This value is sent to child through pipe
        
        exit(0);
	}
	
	return  0;
}
