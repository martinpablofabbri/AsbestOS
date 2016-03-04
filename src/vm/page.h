#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stdbool.h>

bool add_user_page (void* upage);

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

