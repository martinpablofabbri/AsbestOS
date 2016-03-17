#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "devices/block.h"

struct bitmap;

void inode_init(void);
bool inode_create(block_sector_t, off_t);
struct inode *inode_open(block_sector_t);
struct inode *inode_reopen(struct inode *);
block_sector_t inode_get_inumber(const struct inode *);
void inode_close(struct inode *);
void inode_remove(struct inode *);
off_t inode_read_at(struct inode *, void *, off_t size, off_t offset);
off_t inode_write_at(struct inode *, const void *, off_t size, off_t offset);
void inode_deny_write(struct inode *);
void inode_allow_write(struct inode *);
off_t inode_length(const struct inode *);

/* inode index constants */
#define NUM_DIRECT 124
#define NUM_INDIRECT 1
#define NUM_DOUBLE_INDIRECT 1

/*! On-disk inode.
    Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk {
    // Metadata
    block_sector_t start;               /*!< First data sector. */
    off_t length;                       /*!< File size in bytes. */
    // Sector pointers
    block_sector_t direct[NUM_DIRECT];
    block_sector_t *indirect[NUM_INDIRECT];
    block_sector_t **double_indirect[NUM_DOUBLE_INDIRECT];

    
    // TODO(jg): remove
    /* unsigned magic;                     /\*!< Magic number. *\/ */
    /* uint32_t unused[125];               /\*!< Not used. *\/ */
};

block_sector_t* idx2block_sectorp (int block_idx, struct inode_disk *inode_disk_);

block_sector_t idx2block_sector (int block_idx, struct inode_disk *inode_disk_);
block_sector_t offset2block_sector (off_t offset, struct inode_disk *inode_disk_);
#endif /* filesys/inode.h */
