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
    int padded_size = PADDED_SIZE(size);

    // create a new block     // from memory  // of the next free space
    struct block *new_block = (struct block *)PTR_OFFSET((currentBlock + 1), padded_size);
    // new size is    //old size         // - new size
    new_block->size = currentBlock->size - padded_size - BLOCK_SIZE;
    new_block->in_use = 0;
    // new block points to the next block
    new_block->next = currentBlock->next;
    // current block points to the new block
    currentBlock->size = padded_size; 
    currentBlock->next = new_block;
}

void *myalloc(int size) {
    //are we initialized?
    if (!heap) {
        init_heap();
    }

    int padded_size = PADDED_SIZE(size);

    struct block *current = head;

    // find a block.size >= size we want
    while (current != NULL) {
        if (current->in_use == 0 && current->size >= padded_size + BLOCK_SIZE) { // and this block size
            // use this block 
            current->in_use = 1;
            // if the block is big
            if (current->size > padded_size + BLOCK_SIZE) { // and this block size
                split(current, padded_size);
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

    p = myalloc(10); print_data();

    myfree(p); print_data();

    // [16,used] -> [976,free]
    // [1008,free]

    void *a, *q;

    a = myalloc(10); print_data();
    q = myalloc(20); print_data();

    myfree(a); print_data();
    myfree(q); print_data();

    // [16,used] -> [976,free]
    // [16,used] -> [32,used] -> [928,free]
    // [16,free] -> [32,used] -> [928,free]
    // [1008,free]

    void *b, *h;

    b = myalloc(10); print_data();
    h = myalloc(20); print_data();

    myfree(b); print_data();
    myfree(h); print_data();

    // [16,used] -> [976,free]
    // [16,used] -> [32,used] -> [928,free]
    // [16,used] -> [976,free]
    // [1008,free]

    void *c, *g, *r, *s;

    c = myalloc(10); print_data();
    g = myalloc(20); print_data();
    r = myalloc(30); print_data();
    s = myalloc(40); print_data();

    myfree(c); print_data();
    myfree(g); print_data();
    myfree(s); print_data();
    myfree(r); print_data();

    // [16,used] -> [976,free]
    // [16,used] -> [32,used] -> [928,free]
    // [16,used] -> [32,used] -> [32,used] -> [880,free]
    // [16,used] -> [32,used] -> [32,used] -> [48,used] -> [816,free]
    // [16,used] -> [32,free] -> [32,used] -> [48,used] -> [816,free]
    // [64,free] -> [32,used] -> [48,used] -> [816,free]
    // [64,free] -> [32,used] -> [880,free]
    // [1008,free]  

    return 0;
}
