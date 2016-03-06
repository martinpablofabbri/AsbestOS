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

// TODO
//
//       get_frame() //get empty frame fro either evicted page or new page
//           tries palloc_get_page()
//           check list for unused page
//           choose an in-use page
//               evict that page
//           eventually return like palloc_get_page()
//
//       select_frame()
//       evict_frame()
//       destroy_frame()

#endif /* vm/frame.h */

