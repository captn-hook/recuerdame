#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "main.h"

void *heap = NULL;
struct block *head = NULL;

void init_heap() {
    heap = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);

    if (heap == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    head = (struct block *)heap;

    head->size = HEAP_SIZE - BLOCK_SIZE;
    head->in_use = 0;
    head->next = NULL;
}

void split(struct block *currentBlock, int size) {
    // create a new block     // from memory  // of the next free space
    struct block *new_block = (struct block *)PTR_OFFSET((currentBlock + 1), size);
    // new size is    //old size         // - new size    
    new_block->size = currentBlock->size - size - BLOCK_SIZE;
    new_block->in_use = 0;
    // new block points to the next block
    new_block->next = currentBlock->next;
    // current block points to the new block
    currentBlock->size = size;
    currentBlock->next = new_block;
}

void *myalloc(int size) {
    //are we initialized?
    if (!heap) {
        init_heap();
    }

    struct block *current = head;

    // find a block.size >= size we want
    while (current != NULL) {
        if (current->in_use == 0 && current->size >= size) {
            // use this block 
            current->in_use = 1;
            // if the block is big
            if (current->size > size + BLOCK_SIZE) {
                split(current, size);
            }
            // give it to the user
            return (void *)(current + 1);
        }
        current = current->next;
    }

    // no block found
    return NULL;
}


void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        // Uncomment the following line if you want to see the pointer values
        //printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}

int main() {

    void *p;

    print_data();
    p = myalloc(64);
    print_data();

    //reset the heap
    heap = NULL;
    head = NULL;
    
    void *p2;

    // [empty]
    // [1008,used]

    print_data();
    p2 = myalloc(16);
    print_data();
    p2 = myalloc(16);
    printf("%p\n", p2);

    // [empty]
    // [1008,used]
    // 0x0

    return 0;
}
