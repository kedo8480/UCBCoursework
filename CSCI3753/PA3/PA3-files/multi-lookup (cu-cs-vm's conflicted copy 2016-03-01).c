// Kelsey Dowd
// SID: 102348752

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "util.h"
#include "queue.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

#define NUM_THREADS	5

/* Write Function */
void* write(void* threadid)
{
    long* tid = threadid;

	while(queue_is_empty(q)) {
		usleep(100);
	}
	
	pthread_mutex_init();
	pthread_mutex_lock();
	queue_pop(q);
	pthread_mutex_unlock();
	
	dnslookup(hostname, firstipstr, sizeof(firstipstr));
	
	pthread_mutex_init();
	pthread_mutex_lock();
	
	//write to file

	pthread_mutex_unlock();
	

    return NULL;
}

/* Read Function */
void* read(void* threadid)
{
    long* tid = threadid;

	fopen(argv[1], "r");
	

	while(queue_is_empty(q))    
	    usleep(100);
	}
	
	pthread_mutex_init();
	pthread_mutex_lock();
	queue_push(q, hostname);
	pthread_mutex_unlock();
    

    return NULL;
}


int main(int argc, char* argv[]){
    /* Local Vars */
    FILE* inputfp = NULL;
    FILE* outputfp = NULL;
    char hostname[SBUFSIZE];
    char errorstr[SBUFSIZE];
    char firstipstr[INET6_ADDRSTRLEN];
    int i;
    
    /* Check Arguments */
    if(argc < MINARGS){
	fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
	fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
	return EXIT_FAILURE;
    }

    /* Open Output File */
    outputfp = fopen(argv[(argc-1)], "w");
    if(!outputfp){
	perror("Error Opening Output File");
	return EXIT_FAILURE;
    }

    /* Loop Through Input Files */
    for(i=1; i<(argc-1); i++){
	
	/* Open Input File */
	inputfp = fopen(argv[i], "r");
	if(!inputfp){
	    sprintf(errorstr, "Error Opening Input File: %s", argv[i]);
	    perror(errorstr);
	    break;
	}	

	/* Read File and Process*/
	while(fscanf(inputfp, INPUTFS, hostname) > 0){
	
	    /* Lookup hostname and get IP string */
	    if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
	       == UTIL_FAILURE){
		fprintf(stderr, "dnslookup error: %s\n", hostname);
		strncpy(firstipstr, "", sizeof(firstipstr));
	    }
	
	    /* Write to Output File */
	    fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
	}

	/* Close Input File */
	fclose(inputfp);
    }

    /* Close Output File */
    fclose(outputfp);

    return EXIT_SUCCESS;
}
