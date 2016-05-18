/*
 * Kelsey Dowd
 * Programming Assignment 4
 * 4/14/16
 * SID: 102348752
 * 
 */

#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int i;
    int j;
    int k;
    int proctmp;
    int pagetmp;

    /* initialize static vars on first run */
    if(!initialized){
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
		timestamps[proctmp][pagetmp] = 0; 
	    }
	}
	initialized = 1;
    }
    
    /* TODO: Implement LRU Paging */
    
    // for each process, loop through
    for(i=0; i < MAXPROCESSES; i++) {
		// if the page is active, continue
		if(q[i].active) {
			// Set page needed = pc/PAGESIZE
			int pc = q[i].pc;
			int page = pc/PAGESIZE;
			
			// if the page is not already paged in or being paged in, continue
			if(!q[i].pages[page]) {
				// if paging in fails, then continue to kick out a page
				if(!pagein(i,page)) {
				
					// find a page that is paged in already and set that equal to the lrupage
					int lrupage;
					for(k=0; k < q[i].npages; k++) {
						if(q[i].pages[k]) {
							lrupage = k;
							break;
						}
					}	
					
					// loop through the pages and find the minimum time stamp to find the LRU page
					for(j=0; j < q[i].npages; j++) {
						if(q[i].pages[j]) {
							if(timestamps[i][j] < timestamps[i][lrupage]) {
								lrupage = j;
							}
						}
					}
					
					
					// set the time stamp for the lrupage to 0
					timestamps[i][lrupage] = 0;
					
					// remove the page
					if(pageout(i,lrupage)) {
					}
					else {
						printf("pageout failed/n");
					}
				}
			}
			
			//update the time stamp for the page to the tick value
			timestamps[i][page] = tick;
			
		}
	}
    
    //incrememt the clock
    tick++;
 
} 
