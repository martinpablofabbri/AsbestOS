#include "page.h"

#include "userprog/pagedir.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "vm/frame.h"

/*! Adds a new user page at the given user virtual address.
    Returns true if the operation was successful.
*/
bool add_user_page (void* upage) {
    // TODO(keegan): Check to make sure there's not already a page here.
    uint8_t *kpage;
    bool success = false;

    kpage = frame_acquire();
    if (kpage != NULL) {
	struct thread *t = thread_current();
	/* Verify that there's not already a page at that virtual
	   address, then map our page there. */
	success = (pagedir_get_page(t->pagedir, upage) == NULL &&
		   pagedir_set_page(t->pagedir, upage, kpage, true));

	// TODO(keegan): frame_destroy
        if (!success)
            palloc_free_page(kpage);
    }
    return success;
}
