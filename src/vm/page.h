#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <list.h>
#include <stdbool.h>

#include "vm/frame.h"
#include "vm/swap.h"
#include "filesys/off_t.h"
#include "filesys/directory.h"

/*! Possible sources of data for a supplemental page table entry. */
enum spt_source {
    SPT_SRC_FILE,                      /*!< The data is mapped from a
					 file. */
    SPT_SRC_EXEC,                      /*!< The data comes from the
					 executable. */
    SPT_SRC_ZERO                       /*!< The data is all zeros. */
};

/*! Represent a page of memory owned by the current user process.
 */
struct spt_entry {
    void* upage;                       /*!< Address of the user
					 virtual page represented by
					 this entry. */
    struct list_elem elem;             /*!< List element. */
    bool created;                      /*!< Has the page been created
					 yet? */
    enum spt_source src;               /*!< What is the source of the
					 data? */
    char filename[NAME_MAX+1];         /*!< Filename of the data
					 source. */
    off_t file_ofs;                    /*!< Offset into the file. */
    size_t read_bytes;                 /*!< Number of bytes to
					 read. */
    bool writable;                     /*!< Is the page writable? */
    
    swap_info_t swap_info;             /*!< Where in swap is the data? */
    struct frame_entry* frame;         /*!< Which frame the page is in. */
    struct thread* thread;             /*!< Thread which owns the page. */
};

void page_init (void);
struct spt_entry* page_add_user (void* upage);
bool page_add_file (const char* fname, void* upage);
bool page_remove_file (const char* fname, void* upage);
bool page_fault_recover (const void* uaddr);
bool page_valid_addr (const void* uaddr, bool write);
void page_extra_stack (const void* uaddr, void* esp);
bool page_evict (struct spt_entry* entry);
void page_kill_all (void);

// include "threads/pte.h"

// TODO
// basic page table structure +

//             +-----------------------------------+
//             |       Valid   Dirty   Address     |
//             |     +_______+_______+_________+   |
//             |PTE0 |       |       |         |   |
//             |PTE1 |       |       |         |   |
//             |PTE2 |       |       |         |   |
//             |.... |       |       |         |   |
//             |PTEn |       |       |         |   |
//             |     +_______+_______+_________+   |
//             +-----------------------------------+
//
//             supplemental page table data: so something like
//             +-------------------------------------------------------------+
//             |       read/write    start/   location of        Flags       |
//             |       permissions   stop         data                       |
//             |     +-------------+-------+--------------+-------------+    |
//             |PTE0 |             |       |              |             |    |
//             |PTE1 |             |       |              |             |    |
//             |PTE2 |             |       |              |             |    |
//             |.... |             |       |              |             |    |
//             |PTEn |             |       |              |             |    |
//             |     +-------------+-------+--------------+-------------+    |
//             +-------------------------------------------------------------+
//
//
//             page fault handler (called inside exception.c)
//
//                 get supplemental pte
//                 get frame
//                 copy data
//
//
//
//
//
//
//             structs
//
//             frame
//                 spt_entry*
//                 kernel vaddr
//                 lock
//
//             spt_entry
//                 pte*
//                 swap info
//                 start uvaddr
//                 (all same size, so no stop)

#endif /* vm/page.h */

