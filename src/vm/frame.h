#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>

/*! Represent a page of physical memory from the user pool.
 */
struct frame_entry {
    struct list_elem elem;           /*!< List element. */
    void *paddr;                     /*!< Physical address of the page. */
};

void *frame_acquire(void);

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

