#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

#define ALARM_TIME (10)
#define KEY (3141)

pid_t childpid;
char *res;

void signalHandler(int signo) {
	if (signo == SIGALRM && childpid == 0) {
		int pageSize, numPages;
		unsigned char *vec;

		pageSize = getpagesize();
		numPages = (pageSize + pageSize - 1) / pageSize;
		vec      = malloc(numPages);

		if (vec == NULL) {
			printf("malloc error\n");
			exit(1);
		}
    
        // Checks whether the page is within the memory
        if (mincore(res, pageSize, vec) == -1) {
			printf("mincore error\n");
			exit(1);
		}
        
        // Loops through the memory and check whether the page is present
		for (int i = 0; i < numPages; i++) {
			if (i % 64 == 0)
				printf("%s%10p: ", i == 0 ? "" : "\n", res + (i * pageSize));

			printf("%s", (vec[i] & 0x01) ? "In memory" : "Not in memory");
		}

		printf("\n");
		free(vec); // Free the vector that was allocated
		printf("Page size: %i\n", pageSize);

		printf("State of resources:\n%s\n", res);

		alarm(ALARM_TIME); // Set up the next alarm
	}
}

// This does same thing as alloc but now we provide resources
bool provideResource(char **res, char type, char units) {
	for (int i = 0; (*res)[i] != 0; i++) {
		if ((*res)[i] == type && (*res)[i + 1] == ' ') {
			(*res)[i + 2] = ((*res)[i + 2] + units - '0' <= '9' ? (*res)[i + 2] + units - '0' : '9');
			printf("Resource %c is now at %c\n", (*res)[i], (*res)[i + 2]);
			return true;
		}
	}
	return false;
}

int singleCharacter() {
	int character;
	character = getchar();
	while ('\n' != getchar())
		;
	return character;
}

void main(void) {
	int  resourseFile, fileSize;
	char resNum, numRes;

	if (signal(SIGALRM, signalHandler) == SIG_ERR) { // Register  the  signal  handler
		printf("Failed to register a signal handler");
		exit(1);
	}

	resourseFile = open("res.txt", O_RDWR); // File and memory mapping setup
	fileSize     = lseek(resourseFile, 0, SEEK_END);

	if (resourseFile == -1) {
		printf("File not found\n");
		exit(1);
	}

	res = mmap((uintptr_t)0, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, resourseFile, 0);

	if (res == MAP_FAILED) {
		printf("Memory map failed\n");
		exit(1);
	}

    // Creates a fork between child and parent
	if ((childpid = fork()) == -1) {
		perror("Fork error");
		exit(1);
	}

	if (childpid == 0) {                 // Child process
		prctl(PR_SET_PDEATHSIG, SIGHUP); // Die when parent exits
		alarm(ALARM_TIME);               // Set the initial alarm

		while (1)
			sleep(10); // Everything is triggered by the alarm signal

	}
    else { // Parent process
		struct sembuf lock_sops[2], release_sops[1];
		int           semId;

		union semun { // Semaphore setup
			int              val;
			struct semid_ds *buf;
			short *         array;
		} argument;

		argument.val = 0; // Set initial semaphore value to zero

		if ((semId = semget(KEY, 1, 0666 | IPC_CREAT)) < 0) {
			printf("Unable to obtain semaphore #%i\n", KEY);
			exit(1);
		}

		if (semctl(semId, 0, SETVAL, argument) < 0) { // Set the initial semaphore value to 1
			printf("Cannot set semaphore value.\n");
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

		while (1) {
			printf("Provide more resources (y/n)? ");
			if (singleCharacter() != 'y')
				exit(0);
			printf("Enter the resource number and number of resources to be provided: ");
			resNum = singleCharacter();
			numRes = singleCharacter();

			if (semop(semId, lock_sops, sizeof(lock_sops) / sizeof(struct sembuf)) != 0) {
				printf("Semaphore wait and increment operation failed\n");
			}

			if (!provideResource(&res, resNum, numRes))
				printf("Resource providing failed\n");

			if (msync(res, fileSize, MS_SYNC) != 0) {
				printf("Could not synchronize file data\n");
				exit(1);
			}

			semop(semId, release_sops, sizeof(release_sops) / sizeof(struct sembuf));
		}
	}

	exit(0); // Exit all processes
}
