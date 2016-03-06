#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "devices/block.h"

// TODO
void swap_init(); // Initalize new block device for a swap partition

void swap_read(void *kaddr, void *frame);  // read from kaddr into frame
void * swap_write(void *data);  // write data to swap; return place in swap

#endif /* vm/swap.h */

