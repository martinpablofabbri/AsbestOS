#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init(void);
void sys_exit (int status);

typedef int mapid_t;

#endif /* userprog/syscall.h */

