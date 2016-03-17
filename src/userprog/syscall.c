#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include <list.h>
#include <string.h>
#include "devices/shutdown.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler(struct intr_frame *);
bool access_ok (const void *addr, unsigned long size);
int get_user_1 (const void *addr, uint8_t* dest);
int get_user_2 (const void *addr, uint16_t* dest);
int get_user_4 (const void *addr, uint32_t* dest);
static struct file_item * fileitem_from_fd (int fd);

static void sys_halt (void);
static tid_t sys_exec (const char *file);
static int sys_wait (tid_t tid);
static int sys_read (int fd, void *buffer, unsigned size);
static int sys_write (int fd, const void *buffer, unsigned size);
static bool sys_create(const char *name, uint32_t initial_size);
static bool sys_remove(const char *name);
static int sys_open(const char *name);
static int sys_filesize(int fd);
static void sys_seek (int fd, unsigned position);
static unsigned sys_tell (int fd);
static void sys_close(int fd);
static bool sys_mkdir(const char* dir);
static bool sys_chdir(const char* dir);

/* Struct for list element with a file and file descriptor */
struct file_item {
    struct list_elem elem;               /*!< List element of
					   file_item. */
    struct file *file;                   /*!< Reference to file
					   returned by filesys code. */
    int fd;                              /*!< File descriptor
					   associated with the file. */
};

bool access_ok (const void *addr, unsigned long size) {
    // Function signature as approximately in "Understanding the Linux
    // Kernel," pg 412
    unsigned long a = (unsigned long) addr;
    if (a + size - 1 < a ||
        !is_user_vaddr(addr) ||
        !is_user_vaddr(addr + size - 1))
        return false;

    // TODO(keegan): this doesn't allow for the possibility of crossing
    // multiple page boundaries
    if (!pagedir_get_page(thread_current()->pagedir, addr) ||
        !pagedir_get_page(thread_current()->pagedir, addr + size - 1))
        return false;

    return true;
}

int get_user_1 (const void *addr, uint8_t* dest) {
    if (!access_ok(addr, 1))
        return -1;
    *dest = *(uint8_t*)addr;
    return 0;
}

int get_user_2 (const void *addr, uint16_t* dest) {
    if (!access_ok(addr, 2))
        return -1;
    *dest = *(uint16_t*)addr;
    return 0;
}

int get_user_4 (const void *addr, uint32_t* dest) {
    if (!access_ok(addr, 4))
        return -1;
    *dest = *(uint32_t*)addr;
    return 0;
}

void syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}


#define SYSCALL_0(f) ({                         \
            f();                                \
        })

#define SYSCALL_1(f,t1) ({                      \
            uint32_t a1;                        \
            if (get_user_4(esp + 1, &a1) == -1) \
                goto fail;                      \
            f((t1)a1);                          \
        })

#define SYSCALL_2(f,t1,t2) ({                   \
            uint32_t a1, a2;                    \
            if (get_user_4(esp + 1, &a1) == -1) \
                goto fail;                      \
            if (get_user_4(esp + 2, &a2) == -1) \
                goto fail;                      \
            f((t1)a1,(t2)a2);                   \
        })

#define SYSCALL_3(f,t1,t2,t3) ({                \
            uint32_t a1, a2, a3;                \
            if (get_user_4(esp + 1, &a1) == -1) \
                goto fail;                      \
            if (get_user_4(esp + 2, &a2) == -1) \
                goto fail;                      \
            if (get_user_4(esp + 3, &a3) == -1) \
                goto fail;                      \
            f((t1)a1,(t2)a2,(t3)a3);            \
        })


static void syscall_handler(struct intr_frame *f) {
    uint32_t* esp = f->esp;
    int *eax = (int *)&f->eax;

    uint32_t syscall_num;
    if (get_user_4(esp, &syscall_num) == -1)
        goto fail;

    switch (syscall_num) {
    case SYS_HALT:
        SYSCALL_0(sys_halt);
        break;
    case SYS_EXIT:
        SYSCALL_1(sys_exit, int);
        break;
    case SYS_EXEC:
        *eax = SYSCALL_1(sys_exec, const char*);
        break;
    case SYS_WAIT:
        *eax = SYSCALL_1(sys_wait, tid_t);
        break;
    case SYS_READ:
        *eax = SYSCALL_3(sys_read, int, void*, unsigned);
        break;
    case SYS_WRITE:
        *eax = SYSCALL_3(sys_write, int, void*, unsigned);
        break;
    case SYS_CREATE:
        *eax = SYSCALL_2(sys_create, const char*, uint32_t);
        break;
    case SYS_REMOVE:
        *eax = SYSCALL_1(sys_remove, const char*);
        break;
    case SYS_OPEN:
        *eax = SYSCALL_1(sys_open, const char*);
        break;
    case SYS_CLOSE:
        SYSCALL_1(sys_close, int);
        break;
    case SYS_FILESIZE:
        *eax = SYSCALL_1(sys_filesize, int);
        break;
    case SYS_SEEK:
        SYSCALL_2(sys_seek, int, unsigned);
        break;
    case SYS_TELL:
        *eax = SYSCALL_1(sys_tell, int);
        break;
    case SYS_MKDIR:
        *eax = SYSCALL_1(sys_mkdir, const char*);
        break;
    case SYS_CHDIR:
        *eax = SYSCALL_1(sys_chdir, const char*);
        break;
    default:
        printf("Syscall %u: Not implemented.\n", syscall_num);
    }
    return;

 fail:
    sys_exit(-1);
}

static void sys_halt (void) {
    shutdown_power_off();
}

void sys_exit (int status) {
    /* Clean up any file descriptors. */
    struct list *open_files = &thread_current()->open_files;

    while (!list_empty(open_files)) {
	struct file_item *fitem = list_entry(list_pop_front(open_files),
					     struct file_item,
					     elem);
	file_close(fitem->file);
	free(fitem);
    }

    printf("%s: exit(%d)\n", thread_name(), status);
    thread_current()->retval = status;
    thread_exit();
}

static tid_t sys_exec (const char *file) {
    if (!access_ok(file, 1))
        return -1;
    tid_t ret = process_execute(file);
    return (ret == TID_ERROR) ? -1 : ret;
}

static int sys_wait (tid_t tid) {
    return process_wait(tid);
}

//// File system syscalls

/* Get a file descriptor using a struct file*.
 *  Returns -1 on not finding file */
static int fd_from_file (struct file *file) {
    struct thread *curr_thread = thread_current();
    struct list_elem *e;
    for (e = list_begin(&curr_thread->open_files);
         e != list_end(&curr_thread->open_files);
         e = list_next(e)) {
        struct file_item *fitem = list_entry(e, struct file_item, elem);
        if (fitem->file == file) {
            return fitem->fd;
        }
    }
    // Did not find files with matching fd
    return -1;
}

/* Get a fileitem from file descriptor.
 * Return NULL on not finding fileitem. */
static struct file_item * fileitem_from_fd (int fd) {
    struct thread *curr_thread = thread_current();
    struct list_elem *e;
    for (e = list_begin(&curr_thread->open_files);
         e != list_end(&curr_thread->open_files);
         e = list_next(e)) {
        struct file_item *fitem = list_entry(e, struct file_item, elem);
        if (fitem->fd == fd) {
            return fitem;
        }
    }
    // Did not find files with matching fd
    return NULL;
}

/* Read size bytes from file, copy into butter and return bytes read */
static int sys_read (int fd, void *buffer, unsigned size) {
    // Check for size 0
    if (size == 0) {
        return 0;
    }

    // Access Checks
    if (!access_ok(buffer, size)) {
        sys_exit(-1);
    }

    struct file *file;
    int bytes_read;

    struct file_item *fitem = fileitem_from_fd(fd);
    if (fitem == NULL) {
        return -1;
        // no matching file descriptor
    }
    file = fitem->file;

    if (file_is_dir(file))
        return -1;

    bytes_read = file_read(file, buffer, size);

    return bytes_read;

}

/* Write size bytes into file, write from buffer and return bytes written */
static int sys_write (int fd, const void *buffer, unsigned size) {
    // Check for size 0
    if (size == 0) {
        return 0;
    }

    // Access Checks
    if (!access_ok(buffer, size)) {
        sys_exit(-1);
    }

    // Case when writing to STDOUT
    if (fd == 1) {
        printf("%s",(char*)buffer);
        return size;
    }

    struct file *file;
    int bytes_written;

    struct file_item *fitem = fileitem_from_fd(fd);
    if (fitem == NULL) {
        return -1;
        // no matching file descriptor
    }
    file = fitem->file;

    if (file_is_dir(file))
        return -1;

    bytes_written = file_write(file, buffer, size);

    return bytes_written;

}

/* Create a file. sys_exits with error if passed an invalid name ptr */
static bool sys_create(const char *name, uint32_t initial_size) {
    if (name == NULL || !access_ok((void*) name, sizeof(const char *)))
	sys_exit(-1);

    char* k_name = (char*)malloc(READDIR_MAX_LEN + 1);
    if (!k_name)
        return false;

    strlcpy(k_name, name, READDIR_MAX_LEN + 1);

    bool ret = filesys_create(k_name, initial_size);
    free(k_name);

    return ret;
}

/* Remove file. sys_exits with error if passed an invalid name ptr */
static bool sys_remove(const char *name) {
    if (name == NULL || !access_ok((void*) name, sizeof(const char *)))
	sys_exit(-1);

    bool ret = filesys_remove(name);
    return ret;
}

/* Open file. sys_exits with error if file not found. */
static int sys_open(const char *name) {
    struct file *file;
    int fd;

    if (name == NULL || !access_ok((void*) name, sizeof(const char *)))
	sys_exit(-1);

    char* k_name = (char*)malloc(READDIR_MAX_LEN + 1);
    if (!k_name)
        return false;
    strlcpy(k_name, name, READDIR_MAX_LEN + 1);

    struct file_item *fitem = malloc(sizeof(struct file_item));
    file = filesys_open(k_name);
    free(k_name);
    if (file == NULL) {
        return -1;
    }

    fitem->file = file;
    struct thread *curr_thread = thread_current();
    fd = curr_thread->lowest_available_fd;
    curr_thread->lowest_available_fd += 1;

    fitem->fd = fd;

    list_push_back(&curr_thread->open_files, &fitem->elem);
    return fd;
}

/* Get file size */
static int sys_filesize(int fd) {
    struct file *file;
    int length;

    file = fileitem_from_fd(fd)->file;
    length = file_length(file);

    return length;
}

/* Move current position into file to new position */
static void sys_seek (int fd, unsigned position) {
    struct file *file;

    struct file_item *fitem = fileitem_from_fd(fd);
    if (fitem == NULL) {
        return;
        // no matching file descriptor
    }
    file = fitem->file;
    file_seek(file, position);
}

/* Get currect offset into file */
static unsigned sys_tell (int fd) {
    struct file *file;
    unsigned pos;

    struct file_item *fitem = fileitem_from_fd(fd);
    if (fitem == NULL) {
        return 0;
        // no matching file descriptor
    }
    file = fitem->file;
    pos = file_tell(file);

    return pos;
}

/* Close file using file descriptor. */
static void sys_close(int fd) {
    struct file *file;

    struct file_item *fitem = fileitem_from_fd(fd);
    if (fitem == NULL) {
        // No matching file descriptor. File possibly already closed.
        return;
    }
    file = fitem->file;
    file_close(file);
    // remove file_item from opened files list and free
    list_remove(&fitem->elem);
    free(fitem);
}

/* Make a directory. */
static bool sys_mkdir(const char* dir) {
    if (dir == NULL || !access_ok((void*) dir, sizeof(const char *)))
	sys_exit(-1);

    char* k_dir = (char*)malloc(READDIR_MAX_LEN + 1);
    if (!k_dir)
        return false;

    strlcpy(k_dir, dir, READDIR_MAX_LEN + 1);

    // TODO(keegan): don't have the 16 be constant
    bool ret = filesys_create_dir(k_dir, 16);

    free(k_dir);

    return ret;
}

/* Change directories. */
static bool sys_chdir(const char* dir) {
    if (dir == NULL || !access_ok((void*) dir, sizeof(const char *)))
	sys_exit(-1);

    char* k_dir = (char*)malloc(READDIR_MAX_LEN + 1);
    if (!k_dir)
        return false;

    strlcpy(k_dir, dir, READDIR_MAX_LEN + 1);

    bool ret = filesys_change_dir(k_dir);

    free(k_dir);

    return ret;
}
