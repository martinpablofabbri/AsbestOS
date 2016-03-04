#include "frame.h"

#include "threads/palloc.h"

/*! Attempts to acquire an unused frame. On success, returns
  the kernel virtual address of the acquired page. On failure, returns NULL.
 */
void *frame_acquire (void) {
    return palloc_get_page(PAL_USER);
}
