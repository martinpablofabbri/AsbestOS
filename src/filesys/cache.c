#include "cache.h"

#include <stdint.h>
#include <string.h>
#include "threads/malloc.h"
#include "threads/synch.h"

#define CACHE_SIZE 64

enum cache_entry_status {
    ALIVE,
    DIRTY,
    EMPTY
};

struct cache_entry {
    block_sector_t sector;
    block_sector_t evict_sector;
    enum cache_entry_status status;
    uint8_t* data;
    struct lock ent_lock;
};

struct block* fs_block;
uint8_t* block_cache;
struct cache_entry* cache_info;
struct lock cache_info_lock;                 /*! Lock the cache_info to prevent
                                               updates. */

struct cache_entry* entry_acquire (block_sector_t sector);
struct cache_entry* cache_lookup (block_sector_t sector);
struct cache_entry* cache_get_empty (void);
struct cache_entry* cache_choose_evictee (void);
void cache_evict (struct cache_entry* ent);

/*! Initialize the cache. */
void cache_init(struct block* block) {
    fs_block = block;

    block_cache = (uint8_t*)malloc(CACHE_SIZE * BLOCK_SECTOR_SIZE);
    ASSERT(block_cache != NULL);

    cache_info = (struct cache_entry*)malloc(CACHE_SIZE * 
                                             sizeof(struct cache_entry));
    ASSERT(cache_info != NULL);

    int i;
    for (i = 0; i < CACHE_SIZE; i++) {
        cache_info[i].status = EMPTY;
        cache_info[i].data = &block_cache[i * BLOCK_SECTOR_SIZE];
        lock_init(&cache_info[i].ent_lock);
    }

    lock_init(&cache_info_lock);
}

/*! Read a block from the cache, bringing the page in from the disk if
  necessary. */
void cache_read(block_sector_t sector, void* buffer, off_t size, off_t offset) {
    struct cache_entry* ent = entry_acquire(sector);
    memcpy(buffer, ent->data + offset, size);
    lock_release(&ent->ent_lock);
}

/*! Write information to the cache. */
void cache_write(block_sector_t sector, const void *buffer,
                 off_t size, off_t offset) {
    struct cache_entry* ent = entry_acquire(sector);
    memcpy(ent->data + offset, buffer, size);
    ent->status = DIRTY;
    lock_release(&ent->ent_lock);
}

/*! Evict all entries from the cache and write back to disk. Must be
    called right before shutdown. */
void cache_flush (void) {
    lock_acquire(&cache_info_lock);
    int i;
    struct cache_entry* ent;
    for (i = 0; i < CACHE_SIZE; i++) {
        ent = &cache_info[i];
        if (ent->status == DIRTY) {
            lock_acquire(&ent->ent_lock);
            ent->sector = -1;
            lock_release(&cache_info_lock);
            cache_evict(&cache_info[i]);
            lock_acquire(&cache_info_lock);
            lock_release(&ent->ent_lock);
        }
    }
    lock_release(&cache_info_lock);
    free(cache_info);
    free(block_cache);
}

/*! Acquires an entry for the specified sector. The entry will be
  locked upon exit from the routine. */
struct cache_entry* entry_acquire (block_sector_t sector) {
    struct cache_entry* ent;
    lock_acquire(&cache_info_lock);
    ent = cache_lookup(sector);

    if ((ent = cache_lookup(sector))) {
        // See if it already exists
        lock_acquire(&ent->ent_lock);
        lock_release(&cache_info_lock);
    } else {
        if ((ent = cache_get_empty())) {
            // See if there's an empty cache slot
            lock_acquire(&ent->ent_lock);
            /* It's okay to update the sector and status while the
            actual data is not up to date, since the entry_lock will
            be held until the data is read in. */
            ent->sector = sector;
            ent->status = ALIVE;
            lock_release(&cache_info_lock);
        } else {
            ent = cache_choose_evictee();
            if (!ent) {
                PANIC("Could not acquire a filesystem cache entry.");
            }
            lock_acquire(&ent->ent_lock);
            ent->sector = sector;
            lock_release(&cache_info_lock);
            cache_evict(ent);
        }
        ent->status = ALIVE;
        ent->evict_sector = sector;

        ASSERT(!lock_held_by_current_thread(&cache_info_lock));
        ASSERT(lock_held_by_current_thread(&ent->ent_lock));
        block_read(fs_block, sector, ent->data);
    }
    return ent;
}


/*! Look for a certain sector in the cache, and return a pointer to
  the cache entry if found. Return NULL otherwise. */
struct cache_entry* cache_lookup (block_sector_t sector) {
    ASSERT(lock_held_by_current_thread(&cache_info_lock));
    //TODO(keegan): Replace with hash map?
    int i;
    struct cache_entry* ent;
    for (i=0; i<CACHE_SIZE; i++) {
        ent = &cache_info[i];
        if ((ent->status == ALIVE || ent->status == DIRTY) &&
            ent->sector == sector)
            return ent;
    }
    return NULL;
}

/*! Returns an empty cache entry.
  The cache_info_lock must be locked prior to entry.
  Returns NULL if unsuccessful. */
struct cache_entry* cache_get_empty (void) {
    ASSERT(lock_held_by_current_thread(&cache_info_lock));

    int i;
    struct cache_entry* ent;
    for (i=0; i<CACHE_SIZE; i++) {
        ent = &cache_info[i];
        if (ent->status == EMPTY) {
            return ent;
        }
    }
    return NULL;
}

/*! Selects a cache entry to be evicted. 
 The cache_info_lock must be locked prior to entry.
 Returns NULL if unsuccessful. */
struct cache_entry* cache_choose_evictee (void) {
    ASSERT(lock_held_by_current_thread(&cache_info_lock));

    // TODO(keegan): Better eviction algorithm.
    static int ind = 0;
    struct cache_entry* ent = &cache_info[ind];
    ind = (ind + 1) % CACHE_SIZE;
    return ent;
}

/*! Evicts a cache entry to disk. The cache_info_lock must not be
    held by the current thread. */
void cache_evict (struct cache_entry* ent) {
    ASSERT(!lock_held_by_current_thread(&cache_info_lock));
    ASSERT(ent != NULL);
    ASSERT(lock_held_by_current_thread(&ent->ent_lock));

    if (ent->status == DIRTY) {
        block_write(fs_block, ent->evict_sector, ent->data);
    }
}
