#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/pagedir.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler(struct intr_frame *);
bool access_ok (const void *addr, unsigned long size);
int get_user_1 (const void *addr, uint8_t* dest);
int get_user_2 (const void *addr, uint16_t* dest);
int get_user_4 (const void *addr, uint32_t* dest);

static void sys_halt (void);
static void sys_exit (int status);
static int sys_write (int fd, const void *buffer, unsigned size);

// TODO(keegan): Does it make sense to have this function here?
#pragma GCC push_options
#pragma GCC optimize ("O0")
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
#pragma GCC pop_options

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
    uint32_t* esp = f->esp;
    int *eax = (int *)&f->eax;

    //if (!access_ok(esp, 4)) {
    //goto fail;
    //}

    uint32_t syscall_num;
    if (get_user_4(esp, &syscall_num) == -1)
	goto fail;

    /* Get arguments from stack. */
    uint32_t a1, a2, a3;
    int valid_args = 3;
    valid_args += get_user_4(esp + 1, &a1);
    valid_args += get_user_4(esp + 2, &a2);
    valid_args += get_user_4(esp + 3, &a3);

    switch (syscall_num) {
    case SYS_HALT:
	sys_halt();
	break;
    case SYS_EXIT:
	if (valid_args < 1)
	    goto fail;
	sys_exit((int)a1);
	break;
    case SYS_WRITE:
	sys_write((int)a1, (void*)a2, (unsigned)a3);
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

static void sys_exit (int status) {
    printf("%s: exit(%d)\n", thread_name(), status);
    thread_exit();
}

static int sys_write (int fd, const void *buffer, unsigned size UNUSED) {
    if (fd == 1)
	printf("%s",(char*)buffer);
    return 0;
}
