#include <stdlib.h>
#include <stdbool.h>

#define ALIGNMENT 16 /* The alignment of all payloads returned by umalloc */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/*
 * memory_block_t - Represents a block of memory managed by the heap. The 
 * struct can be left as is, or modified for your design.
 * In the current design bit0 is the allocated bit
 * bits 1-3 are unused.
 * and the remaining 60 bit represent the size.
 */
typedef struct memory_block_struct {
    size_t block_size_alloc;
    struct memory_block_struct *next;
} memory_block_t;

// Helper Functions, this may be editted if you change the signature in umalloc.c

/*
*  STUDENT TODO:
*      Write 1-2 sentences for each function explaining what it does. Don't just repeat the name of the function back to us.
*/
<<<<<<< HEAD
// Does AND bitwise operator on block_size_alloc and 0x1 to determine if 
// the block is marked as allocated.
bool is_allocated(memory_block_t *block);
// Changes bit 0 to 1 to denote the block as allocated.
void allocate(memory_block_t *block);
// Changes bit 0 to 0 to denote that the block is not allocated anymore.
void deallocate(memory_block_t *block);
// After ignoring the first 4 bits, return size of the block
size_t get_size(memory_block_t *block);
// Based on the block given, it will return the next member of the block.
memory_block_t *get_next(memory_block_t *block);
// Will initialize a given block with given values. block_size_alloc will be occupied
// with the size and whether it is allocated or not. next will be set to NULL.
void put_block(memory_block_t *block, size_t size, bool alloc);
// adds 1 to the pointer of the given block and returns the payload.
void *get_payload(memory_block_t *block);
// subtracts 1 from the given pointer to return the memory block.
memory_block_t *get_block(void *payload);

// using the given size, find a block that is big enough to satisfy the needed size.
memory_block_t *find(size_t size);
// if needed, extend the heap to fit size of new block.
memory_block_t *extend(size_t size);
// split the block of memory into 2 new blocks: an allocated portion, and an unallocated portion.
memory_block_t *split(memory_block_t *block, size_t size);
// combines a free memory block with its neighbor.
=======
bool is_allocated(memory_block_t *block);
void allocate(memory_block_t *block);
void deallocate(memory_block_t *block);
size_t get_size(memory_block_t *block);
memory_block_t *get_next(memory_block_t *block);
void put_block(memory_block_t *block, size_t size, bool alloc);
void *get_payload(memory_block_t *block);
memory_block_t *get_block(void *payload);

memory_block_t *find(size_t size);
memory_block_t *extend(size_t size);
memory_block_t *split(memory_block_t *block, size_t size);
>>>>>>> 399c9fa3fea9b9eaec0903b952bf07d5c44c7de5
memory_block_t *coalesce(memory_block_t *block);


// Portion that may not be edited
int uinit();
void *umalloc(size_t size);
void ufree(void *ptr);