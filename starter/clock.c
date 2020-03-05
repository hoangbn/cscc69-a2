#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int clk_hand;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {

	while (1) {
		struct frame curr_frame = coremap[clk_hand];
		pgtbl_entry_t* curr_pte = curr_frame.pte;
		// If the ref bit is on
		if (curr_pte->frame & PG_REF) {
			// Turn ref bit off...
			curr_pte->frame = curr_pte->frame & ~PG_REF;
			// ...and increment clk_hand to go to next page (in while loop)
			clk_hand++;
			if (clk_hand == memsize) clk_hand = 0;
		}
		// Otherwise, if the ref bit is off
		else {
			// Evict the page. In other words, return value is the current PTE's index in the coremap.
			int result = clk_hand;
			// And make sure to increment clk_hand so that we don't get stuck on evicting the same page
			clk_hand++;
			if (clk_hand == memsize) clk_hand = 0;
			return result;
		}
	}
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	// Since we are accessing page table entry <p>, set its frame's reference bit on
	// This will give <p> a "second chance" to not be evicted
	p->frame = p->frame | PG_REF; // is actually done in pagetable too
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	clk_hand = 0;
}
