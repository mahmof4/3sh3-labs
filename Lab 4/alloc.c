#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <unistd.h>

#define KEY (3141)

// This function allocate resources
bool allocateResource(char **res, char type, char units) {
	for (int i = 0; (*res)[i] != 0; i++) {
		if ((*res)[i] == type && (*res)[i + 1] == ' ') {
			(*res)[i + 2] = ((*res)[i + 2] - units + '0' >= '0' ? (*res)[i + 2] - units + '0' : '0');
			printf("Resource %c is now at %c\n", (*res)[i], (*res)[i + 2]);
			return true;
		}
	}
	return false;
}

// This function gets single character from the input
int singleCharacter() {
	int character;
	character = getchar();
	while ('\n' != getchar())
		;
	return character;
}

int main(void) {
	struct sembuf lock_sops[2], release_sops[1];
	int resourseFile, fileSize, semId;
	char * res;
	char resNum, numRes;

	if ((semId = semget(KEY, 1, 0666 | IPC_CREAT)) < 0) { // Get process semaphore
		printf("Unable to obtain semaphore #%i\n", KEY);
		exit(1);
	}

	lock_sops[0].sem_num = 0; // First semaphore
	lock_sops[0].sem_op  = 0; // Wait for zero
	lock_sops[0].sem_flg = 0; // Wait
	lock_sops[1].sem_num = 0; // First semaphore
	lock_sops[1].sem_op  = 1; // Increment
	lock_sops[1].sem_flg = 0;

	release_sops[0].sem_num = 0;  // First semaphore
	release_sops[0].sem_op  = -1; // Decrement
	release_sops[0].sem_flg = 0;  // Wait

	resourseFile = open("res.txt", O_RDWR);
	fileSize     = lseek(resourseFile, 0, SEEK_END);

	if (resourseFile == -1) {
		printf("File not found\n");
		return 1;
	}
    
    // Mapping the memory region for the resource file
	res = mmap((uintptr_t)0, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, resourseFile, 0);

	if (res == MAP_FAILED) {
		printf("Memory map failed\n");
		return 1;
	}

	while (1) {
		printf("Allocate more resources (y/n)? ");
		if (singleCharacter() != 'y')
			exit(0);
		printf("Enter the resource number and number of resources needed: ");
		resNum = singleCharacter();
		numRes = singleCharacter();

		if (semop(semId, lock_sops, sizeof(lock_sops) / sizeof(struct sembuf)) != 0) {
			printf("Semaphore wait and increment operation failed\n");
		}

		if (!allocateResource(&res, resNum, numRes))
			printf("Resource allocation failed\n");

		if (msync(res, fileSize, MS_SYNC) != 0) {
			printf("Could not synchronize file data\n");
			exit(1);
		}
        
        // Release the semaphore
		semop(semId, release_sops, sizeof(release_sops) / sizeof(struct sembuf));
	}

	return 0;
}
