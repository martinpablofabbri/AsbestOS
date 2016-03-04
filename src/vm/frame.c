#include "frame.h"

#include <list.h>

#include "threads/malloc.h"
#include "threads/palloc.h"

/*! Represent a page of physical memory from the user pool.
 */
struct frame_entry {
    struct list_elem elem;           /*!< List element. */
    void *kaddr;                     /*!< Kernel virtual address of the page. */
};
static struct list all_frames;


void init_frame_entry (struct frame_entry* entry);


/*! Initialize the frame subsystem. */
void frame_init (void) {
    list_init(&all_frames);
}

/*! Attempts to acquire an unused frame. On success, returns
  the kernel virtual address of the acquired page. On failure, returns NULL.
 */
void *frame_acquire (void) {
    void* kpage = palloc_get_page(PAL_USER | PAL_ZERO);
    if (!kpage)
	return NULL;

    struct frame_entry* entry;
    entry = (struct frame_entry*)malloc(sizeof(struct frame_entry));
    if (entry) {
	list_push_back(&all_frames, &entry->elem);
	entry->kaddr = kpage;
    } else {
	palloc_free_page(kpage);
	kpage = NULL;
    }
    return kpage;
}

/*! Initialize the variables in a frame_entry struct. */
void init_frame_entry (struct frame_entry* entry) {
    ASSERT(entry != NULL);
}
