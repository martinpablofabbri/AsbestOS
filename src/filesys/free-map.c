#include "filesys/free-map.h"
#include <bitmap.h>
#include <debug.h>
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/inode.h"

static struct file *free_map_file;   /*!< Free map file. */
static struct bitmap *free_map;      /*!< Free map, one bit per sector. */

/*! Initializes the free map. */
void free_map_init(void) {
    free_map = bitmap_create(block_size(fs_device));
    if (free_map == NULL)
        PANIC("bitmap creation failed--file system device is too large");
    bitmap_mark(free_map, FREE_MAP_SECTOR);
    bitmap_mark(free_map, ROOT_DIR_SECTOR);
}

/*! Allocates CNT consecutive sectors from the free map and stores the first
    into *SECTORP.
    Returns true if successful, false if not enough consecutive sectors were
    available or if the free_map file could not be written. */
bool free_map_allocate(size_t cnt, block_sector_t *sectorp) {
    block_sector_t sector = bitmap_scan_and_flip(free_map, 0, cnt, false);
    if (sector != BITMAP_ERROR && free_map_file != NULL &&
        !bitmap_write(free_map, free_map_file)) {
        bitmap_set_multiple(free_map, sector, cnt, false); 
        sector = BITMAP_ERROR;
    }
    if (sector != BITMAP_ERROR)
        *sectorp = sector;
    return sector != BITMAP_ERROR;
}

bool free_map_index_allocate(int sectors, struct inode_disk* inode_disk_) {
    // TODO(jg) (done?)
    int sector_idx;
    for (sector_idx = 0; sector_idx < sectors; ++sector_idx) {
	block_sector_t *sectorp = idx2block_sectorp(sector_idx, inode_disk_);
	block_sector_t sector = bitmap_scan_and_flip(free_map, 0, 1, false);

	if (sector != BITMAP_ERROR && free_map_file != NULL &&
	    !bitmap_write(free_map, free_map_file)) {
	    // We found a free sector, the free_map_file exists,
	    // but we could not write the free_map to the free_map file

	    // So we set the block we found to free again and indicate error
	    bitmap_set_multiple(free_map, sector, 1, false);
	    sector = BITMAP_ERROR;
	}

	// If we encountered no errors finding a free sector,
	// put the sector number into the index and move on to
	// allocating the next sector
	if (sector != BITMAP_ERROR) {
	    *sectorp = sector;
	    continue;
	}

	// If we did encounter an error, deallocate all of the sectors we have
	// allocated so far in this function and return an error.
	if (sector == BITMAP_ERROR) {
	    int sector_idx_to_clear;
	    for (sector_idx_to_clear = 0;
		 sector_idx_to_clear < sector_idx;
		 ++sector_idx_to_clear) {
		bitmap_set_multiple(free_map, idx2block_sector(sector_idx, inode_disk_), 1, false);
	    }
	    return false;
	}
    }
    // All sectors success
    return true;
}

/*! Makes CNT sectors starting at SECTOR available for use. */
void free_map_release(block_sector_t sector, size_t cnt) {
    ASSERT(bitmap_all(free_map, sector, cnt));
    bitmap_set_multiple(free_map, sector, cnt, false);
    bitmap_write(free_map, free_map_file);
}

/*! Opens the free map file and reads it from disk. */
void free_map_open(void) {
    free_map_file = file_open(inode_open(FREE_MAP_SECTOR));
    if (free_map_file == NULL)
        PANIC("can't open free map");
    if (!bitmap_read(free_map, free_map_file))
        PANIC("can't read free map");
}

/*! Writes the free map to disk and closes the free map file. */
void free_map_close(void) {
    file_close(free_map_file);
}

/*! Creates a new free map file on disk and writes the free map to it. */
void free_map_create(void) {
    /* Create inode. */
    if (!inode_create(FREE_MAP_SECTOR, bitmap_file_size(free_map)))
        PANIC("free map creation failed");

    /* Write bitmap to file. */
    free_map_file = file_open(inode_open(FREE_MAP_SECTOR));
    if (free_map_file == NULL)
        PANIC("can't open free map");
    if (!bitmap_write(free_map, free_map_file))
        PANIC("can't write free map");
}

