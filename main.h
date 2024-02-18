
#ifndef MAIN_H
#define MAIN_H

#define HEAP_SIZE 1024

#define ALIGNMENT 16   // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))

#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))


struct block {
    int size;
    int in_use;
    struct block *next;
};

#define BLOCK_SIZE PADDED_SIZE(sizeof(struct block))

void *myalloc(int size);

void myfree(void *p);

void init_heap();

void split(struct block *b, int size);

void print_data();

#endif