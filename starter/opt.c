#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"
#include "sim.h"


extern unsigned memsize;

extern int debug;

extern struct frame *coremap;

struct Node { 
    addr_t vaddr;
    struct Node* next; 
}; 

struct Node* head = NULL;
/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	int evict_frame = 0; // default value to return (if no page will be used again)
	int farthest_num = 0;
	for (int i = 0; i < memsize; i++) {
		printf("YO\n");
		// check to see if vaddr at current frame will ever appear again
		// if yes, compare it to current furthest, update if necessary
		addr_t cur_frame_vaddr = coremap[i].vaddr;
		struct Node *current = head->next; // head contains vaddr that we're trying to use to access physmem
		int cur_farthest_num = 0;
		while (current != NULL) {
			if (current->vaddr != cur_frame_vaddr) {
				cur_farthest_num++;
				current = current->next;
			}
			else break;
		}
		// if we reached the end of list, we found the page that will never get used again
		if (current == NULL) return i;
		// else, check if this page appears further than currently furthest frame and update
		if (cur_farthest_num > farthest_num) {
			farthest_num = cur_farthest_num;
			evict_frame = i;
		}
	}
	return evict_frame;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	// Set the "vaddr" field of the frame corresponding to the given PTE
	struct frame curr_frame = coremap[p->frame >> PAGE_SHIFT];
	curr_frame.vaddr = head->vaddr;
	// move to next node in our linked list because the vaddr at head was just accessed
	head = head->next;
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	FILE *infp = fopen(tracefile, "r");
	char buf[MAXLINE];
	char _;
	struct Node* current = NULL;
	struct Node* next;
	addr_t vaddr = 0;
	// initialize head of linked list containing future vaddrs
	while (fgets(buf, MAXLINE, infp) != NULL) {
		if (buf[0] != '=') {
			sscanf(buf, "%c %lx", &_, &vaddr);
			head = (struct Node*)malloc(sizeof(struct Node));
			head->vaddr = vaddr;
			current = head;
			break;
		} else {
			continue;
		}
	}
	// create linked list of nodes containing future vaddrs
	while(fgets(buf, MAXLINE, infp) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &_, &vaddr);
			next = (struct Node*)malloc(sizeof(struct Node));
			next->vaddr = vaddr;
			printf("%lu\n", vaddr);
			current->next = next;
			current = next;
		} else {
			continue;
		}
	}
}

