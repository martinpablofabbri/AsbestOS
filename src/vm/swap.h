#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t swap_info_t;

void swap_init(void);
void swap_read(void* kpage, swap_info_t info);
bool swap_write(void* kpage, swap_info_t* info);

#endif /* vm/swap.h */
