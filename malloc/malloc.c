//
// >>>> malloc challenge! <<<<
//
// Your task is to improve utilization and speed of the following malloc
// implementation.
// Initial implementation is the same as the one implemented in simple_malloc.c.
// For the detailed explanation, please refer to simple_malloc.c.

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Interfaces to get memory pages from OS
//

void *mmap_from_system(size_t size);
void munmap_to_system(void *ptr, size_t size);

//
// Struct definitions
//

typedef struct my_metadata_t {
  size_t size;
  struct my_metadata_t *next;
} my_metadata_t;

typedef struct my_heap_t {
  my_metadata_t *free_head; 
  my_metadata_t dummy;
} my_heap_t;

//
// Static variables (DO NOT ADD ANOTHER STATIC VARIABLES!)
//
my_heap_t bins[4]; //my_heap;

//
// Helper functions (feel free to add/remove/edit!)
//
int bin_check(size_t size) {
  if(size < 16){return 0;}
  else if((16 <= size) && (size < 64)) return 1;
  else if((64 <= size) && (size < 128)) return 2;
  else if((128 <= size) && (size < 256)) return 3;
  else if((256 <= size) && (size < 512)) return 4;
  else if((512 <= size) && (size < 1024)) return 5;
  else if((1024 <= size) && (size < 2048)) return 6;
  else return 7;
}

void my_add_to_free_list(my_metadata_t *metadata, int x) {
  assert(!metadata->next);
  metadata->next = bins[x].free_head;
  bins[x].free_head = metadata;
}

void my_remove_from_free_list(my_metadata_t *metadata, my_metadata_t *prev, int x) {
  if (prev) {
    prev->next = metadata->next;
  } else {
    bins[x].free_head = metadata->next;
  }
  metadata->next = NULL;
}

//
// Interfaces of malloc (DO NOT RENAME FOLLOWING FUNCTIONS!)
//

// This is called at the beginning of each challenge.
void my_initialize() {
  for (int i = 0; i < 8; i++){
    bins[i].free_head = &bins[i].dummy;
    bins[i].dummy.size = 0;
    bins[i].dummy.next = NULL;
  }
}

// my_malloc() is called every time an object is allocated.
// |size| is guaranteed to be a multiple of 8 bytes and meets 8 <= |size| <=
// 4000. You are not allowed to use any library functions other than
// mmap_from_system() / munmap_to_system().
void *my_malloc(size_t size) {
  //printf("size; %d\n",size);
  my_metadata_t *best = NULL;
  my_metadata_t *best_prev = NULL;
  int x = bin_check(size);

  while(x < 8){
    my_metadata_t *metadata = bins[x].free_head;
    my_metadata_t *prev = NULL;
    // TODO: Update this logic to Best-fit!
    while (metadata) {
      if (metadata->size >= size) {
          if (best == NULL || metadata->size < best->size) {
              best = metadata;
              best_prev = prev;
          }
      }
      prev = metadata;
      metadata = metadata->next;
    }
    if(best) break;
    x++;
  }

  if (!best) {
    // There was no free slot available. We need to request a new memory region
    // from the system by calling mmap_from_system().
    //
    //     | metadata | free slot |
    //     ^
    //     metadata
    //     <---------------------->
    //            buffer_size
    size_t buffer_size = 4096;
    my_metadata_t *metadata = (my_metadata_t *)mmap_from_system(buffer_size);
    metadata->size = buffer_size - sizeof(my_metadata_t);
    metadata->next = NULL;
    // Add the memory region to the free list.
    int y = bin_check(metadata->size);
    my_add_to_free_list(metadata,y);
    // Now, try my_malloc() again. This should succeed.
    return my_malloc(size); 
    
  }

  // |ptr| is the beginning of the allocated object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  void *ptr = best + 1;
  size_t remaining_size = best->size - size;
  // Remove the free slot from the free list.
  my_remove_from_free_list(best, best_prev, x);

  if (remaining_size > sizeof(my_metadata_t)) {
    // Shrink the metadata for the allocated object
    // to separate the rest of the region corresponding to remaining_size.
    // If the remaining_size is not large enough to make a new metadata,
    // this code path will not be taken and the region will be managed
    // as a part of the allocated object.
    best->size = size;
    // Create a new metadata for the remaining free slot.
    //
    // ... | metadata | object | metadata | free slot | ...
    //     ^          ^        ^
    //     metadata   ptr      new_metadata
    //                 <------><---------------------->
    //                   size       remaining size
    my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
    new_metadata->size = remaining_size - sizeof(my_metadata_t);
    new_metadata->next = NULL;
    // Add the remaining free slot to the free list.
    int z = bin_check(new_metadata->size);
    my_add_to_free_list(new_metadata,z);
  }           
  return ptr;
}

// This is called every time an object is freed.  You are not allowed to
// use any library functions other than mmap_from_system / munmap_to_system.
void my_free(void *ptr) {
  // Look up the metadata. The metadata is placed just prior to the object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  my_metadata_t *metadata = (my_metadata_t *)ptr - 1;
  int x = bin_check(metadata->size);
  // Add the free slot to the free list.
  my_add_to_free_list(metadata,x);
}

// This is called at the end of each challenge.
void my_finalize() {
  // Nothing is here for now.
  // feel free to add something if you want!
}

void test() {
  // Implement here!
  assert(1 == 1); /* 1 is 1. That's always true! (You can remove this.) */
}
