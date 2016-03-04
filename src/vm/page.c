#include "page.h"

#include <list.h>

#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "vm/frame.h"

/*! Represent a page of memory owned by the current user process.
 */
struct spt_entry {
    void* upage;                       /*!< Address of the user
					 virtual page represented by
					 this entry. */
    struct list_elem elem;             /*!< List element. */
    bool created;                      /*!< Has the page been created
					 yet? */
};

void init_spt_entry (struct spt_entry* entry);

/*! Adds a new user page at the given user virtual address.
    Returns true if the operation was successful.
*/
bool page_add_user (void* upage) {
    // TODO(keegan): Check to make sure there's not already a page here.
    uint8_t *kpage;
    bool success = false;

    struct spt_entry* entry;
    entry = (struct spt_entry*)malloc(sizeof(struct spt_entry));
    // TODO(keegan): free
    if (!entry)
	return false;
    init_spt_entry(entry);
    entry->upage = upage;

    kpage = frame_acquire();
    if (kpage != NULL) {
	struct thread *t = thread_current();
	/* Verify that there's not already a page at that virtual
	   address, then map our page there. */
	//success = (pagedir_get_page(t->pagedir, upage) == NULL &&
	//pagedir_set_page(t->pagedir, upage, kpage, true));

	// TODO(keegan): frame_destroy
	success = true;
        if (!success)
            palloc_free_page(kpage);
    }
    return success;
}

/*! Attempt to recover from a page fault at the specified address.
  Returns true if recovery was successful. */
bool page_fault_recover (void* uaddr) {
    return false;
}


/*! Initialize the spt_entry variables. */
void init_spt_entry (struct spt_entry* entry) {
    entry->created = false;
    entry->upage = NULL;
}
