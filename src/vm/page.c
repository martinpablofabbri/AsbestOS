#include "page.h"

#include <string.h>

#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "vm/frame.h"
#include "filesys/filesys.h"

/*! Maximum stack size is 8MB. */
#define MAX_STACK_SIZE 0x800000

#pragma GCC push_options
#pragma GCC optimize ("O0")

void init_spt_entry (struct spt_entry* entry);
struct spt_entry* get_spt_entry (void* uaddr);
bool retrieve_page (struct spt_entry* entry, void* kpage);

/*! Adds a new user page at the given user virtual address.
    Returns the spt_entry if the operation was successful.
*/
struct spt_entry* page_add_user (void* upage) {
    // TODO(keegan): Check to make sure there's not already a page here.
    struct spt_entry* entry;

    entry = (struct spt_entry*)malloc(sizeof(struct spt_entry));
    // TODO(keegan): free
    if (!entry)
	return NULL;
    init_spt_entry(entry);
    entry->upage = upage;
    list_push_back(&thread_current()->supl_page_tbl, &entry->elem);

    return entry;
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

    // TODO(keegan): error handling?
    retrieve_page(entry, kpage);

    struct thread *t = thread_current();
    /* Map our page to the correct location. */
    if (!pagedir_set_page(t->pagedir, upage, kpage, entry->writable)) {
	// TODO(keegan): frame_destroy?
	return false;
    }
    
    return true;
}

/*! Returns true if the given address is a valid one. */
bool page_valid_addr (void* uaddr) {
    // TODO(keegan): Assert we're in kernel mode
    page_extra_stack (uaddr, thread_current()->user_esp);
    struct spt_entry* entry = get_spt_entry(uaddr);
    return (entry != NULL);
}

/*! Initialize the spt_entry variables. */
void init_spt_entry (struct spt_entry* entry) {
    entry->created = false;
    entry->upage = NULL;
    entry->file_ofs = 0;
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

/*! Retrieve the pages. This function is called by the page fault
  handler. If the specified spt page hasn't been created yet, create
  it. Returns true on success. */
bool retrieve_page (struct spt_entry* entry, void* kpage) {
    if (!entry->created) {
	if (entry->src == SPT_SRC_ZERO) {
	    memset(kpage, 0, PGSIZE);
	} else if (entry->src == SPT_SRC_EXEC) {
	    struct file *f = filesys_open(entry->filename);
	    if (f == NULL)
		return false;
	    file_seek(f, entry->file_ofs);
	    if (file_read(f, kpage, entry->read_bytes) !=
		(int) entry->read_bytes) {
		return false;
	    }
	    memset(kpage + entry->read_bytes, 0, PGSIZE - entry->read_bytes);
	    file_close(f);
	} else if (entry->src == SPT_SRC_FILE) {

	} else {
	    ASSERT(false);
	}
	entry->created = true;
    } else {

    }
    return true;
}

/*! Examines the faulting address. Heuristically determines if the
  address is likely due to stack growth. If it is, add a new page. 
  If adding the new page fails, the error will be caught later by the
  page fault handler. */
void page_extra_stack (void* uaddr, void* esp) {
    uintptr_t u = (uintptr_t)uaddr;
    uintptr_t e = (uintptr_t)esp;
    if (e <= u + 32 &&
        u < (unsigned)PHYS_BASE &&
	u >= (unsigned)(PHYS_BASE - MAX_STACK_SIZE)) {
	/* Install page. */
	void* upage = (void*)(u & ~PGMASK);
	struct spt_entry *ent = page_add_user(upage);
	if (ent) {
	    ent->src = SPT_SRC_ZERO;
	    ent->writable = true;
	}
    }
}

#pragma GCC pop_options
