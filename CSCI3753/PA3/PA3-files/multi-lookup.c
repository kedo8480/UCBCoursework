// Kelsey Dowd
// SID: 102348752

/* Here are the files we need to include */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "util.h"
#include "queue.h"

/* global variables */
#define MINARGS 3
#define USAGE "<inputFilePath> ... <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

int reader_count = 0;

/* This is the initialization of two structs we need for the arguments of pthread_create */
typedef struct {
	pthread_mutex_t* Qlock;
	pthread_mutex_t* Globallock;
	char* infile;
	queue* q;
} read_arguments;

typedef struct {
	pthread_mutex_t* Qlock;
	pthread_mutex_t* Wlock;
	FILE* outfile;
	queue* q;
} write_arguments;

/* Write Function (for resolver threads)*/
void* write_thread_function(void* arguments)
{
	char firstipstr[INET6_ADDRSTRLEN]; //this needs to be the right size or it will cut off the ip address
	int counter = 0; //this is the counter to keep track of how many hostnames each thread resolves
	
	/* Making the input argument struct into a struct instead of a void* */
    write_arguments *new_variable_for_arguments2 = (write_arguments*)arguments;
	
	/* The thread runs through this loop until there are no reader threads left or the queue isnt empty */
	while(!queue_is_empty(new_variable_for_arguments2->q) | (reader_count > 0)) {
		
		/* If queue is empty, wait */
		while(queue_is_empty(new_variable_for_arguments2->q)) {
			usleep(100);
		}
		
		/* When popping the value from the queue, make sure to lock/unlock the Qlock */
		pthread_mutex_lock(new_variable_for_arguments2->Qlock);
			char* hostname = queue_pop(new_variable_for_arguments2->q);	
		pthread_mutex_unlock(new_variable_for_arguments2->Qlock);
		
		/* Lookup the IP address, if there is an error, print which hostname it was */
		if(dnslookup(hostname, firstipstr, sizeof(firstipstr)) == UTIL_FAILURE){
			fprintf(stderr, "dnslookup error: %s\n", hostname);
		}
		
		
		/* When writing to the output file, lock/unlock the Wlock */
		/* Also increment the counter so that the thread knows how many hostnames it resolved */
		/* Also free the hostname memory */
		pthread_mutex_lock(new_variable_for_arguments2->Wlock);
			fprintf(new_variable_for_arguments2->outfile, "%s,%s\n", hostname, firstipstr);
			free(hostname);
			counter += 1;
		pthread_mutex_unlock(new_variable_for_arguments2->Wlock);
	}
	
	/* Print the number of hostnames processed and exit */
    printf("Resolver thread processed %d hostnames from the queue.\n", counter);
    return NULL;
}

/* Read Function (for requester threads) */
void* read_thread_function(void* arguments)
{
	/* Making the input argument struct into a struct instead of a void* */
    read_arguments *new_variable_for_arguments = (read_arguments*)arguments;
    
    /* Allocate memory for the payload to push to the stack */
    char* hostname = malloc(sizeof(char)*1025);
	
	/* Create a file pointer for the name of the file */
	FILE* readfilepointer = fopen(new_variable_for_arguments->infile, "r");
	
	/* Counter will keep track of how many hostnames pushed to the stack */
	int counter = 0;
	
	/* While the file still has things to read from it, keep looping to read hostnames */
	while(fscanf(readfilepointer, "%1024s", hostname) > 0) {
		
		/* If queue is full, wait */
		while(queue_is_full(new_variable_for_arguments->q)) {  
			usleep(100);
		}
		
		/* When pushing hostnames to the queue, lock/unlock the Qlock */
		/* Also increment the counter to keep track of how many hostnames pushed to the queue */
		pthread_mutex_lock(new_variable_for_arguments->Qlock);
			queue_push(new_variable_for_arguments->q, hostname);
			counter += 1;
		pthread_mutex_unlock(new_variable_for_arguments->Qlock);
		
		/* reallocate the hostname to new memory */
		hostname = malloc(sizeof(char)*1025);
	}
	
	/* Update the global variable to report how many reader threads there are left */
	/* Need to lock the Globallock to ensure it is locked so it can't be changed by another thread at the same time */
	pthread_mutex_lock(new_variable_for_arguments->Globallock);
		reader_count -= 1;
	pthread_mutex_unlock(new_variable_for_arguments->Globallock);
	
	/* Print the number of hostnames pushed to the queue, free memory, and exit */
	printf("Requester thread added %d hostnames to the queue.\n", counter);
    free(hostname);
    fclose(readfilepointer);
	return NULL;
}


/* Main function */
int main(int argc, char* argv[]){
    
    /* Initiallize i and the queue */
    int i;
    queue q;
    queue_init(&q, 50);
    
    /* Define the thread arrays */
    pthread_t read_threads[argc-2];
    pthread_t write_threads[10];
   
    /* Initiallize locks */
    pthread_mutex_t Globallock;
	pthread_mutex_init(&Globallock, NULL);
  
    pthread_mutex_t Wlock;
	pthread_mutex_init(&Wlock, NULL);
	
	pthread_mutex_t Qlock;
	pthread_mutex_init(&Qlock, NULL);
    
    /* Check Arguments, if not enough, throw error */
    if(argc < MINARGS){
		fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
		return EXIT_FAILURE;
    }

	/* Create an array of pointers to memory so that you can pass each element into the structs which are passed into the pthreads */
	read_arguments** memory_array_read = malloc(sizeof(read_arguments*) * (argc-2));
	write_arguments** memory_array_write = malloc(sizeof(write_arguments*) * 10);

	/* Create file pointer to the results file */
	FILE* filepointer = fopen(argv[argc-1], "w");
	
	/* Launch reader threads: first allocate memory and fill struct needed for arguments */
	for(i = 0; i < argc-2; i++) {
		read_arguments* read_args = malloc(sizeof(read_arguments));
		read_args->q = &q;
		read_args->Qlock = &Qlock;
		read_args->infile = argv[i+1];
		read_args->Globallock = &Globallock;
		
		memory_array_read[i] = read_args;
		
		pthread_create(&(read_threads[i]), NULL, read_thread_function, (void*)read_args);
		
		reader_count += 1;
	}
	
	/* Launch writer threads: first allocate memory and fill struct needed for arguments */
	for(i = 0; i < 10; i++) {
		write_arguments* write_args = malloc(sizeof(write_arguments));
		write_args->q = &q;
		write_args->Qlock = &Qlock;
		write_args->Wlock = &Wlock;
		write_args->outfile = filepointer;
		
		memory_array_write[i] = write_args;
		
		pthread_create(&(write_threads[i]), NULL, write_thread_function, (void*)write_args);
	}
	
	/* Wait for writer and reader threads to join before ending */
	for(i=0; i<10; i++) {
		pthread_join(write_threads[i], NULL);
	}
	
	for(i=0; i<argc-2; i++) {
		pthread_join(read_threads[i], NULL);
	}

	fclose(filepointer);

	/* Destroy mutex locks */
	pthread_mutex_destroy(&Qlock);
	pthread_mutex_destroy(&Wlock);
	pthread_mutex_destroy(&Globallock);
	
	queue_cleanup(&q);
	
	/* Free memory used to keep track of the structs passed into the threads */
	for(i = 0; i < argc-2; i++) {
		free(memory_array_read[i]);
	}
	
	free(memory_array_read);
	
	for(i = 0; i < 10; i++) {
		free(memory_array_write[i]);
	}
	
	free(memory_array_write);
	
    return EXIT_SUCCESS;
}
