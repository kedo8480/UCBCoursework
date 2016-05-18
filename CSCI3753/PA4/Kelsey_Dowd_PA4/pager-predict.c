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
    
    /* This file contains the stub for a predictive pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int freqArray[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];
    static int pastAccessed[MAXPROCESSES];

    /* Local vars */
    int i;
    int j;
    int p;
    int proctmp;
    int pagetmp;
    int toPage;
    
    /* initialize static vars on first run */
    if(!initialized){
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
		pastAccessed[proctmp] = -1;
	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
			for(toPage=0; toPage < MAXPROCPAGES; toPage++) {
				freqArray[proctmp][pagetmp][toPage] = 0;
			}
	    }
	}
	initialized = 1;
    }
    
    
    /* TODO: Implement Predictive Paging */
    
    // for each process, loop through
    for(i=0; i < MAXPROCESSES; i++) {
		// if the page is active, continue
		if(q[i].active) {
			// Set page needed = pc/PAGESIZE
			int pc = q[i].pc;
			int page = pc/PAGESIZE;
			
			// set the nextPages predicted to the inital values of the frequency array
			int nextPage1 = freqArray[i][page][0];
			int nextPage2 = freqArray[i][page][0];
			
			// If the past page is different from the current page, update the frequency array
			if (pastAccessed[i] != page) {
				freqArray[i][pastAccessed[i]][page] += 1;
			}
			
			// Update the past accessed to the current page
			pastAccessed[i] = page;
			
			// Find the most probable page that will be next using the frequency array
			for(p=0; p < MAXPROCPAGES; p++) {
				if (freqArray[i][page][p] > freqArray[i][page][nextPage1]) {
					nextPage1 = p;
				}
			}
			
			// Find the second most probable page that will be next using the frequency array
			for(p=0; p < MAXPROCPAGES; p++) {
				if (freqArray[i][page][p] > freqArray[i][page][nextPage2]) {
					if (p != nextPage1) {
						nextPage2 = p;
					}
				}
			}
			
			// If the process is not the current page or the next predicted pages and its is paged in, page it out
			for(j=0; j < MAXPROCPAGES; j++) {
				if ((j != page) && (j != nextPage1) && (j != nextPage2) && (q[i].pages[j])) {
					pageout(i, j);
				}
			}
			
			// if current page isnt paged in, page in current page
			if(!q[i].pages[page]) {
				pagein(i, page);
			}
			
			// if nextPage1 is not paged in, page in nextPage1
			if(!q[i].pages[nextPage1]) {
				pagein(i,nextPage1);
			}
			
			// if nextPage2 is not paged in, page in nextPage2
			if(!q[i].pages[nextPage2]) {
				pagein(i,nextPage2);
			}
		}
	}
    
    //incrememt the clock
    tick++;
    
} 
