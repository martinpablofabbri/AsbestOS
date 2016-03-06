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
struct spt_entry* get_spt_entry (const void* uaddr);
bool retrieve_page (struct spt_entry* entry, void* kpage);

/*! Adds a new user page at the given user virtual address.
    Returns the spt_entry if the operation was successful.
*/
struct spt_entry* page_add_user (void* upage) {
    ASSERT(get_spt_entry(upage) == NULL);
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

/*! Maps a file into memory by creating several new user pages
    containing the file's data. Assumes that there are no conflicts
    with mapping the specified address and that the address is page
    aligned. Returns true on success. */
bool page_add_file (const char* fname, void* upage) {
    ASSERT(pg_ofs(upage) == 0);

    struct file* file = filesys_open(fname);
    if (!file)
        return false;

    off_t size = file_length(file);
    file_close(file);

    if (size == 0)
        return false;

    off_t ofs = 0;
    uint32_t read_bytes = size;
    uint32_t zero_bytes = (size % PGSIZE == 0) ? 0 : PGSIZE - (size % PGSIZE);

    while (read_bytes > 0 || zero_bytes > 0) {
        /* Calculate how to fill this page.
           We will read PAGE_READ_BYTES bytes from FILE
           and zero the final PAGE_ZERO_BYTES bytes. */
        size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
        size_t page_zero_bytes = PGSIZE - page_read_bytes;

	struct spt_entry* entry = page_add_user (upage);
        //TODO(keegan): on failure, do something about previously
        //allocated spt_entries
	if (entry == NULL)
	    return false;

	entry->src = SPT_SRC_FILE;
	strlcpy(entry->filename, fname, NAME_MAX + 1);
	entry->file_ofs = ofs;
	entry->read_bytes = page_read_bytes;
	entry->writable = true;
	ofs += page_read_bytes;

        /* Advance. */
        read_bytes -= page_read_bytes;
        zero_bytes -= page_zero_bytes;
        upage += PGSIZE;
    }
    return true;    
}


/*! Attempt to recover from a page fault at the specified address.
  Returns true if recovery was successful. */
bool page_fault_recover (const void* uaddr) {
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

/*! Returns true if the given address is a valid one. If WRITE is set
    and the page is not writable, the address is not valid. */
bool page_valid_addr (const void* uaddr, bool write) {
    // TODO(keegan): Assert we're in kernel mode
    page_extra_stack (uaddr, thread_current()->user_esp);
    struct spt_entry* entry = get_spt_entry(uaddr);
    return (entry != NULL && (!write || entry->writable));
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
struct spt_entry* get_spt_entry (const void* uaddr) {
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
void page_extra_stack (const void* uaddr, void* esp) {
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
