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
#define USAGE "<inputFilePath> ... <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

int NUM__READER_THREADS = 0;

typedef struct {
	pthread_mutex_t* Qlock;
	pthread_mutex_t* Globallock;
	char* infile;
	queue* q;
	int NUM_READER_THREADS;
} read_arguments;

typedef struct {
	pthread_mutex_t* Qlock;
	pthread_mutex_t* Wlock;
	pthread_mutex_t* Globallock;
	FILE* outfile;
	queue* q;
	int NUM_READER_THREADS;
} write_arguments;

/* Write Function */
void* write_thread_function(void* arguments);

/* Read Function */
void* read_thread_function(void* arguments);
