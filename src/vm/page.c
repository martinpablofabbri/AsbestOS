#include "page.h"

#include <list.h>

#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
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
struct spt_entry* get_spt_entry (void* uaddr);

/*! Adds a new user page at the given user virtual address.
    Returns true if the operation was successful.
*/
bool page_add_user (void* upage) {
    // TODO(keegan): Check to make sure there's not already a page here.
    struct spt_entry* entry;

    entry = (struct spt_entry*)malloc(sizeof(struct spt_entry));
    // TODO(keegan): free
    if (!entry)
	return false;
    init_spt_entry(entry);
    entry->upage = upage;
    list_push_back(&thread_current()->supl_page_tbl, &entry->elem);

    return true;
}

/*! Attempt to recover from a page fault at the specified address.
  Returns true if recovery was successful. */
bool page_fault_recover (void* uaddr) {
    struct spt_entry* entry = get_spt_entry(uaddr);
    if (entry == NULL) {
	/* The given uaddr is not one of the current process's. */
	return false;
    }

    void* upage = (void*)((uintptr_t)uaddr & ~PGMASK);
    void* kpage = frame_acquire();
    if (kpage == NULL)
	return false;

    struct thread *t = thread_current();
    /* Map our page to the correct location. */
    if (!pagedir_set_page(t->pagedir, upage, kpage, true)) {
	// TODO(keegan): frame_destroy
	palloc_free_page(kpage);
	return false;
    }
    
    return true;
}


/*! Initialize the spt_entry variables. */
void init_spt_entry (struct spt_entry* entry) {
    entry->created = false;
    entry->upage = NULL;
}

/*! Return the spt_entry corresponding to a given userspace virtual
  address UADDR. Returns NULL if the given userspace address does not
  correspond to any entry in the Supplemental Page Table. */
struct spt_entry* get_spt_entry (void* uaddr) {
    struct list_elem *e;
    struct spt_entry* ret = NULL;

    struct list *spt_list = &thread_current()->supl_page_tbl;

    uintptr_t ua = (uintptr_t)uaddr;
    for (e = list_begin(spt_list); e != list_end(spt_list);
         e = list_next(e)) {
        struct spt_entry *s = list_entry(e, struct spt_entry, elem);
	uintptr_t base = (uintptr_t)s->upage;
	if (base <= ua && ua < base + PGSIZE) {
	    ret = s;
	    break;
	}
    }

    return ret;
}
