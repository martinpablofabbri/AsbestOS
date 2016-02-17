#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler(struct intr_frame *);
bool access_ok (const void *addr, unsigned long size);
int get_user_1 (const void *addr, uint8_t* dest);
int get_user_2 (const void *addr, uint16_t* dest);
int get_user_4 (const void *addr, uint32_t* dest);

static void sys_halt (void);
static void sys_exit (int status);
static tid_t sys_exec (const char *file);
static int sys_wait (tid_t tid);
static int sys_write (int fd, const void *buffer, unsigned size);
static bool sys_create(const char *name, uint32_t initial_size);
static bool sys_remove(const char *name);
static int sys_open(const char *name);
static int sys_filesize(int fd);
static void sys_close(int fd);
static struct lock filesys_lock;

// TODO(keegan): Does it make sense to have this function here?
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
    lock_init(&filesys_lock);
}


#define SYSCALL_0(f) ({				\
	    f();				\
	})

#define SYSCALL_1(f,t1) ({			\
	    uint32_t a1;			\
	    if (get_user_4(esp + 1, &a1) == -1)	\
		goto fail;			\
	    f((t1)a1);				\
	})

#define SYSCALL_2(f,t1,t2) ({			\
	    uint32_t a1, a2;			\
	    if (get_user_4(esp + 1, &a1) == -1)	\
		goto fail;			\
	    if (get_user_4(esp + 2, &a2) == -1)	\
		goto fail;			\
	    f((t1)a1,(t2)a2);			\
	})

#define SYSCALL_3(f,t1,t2,t3) ({		\
	    uint32_t a1, a2, a3;		\
	    if (get_user_4(esp + 1, &a1) == -1)	\
		goto fail;			\
	    if (get_user_4(esp + 2, &a2) == -1)	\
		goto fail;			\
	    if (get_user_4(esp + 3, &a3) == -1)	\
		goto fail;			\
	    f((t1)a1,(t2)a2,(t3)a3);		\
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
    case SYS_WRITE:
	*eax = SYSCALL_3(sys_write, int, void*, unsigned);
	break;
    case SYS_CREATE:
	// TODO(jg)
	*eax = SYSCALL_2(sys_create, const char*, uint32_t);
        break;
    case SYS_REMOVE:
	// TODO(jg)
	*eax = SYSCALL_1(sys_remove, const char*);
        break;
    case SYS_OPEN:
	// TODO(jg)
	*eax = SYSCALL_1(sys_open, const char*);
        break;
    case SYS_CLOSE:
	// TODO(jg)
	SYSCALL_1(sys_close, int);
        break;
    case SYS_FILESIZE:
	// TODO(jg)
	*eax = SYSCALL_1(sys_filesize, int);

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

static void sys_exit (int status) {
    printf("%s: exit(%d)\n", thread_name(), status);
    thread_current()->retval = status;
    thread_exit();
}

static tid_t sys_exec (const char *file) {
    tid_t ret = process_execute(file);
    return (ret == TID_ERROR) ? -1 : ret;
}

static int sys_wait (tid_t tid) {
    return process_wait(tid);
}

static int sys_write (int fd, const void *buffer, unsigned size UNUSED) {
    if (fd == 1)
	printf("%s",(char*)buffer);
    return 0;
}

static int fd_from_file (struct file * file) {
    // TODO(jg): turn file struct into a file descriptor (int)
    //           for now, just cast the file struct address into int
    int fd = (int) file;
    return fd;
}

static struct file * file_from_fd (int fd) {
    // TODO(jg) will change when change method of turning file structs to fds
    struct file *file;
    file = (struct file *) fd;
    return file;
}

static bool sys_create(const char *name, uint32_t initial_size) {
    if (name == NULL)
	sys_exit(-1);
    
    lock_acquire(&filesys_lock);

    bool ret = filesys_create(name, initial_size); 

    lock_release(&filesys_lock);
    return ret;
}

static bool sys_remove(const char *name) {
    lock_acquire(&filesys_lock);

    bool ret = filesys_remove(name); 

    lock_release(&filesys_lock);
    return ret;
}

static int sys_open(const char *name) {
    struct file *file;
    int fd;

    lock_acquire(&filesys_lock);

    file = filesys_open(name); 
    fd = fd_from_file(file);

    lock_release(&filesys_lock);
    return fd;
}

static int sys_filesize(int fd) {
    struct file *file;
    int length;

    lock_acquire(&filesys_lock);

    file = file_from_fd(fd);
    length = file_length(file);

    lock_release(&filesys_lock);
    return length;
}

static void sys_close(int fd) {
    struct file *file;
    lock_acquire(&filesys_lock);

    file = file_from_fd(fd);
    file_close(file);

    lock_release(&filesys_lock);
}
