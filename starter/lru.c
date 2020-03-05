#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

double counter;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int lru_evict() {
	// Number of page frame to be evicted.
	int victim;
	// Timestamp of page frame to be evicted. Initially, set to timestamp of MOST recently used page frame.
	double victim_timestamp = counter;
	// Find LEAST recently used page frame, by finding the page frame with the smallest timestamp.
	for (int i = 0; i < memsize; i++) {
		struct frame curr_frame = coremap[i];
		if (curr_frame.timestamp <= victim_timestamp) {
			victim_timestamp = curr_frame.timestamp;
			victim = i;
		}
	}
	// Return the number of the least recently used page frame.
	return victim;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	// Set the "timestamp" field of the frame corresponding to the given PTE to the current "counter" value.
	coremap[p->frame >> PAGE_SHIFT].timestamp = counter;
	// Increment the counter
	counter++;
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	counter = 0;
}
