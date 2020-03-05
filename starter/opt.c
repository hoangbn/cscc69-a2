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

// node data structure, used to implement a linked list
// which is used to emulate the trace's list of vaddr's
typedef struct listnode {
	addr_t vaddr;
	struct listnode* next;
} node;

// <head> is the head of the linked list of nodes representing the trace's list of instruction addresses
node* head;

// Given a virtual address, measures how long until the address is referenced again
int evict_helper(addr_t curr_addr) {
	node* curr = head->next; // head contains vaddr that's running now
	// <next_use> = number of references before <curr_addr> is referenced again
	int next_use = 0;
	// Loop through the linked list of nodes starting at <head>
	while (curr != NULL) {
		// If the given address is found in the current node's <vaddr> field
		if (curr_addr == curr->vaddr) {
			// We have found the "next use" of the address, so return it
			return next_use;
		}
		// Increment the number of references seen so far
		next_use++;
		// Move onto next node
		curr = curr->next;
	}
	// If we passed through the entire while loop and haven't found the given address, then the address is never referenced again.
	// Therefore, set <next_use> to -1 to represent this.
	return -1;
}

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	// <victim> is the page frame number of the frame that is referenced again the latest
	int victim = 0;
	// <latest> measures the number of references to the latest "next reference" of any frame in coremap
	int latest = 0;
	// For each frame in coremap
	for (int i = 0; i < memsize; i++) {
		// Measure the number of references until the frame is referenced again
		int curr_next_use = evict_helper(coremap[i].vaddr);
		// If the number of references is greater than the latest
		if (curr_next_use > latest) {
			// Update the <latest> value
			latest = curr_next_use;
			// Set <victim> to be the current frame number, since the current frame has the latest "next reference"
			victim = i;
		}
		// Also, check if the frame is never referenced again (<curr_next_use> == -1)
		else if (curr_next_use == -1) {
			// If this is the case, then the current frame is the best possible eviction
			// Therefore, set <victim> to the current frame number <i>, and stop looping
			victim = i;
			break;
		}
	}
	return victim;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	// Move head to the next node in the list, and free the node prior
	// Set the "vaddr" field of the frame corresponding to the given PTE
	coremap[p->frame >> PAGE_SHIFT].vaddr = head->vaddr;
	node* to_free = head;
	head = head->next;
	free(to_free);
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	// Similar to the "replay_trace()" function in "sim.c",
	// we want to replay the trace and initialize data structures based on the data found

	FILE *fp = fopen(tracefile, "r");

	if (fp == NULL) {
		perror("Error opening tracefile for OPT page replacement algorithm.");
		exit(1);
	}

	char buffer[MAXLINE];
	addr_t vaddr;
	char type;

	node* curr = NULL;

	while (fgets(buffer, MAXLINE, fp) != NULL) {
		if (buffer[0] != '=') {
			sscanf(buffer, "%c %lx", &type, &vaddr);

			// Create a node <new_node> to represent the vaddr found on the current line
			node* new_node = malloc(sizeof(node));

			new_node->next = NULL;
			new_node->vaddr = vaddr;

			// If <head> hasn't been set yet...
			if (head == NULL) {
				// ...then set <head> to <new_node>
				head = new_node;
				// the current node we are manipulating is the head, so set <curr> to <head>
				curr = head;
			// Otherwise, if <curr> has been set previously...
			} else {
				// ...set <curr>'s next node to the <new_node> instead
				curr->next = new_node;
				// Move <curr> onto next node for next iteration of while loop
				curr = curr->next;
			}
		} else {
			continue;
		}
	}
	fclose(fp);
}
