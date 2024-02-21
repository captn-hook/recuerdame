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

void collect() {
    struct block *current = head;
    struct block *prev = NULL;

    while (current != NULL) {
        if (prev != NULL && prev->in_use == 0 && current->in_use == 0) {
            prev->size += BLOCK_SIZE + current->size;
            prev->next = current->next;
        } else {
            prev = current;
        }
        current = current->next;
    }
}

void myfree(void *ptr) {
    struct block *current = head;
    struct block *to_free = (struct block *)ptr - 1;

    while (current != NULL) {
        if (current == to_free) {
            current->in_use = 0;
            collect();
            return;
        }
        current = current->next;
    }
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

    //printf("pre alloc(64)");
    print_data();
    p = myalloc(64);
    //printf("post alloc(64)");
    print_data();
    
    // [empty]
    // [1008,used]

    // [empty]
    // [64,used] -> [928,free] // 64 allocated to user, 16 * 2 blocks, 928 free = 1024 heap

    //reset memory
    //printf("pre free(p)\n");
    myfree(p);
    print_data();

    //printf("pre alloc(16)\n");
    print_data();
    p = myalloc(16);
    //printf("post alloc(16)\n");
    print_data();
    void *p2 = myalloc(16);
    p2 = myalloc(16);
    //printf("post alloc another(16)\n");
    //printf("%p\n", p);
    print_data();

    //printf("pre free(p)\n");
    myfree(p);
    myfree(p2);
    print_data();

    // [empty]
    // [1008,used]
    // 0x0

    // [empty]
    // [16,used] -> [976,free] // 16 allocated to user, 16 * 2 blocks, 976 free = 1024 heap
    // 0x7f37452a4030 // and p2 is no longer null when we try to allocate 2 times 

    return 0;
}
