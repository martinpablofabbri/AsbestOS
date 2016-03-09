/*! \file thread.h
 *
 * Declarations for the kernel threading functionality in PintOS.
 */

#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include <hash.h>
#include "filesys/file.h"
#include "userprog/syscall.h"
#include "threads/synch.h"

/*! States in a thread's life cycle. */
enum thread_status {
    THREAD_RUNNING,     /*!< Running thread. */
    THREAD_READY,       /*!< Not running but ready to run. */
    THREAD_BLOCKED,     /*!< Waiting for an event to trigger. */
    THREAD_DYING        /*!< About to be destroyed. */
};

/*! Thread identifier type.
    You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /*!< Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /*!< Lowest priority. */
#define PRI_DEFAULT 31                  /*!< Default priority. */
#define PRI_MAX 63                      /*!< Highest priority. */

//// File memory mapping
typedef struct _mmapitem {
    struct hash_elem elem;
    mapid_t mapid;
    struct file *file;
    void *addr;
} mmap_item;


unsigned mmap_hash_func (const struct hash_elem *element, void *aux);
bool mmap_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux);

/*! A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

\verbatim
        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+
\endverbatim

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion.

   The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list.
*/
struct thread {
    /*! Owned by thread.c. */
    /**@{*/
    tid_t tid;                          /*!< Thread identifier. */
    enum thread_status status;          /*!< Thread state. */
    char name[16];                      /*!< Name (for debugging purposes). */
    uint8_t *stack;                     /*!< Saved stack pointer. */
    int priority;                       /*!< Priority. */
    struct list_elem allelem;           /*!< List element for all threads list. */
    struct child_info* child_head;      /*!< Head of the doubly linked
					  list of children. */
    struct child_info* self_info;       /*!< Pointer to the child_info
                                           struct held by this
                                           thread's parent. */
    int retval;                         /*!< The return value of a
					  thread. Initialized to -1. */
    /**@}*/

    /*! Shared between thread.c and synch.c. */
    /**@{*/
    struct list_elem elem;              /*!< List element. */
    /**@}*/

#ifdef USERPROG
    /*! Owned by userprog/process.c. */
    /**@{*/
    uint32_t *pagedir;                  /*!< Page directory. */
    struct list open_files;             /*!< List of files opened by
                                           the process. */
    int lowest_available_fd;            /*!< Keeps track of which file
					  descriptor to give out next. */
    struct file *executing_file;        /*!< Reference to the file
                                           descriptor of the current
                                           executable. */
    void* user_esp;                     /*!< When the program is
                                           executing, the stack
                                           pointer of the user
                                           program. */
    /**@}*/
#endif

#ifdef VM
    /*! Owned by vm/page.c. */
    /**@{*/
    struct list supl_page_tbl;          /*!< List of pages used by the
					  process. */
    int last_unused_mmap_id;
    struct hash mmap_mappings;   /*!< Hashtable of file-memory
                                          mappings. */
    /**@}*/
#endif    

    /*! Owned by thread.c. */
    /**@{*/
    unsigned magic;                     /* Detects stack overflow. */
    /**@}*/
};

/*! Information that a parent thread maintains about each of its
  children. */
struct child_info {
    struct child_info* prev;            /*!< Pointer to the previous
					  child_info struct in the
					  parent's list of children. */
    struct child_info* next;            /*!< Pointer to the next
					  child_info struct in the
					  parent's list of children. */

    tid_t child_tid;                    /*!< The thread id of the
					  child process which will
					  fill in the retval field. */

    int retval;                         /*!< Return code of the child
					  process. Defaults to -1. */

    bool child_is_dead;                 /*!< Flag used to indicate
					  when a child has died, and
					  thus when retval is
					  accurate. */
    struct lock child_lock;             /*!< Lock that is accessible
					  by both the parent and the
					  parent's child with the
					  given tid. */
    struct condition has_exited;        /*!< Condition variable used
                                           by the parent and the child
                                           to signal the death of
                                           child. */
};


/*! If false (default), use round-robin scheduler.
    If true, use multi-level feedback queue scheduler.
    Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init(void);
void thread_start(void);

void thread_tick(void);
void thread_print_stats(void);

typedef void thread_func(void *aux);
tid_t thread_create(const char *name, int priority, thread_func *, void *);

void thread_block(void);
void thread_unblock(struct thread *);

struct thread *thread_current (void);
tid_t thread_tid(void);
const char *thread_name(void);

void thread_exit(void) NO_RETURN;
void thread_yield(void);

/*! Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func(struct thread *t, void *aux);

void thread_foreach(thread_action_func *, void *);

int thread_get_priority(void);
void thread_set_priority(int);

int thread_get_nice(void);
void thread_set_nice(int);
int thread_get_recent_cpu(void);
int thread_get_load_avg(void);

struct thread *thread_by_tid(tid_t tid);
void init_child_info(struct child_info *child);

#endif /* threads/thread.h */

