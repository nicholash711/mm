
#include "umalloc.h"
#include "csbrk.h"

//Place any variables needed here from umalloc.c or csbrk.c as an extern.
extern memory_block_t *free_head;
extern sbrk_block *sbrk_blocks;

/*
 * check_heap -  used to check that the heap is still in a consistent state.
 
 * STUDENT TODO:
 * Required to be completed for checkpoint 1:
 *      - Check that pointers in the free list point to valid free blocks. Blocks should be within the valid heap addresses: look at csbrk.h for some clues.
 *        They should also be allocated as free.
 *      - Check if any memory_blocks (free and allocated) overlap with each other. Hint: Run through the heap sequentially and check that
 *        for some memory_block n, memory_block n+1 has a sensible block_size and is within the valid heap addresses.
 *      - Ensure that each memory_block is aligned. 
 * 
 * Should return 0 if the heap is still consistent, otherwise return a non-zero
 * return code. Asserts are also a useful tool here.
 */
int check_heap() {
    // Example heap check:
    // Check that all blocks in the free list are marked free.
    // If a block is marked allocated, return -1.
    memory_block_t *cur = free_head;
    while (cur) {
        if (is_allocated(cur)) {
            return -1;
        }
        cur = cur->next;
    }

    sbrk_block *blocks = sbrk_blocks;
    memory_block_t *ptr = (memory_block_t *)(blocks->sbrk_start);
    while (blocks) {
        // check if ptr is at the end of block
        if ((uint64_t)ptr == blocks->sbrk_end) {
            blocks = blocks->next;
            if (blocks) {
                ptr = (memory_block_t *)(blocks->sbrk_start);
            }
        } else {
            // check if ptr is outside block
            if ((uint64_t)ptr > blocks->sbrk_end) {
                return -1;
            }
            // check if block is 16 byte aligned
            if (get_size(ptr) % 16 != 0) {
                return -1;
            }
            ptr = (memory_block_t *)(get_payload(ptr) + get_size(ptr));
        }
    }

    return 0;
}