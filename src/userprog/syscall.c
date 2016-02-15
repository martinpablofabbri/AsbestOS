#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/pagedir.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler(struct intr_frame *);
int access_ok (const void *addr, unsigned long size);
int get_user_1 (const void *addr, uint8_t* dest);
int get_user_2 (const void *addr, uint16_t* dest);
int get_user_4 (const void *addr, uint32_t* dest);

// TODO(keegan): Does it make sense to have this function here?
int access_ok (const void *addr, unsigned long size) {
    // Function signature as in "Understanding the Linux Kernel," pg 412
    unsigned long a = (unsigned long) addr;
    if (a + size < a ||
	!is_user_vaddr(addr) ||
	!is_user_vaddr(addr + size))
	return 0;

    // TODO(keegan): this doesn't allow for the possibility of crossing
    // multiple page boundaries
    if (!pagedir_get_page(thread_current()->pagedir, addr) ||
	!pagedir_get_page(thread_current()->pagedir, addr + size))
	return 0;

    return 1;
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

static void syscall_handler(struct intr_frame *f) {
    printf("system call!\n");
    uint32_t* esp = f->esp;
    void *eax = &f->eax;
    uint32_t syscall_num = esp[0];

    switch (syscall_num) {
    case SYS_HALT:
	sys_halt();
	break;
    case SYS_EXIT:
	sys_exit((int)esp[1]);
	break;
    case SYS_WRITE:
	sys_write((int)esp[1], (void*)esp[2], (unsigned)esp[3]);
	break;
    default:
	printf("Syscall %u: Not implemented.\n", syscall_num);
    }
}

void sys_halt () {

}

void sys_exit (int status) {
    // TODO: implement.
    printf("Exiting with code %d\n", status);
    thread_exit();
}

int sys_write (int fd, const void *buffer, unsigned size) {
    if (fd == 1)
	printf("%s",buffer);
}
