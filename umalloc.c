#include "umalloc.h"
#include "csbrk.h"
#include "ansicolors.h"
#include <stdio.h>
#include <assert.h>

const char author[] = ANSI_BOLD ANSI_COLOR_RED "Nicholas Hoang nhh355" ANSI_RESET;

/*
 * The following helpers can be used to interact with the memory_block_t
 * struct, they can be adjusted as necessary.
 */

// A sample pointer to the start of the free list.
memory_block_t *free_head;

/*
 * is_allocated - returns true if a block is marked as allocated.
 */
bool is_allocated(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & 0x1;
}

/*
 * allocate - marks a block as allocated.
 */
void allocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc |= 0x1;
}


/*
 * deallocate - marks a block as unallocated.
 */
void deallocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc &= ~0x1;
}

/*
 * get_size - gets the size of the block.
 */
size_t get_size(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & ~(ALIGNMENT-1);
}

/*
 * get_next - gets the next block.
 */
memory_block_t *get_next(memory_block_t *block) {
    assert(block != NULL);
    return block->next;
}

/*
 * put_block - puts a block struct into memory at the specified address.
 * Initializes the size and allocated fields, along with NUlling out the next 
 * field.
 */
void put_block(memory_block_t *block, size_t size, bool alloc) {
    assert(block != NULL);
    assert(size % ALIGNMENT == 0);
    assert(alloc >> 1 == 0);
    block->block_size_alloc = size | alloc;
    block->next = NULL;
}

/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    return (void*)(block + 1);
}

/*
 * get_block - given a payload, returns the block.
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);
    return ((memory_block_t *)payload) - 1;
}

/*
 *  STUDENT TODO:
 *      Describe how you select which free block to allocate. What placement strategy are you using?
 * 
 *      I will be implementing a next fit method to finding a free block. I will have to loop 
 *      through the free list, finding the second block big enough to fit the size and return that
 *      size.
 */

/*
 * find - finds a free block that can satisfy the umalloc request.
 */
memory_block_t *find(size_t size) {
    memory_block_t *cur = free_head;
    size_t diff = UINT16_MAX;
    memory_block_t *best_block = NULL;

    while (cur) {
        size_t temp = get_size(cur) - size;
        if (temp >= 0 && temp < diff) {
            diff = temp;
            best_block = cur;
        }
        cur = cur->next;
    }

    return best_block;
}

/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend(size_t size) {
    // get memory from heap
    memory_block_t *mem = csbrk(size + sizeof(memory_block_t));
    if (!mem) {
        return NULL;
    }

    // initialize starting values
    mem->block_size_alloc = size;
    mem->next = NULL;

    // put into free list
    if (!free_head) {
        free_head = mem;
    } else {
        memory_block_t *cur = free_head;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = mem;
    }

    return mem;
}

/*
 *  STUDENT TODO:
 *      Describe how you chose to split allocated blocks. Always? Sometimes? Never? Which end?
 * 
 *      blocks will always be split. After getting the 16 byte aligned size, split will get the
 *      size and split away a block after the given block to be the new free block.
*/

/*
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    // resize current block to new size and get free block size
    size_t free_size = get_size(block) - size - (size_t)sizeof(memory_block_t);
    block->block_size_alloc = size;

    // create new free block and insert into free list
    memory_block_t *free_block = (memory_block_t *)(get_payload(block) + size);
    if (!free_block) {
        return NULL;
    }
    free_block->block_size_alloc = free_size;
    free_block->next = NULL;

    return free_block;
}

/*
 * coalesce - coalesces a free memory block with neighbors.
 */
memory_block_t *coalesce(memory_block_t *block) {
    assert(!is_allocated(block));

    memory_block_t *coa_block = block;
    // check if neighbor after needs to be coalesced
    if (block->next && get_payload(block) + get_size(block) == block->next) {
        memory_block_t *next = block->next;
        coa_block->block_size_alloc += get_size(next) + (size_t)sizeof(memory_block_t);
        coa_block->next = next->next;
    }

    // check if block is not the head
    if (block != free_head) {
        memory_block_t *cur = free_head;
        memory_block_t *last = free_head;
        // search for last free node before the block
        while (cur != block) {
            memory_block_t *temp = cur;
            cur = cur->next;
            last = temp;
        }
        // check if neightbor before can be coalesced
        if (cur && get_payload(last) + get_size(last) == cur) {
            last->block_size_alloc += get_size(cur) + (size_t)sizeof(memory_block_t);
            last->next = cur->next;
            coa_block = last;
        }
    }
    return coa_block;
}



/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    // get block of heap
    size_t init_size = PAGESIZE / 2;
    memory_block_t *head = csbrk(init_size);
    if (!head) {
        return -1;
    }

    // set initial values for head of free list
    head->block_size_alloc = init_size - sizeof(memory_block_t);
    head->next = NULL;

    free_head = head;

    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {
    // get size after padding
    size_t padded_size = ALIGN(size);
    // find block to allocate
    memory_block_t *block = find(padded_size);
    if (!block) {
        // extend the heap if no block big enough is found
        block = extend(padded_size);
        if (!block) {
            return NULL;
        }
    }

    // split block if needed
    if (block->block_size_alloc > padded_size) {
        memory_block_t *free_block = split(block, padded_size);
        if (!free_block) {
            return NULL;
        }
        // insert split free block into free list
        free_block->next = block->next;
        block->next = free_block;
    }
    
    
    // fix free list
    // check if best is the head;
    if (block == free_head) {
        free_head = free_head->next;
    } else {
        // loop through list until new allocated block is found
        memory_block_t *cur = free_head;
        memory_block_t *last = free_head;
        while (cur != block) {
            memory_block_t *temp = cur;
            cur = cur->next;
            last = temp;
        }
        last->next = cur->next;
    }

    // check block as an allocated block
    block->next = NULL;
    allocate(block);
    
    return block + 1;
}

/*
 *  STUDENT TODO:
 *      Describe your free block insertion policy.
 *      
 *      The method will insert the newly freed block back into the free list in order. Once the
 *      position is found in the free list, the memory will be dellocated and will coalesce with
 *      any neighrbors it has.
*/

/*
 * ufree -  frees the memory space pointed to by ptr, which must have been called
 * by a previous call to malloc.
 */
void ufree(void *ptr) {
    //* STUDENT TODO
    memory_block_t *block = get_block(ptr);

    // find where header should go in free list
    // check if free list is NULL
    if (free_head == NULL) {
        free_head = block;
    } else {
        // loop through linked list until address is in right position
        if ((uint64_t)free_head > (uint64_t)block) {
            block->next = free_head;
            free_head = block;
        } else {
            memory_block_t *cur = free_head;
            memory_block_t *last = free_head;
            while (cur && (uint64_t)cur < (uint64_t)block) {
                memory_block_t *temp = cur;
                cur = cur->next;
                last = temp;
            }
            // insert block
            last->next = block;
            block->next = cur;
        }
        
    }
    // change to free
    deallocate(block);
    // coalesce with any neighbors
    coalesce(block);
}