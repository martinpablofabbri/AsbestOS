#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "threads/thread.h"
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/cache.h"
#include "filesys/directory.h"

/*! Partition that contains the file system. */
struct block *fs_device;

static void do_format(void);

/*! Initializes the file system module.
    If FORMAT is true, reformats the file system. */
void filesys_init(bool format) {
    fs_device = block_get_role(BLOCK_FILESYS);
    if (fs_device == NULL)
        PANIC("No file system device found, can't initialize file system.");

    cache_init(fs_device);
    inode_init();
    free_map_init();

    if (format) 
        do_format();

    free_map_open();
}

/*! Shuts down the file system module, writing any unwritten data to disk. */
void filesys_done(void) {
    free_map_close();
    cache_flush();
}

/*! Creates a file named NAME with the given INITIAL_SIZE.  Returns true if
    successful, false otherwise.  Fails if a file named NAME already exists,
    or if internal memory allocation fails. */
bool filesys_create(const char *name, off_t initial_size) {
    block_sector_t inode_sector = 0;

    struct dir *dir = thread_current()->pwd;
    char * file;
    if (!dir_parse((char*)name, &dir, &file))
        return false;

    bool success = (dir != NULL &&
                    free_map_allocate(1, &inode_sector) &&
                    inode_create(inode_sector, initial_size, false) &&
                    dir_add(dir, file, inode_sector));
    if (!success && inode_sector != 0) 
        free_map_release(inode_sector, 1);

    return success;
}

/*! Creates a directory at the path specified by NAME with the
    specified number of directory entries. Fails if a file named NAME
    already exists, or if internal memory allocation fails. */
bool filesys_create_dir(char* name, size_t entry_cnt) {
    block_sector_t inode_sector = 0;

    struct dir *dir = thread_current()->pwd;
    char * file;
    if (!dir_parse(name, &dir, &file))
        return false;

    bool success = (dir != NULL &&
                    free_map_allocate(1, &inode_sector) &&
                    dir_create(inode_sector, entry_cnt) &&
                    dir_add(dir, file, inode_sector));

    if (!success && inode_sector != 0) 
        free_map_release(inode_sector, 1);

    return success;
}

/*! Changes the directory to the one specified. */
bool filesys_change_dir(char* name) {
    struct dir *dir = thread_current()->pwd;
    struct dir *dir2;
    char * file;
    if (!dir_parse(name, &dir, &file))
        return false;

    struct inode* inode;
    if (dir_lookup(dir, file, &inode)) {
        dir2 = dir_open(inode);
        dir_close(thread_current()->pwd);
        dir_close(dir);
        thread_current()->pwd = dir2;
        return true;
    } else {
        dir_close(dir);
        return false;
    }
}

/*! Opens the file with the given NAME.  Returns the new file if successful
    or a null pointer otherwise.  Fails if no file named NAME exists,
    or if an internal memory allocation fails. */
struct file * filesys_open(const char *name) {
    struct inode *inode = NULL;

    if (strcmp(name, "/") == 0) {
        inode = inode_open(ROOT_DIR_SECTOR);
        return file_open(inode);
    }

    struct dir *dir = thread_current()->pwd;
    char * file;
    if (!dir_parse((char*)name, &dir, &file))
        return false;
    if (dir != NULL)
        dir_lookup(dir, file, &inode);
    dir_close(dir);

    return file_open(inode);
}

/*! Deletes the file named NAME.  Returns true if successful, false on failure.
    Fails if no file named NAME exists, or if an internal memory allocation
    fails. */
bool filesys_remove(const char *name) {
    struct dir *dir = dir_open_root();
    bool success = dir != NULL && dir_remove(dir, name);
    dir_close(dir);

    return success;
}

/*! Formats the file system. */
static void do_format(void) {
    printf("Formatting file system...");
    free_map_create();
    if (!dir_create(ROOT_DIR_SECTOR, 16))
        PANIC("root directory creation failed");
    free_map_close();
    printf("done.\n");
}

