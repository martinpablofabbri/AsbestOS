#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>

#include "vm/page.h"

/*! Represent a page of physical memory from the user pool.
 */
struct frame_entry {
    struct list_elem elem;           /*!< List element. */
    void *kpage;                     /*!< Kernel virtual address of the frame. */
    struct spt_entry* spt;           /*!< Pointer to the spt_entry of
                                       the page occupying the frame. */
};

void frame_init(void);
struct frame_entry* frame_acquire(void);

#endif /* vm/frame.h */

