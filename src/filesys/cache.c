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
    enum cache_entry_status status;
    uint8_t* data;
};

struct block* fs_block;
uint8_t* block_cache;
struct cache_entry* cache_info;
struct lock cache_lock_;

struct cache_entry* cache_lookup (block_sector_t sector);
struct cache_entry* cache_load (block_sector_t sector);
struct cache_entry* cache_get_empty (void);
struct cache_entry* cache_choose_evictee (void);
void cache_evict (struct cache_entry* ent);

void cache_lock(void);
void cache_unlock(void);

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
    }

    lock_init(&cache_lock_);
}

/*! Read a block from the cache, bringing the page in from the disk if
  necessary. */
void cache_read(block_sector_t sector, void* buffer, off_t size, off_t offset) {
    cache_lock();

    struct cache_entry* ent = cache_lookup(sector);
    if (!ent)
        ent = cache_load(sector);

    memcpy(buffer, ent->data + offset, size);

    cache_unlock();
}

/*! Write information to the cache. */
void cache_write(block_sector_t sector, const void *buffer, off_t size, off_t offset) {
    cache_lock();

    struct cache_entry* ent = cache_lookup(sector);
    if (!ent)
        ent = cache_load(sector);

    memcpy(ent->data + offset, buffer, size);
    ent->status = DIRTY;
    
    cache_unlock();
}

/*! Evict all entries from the cache and write back to disk. */
void cache_flush (void) {
    cache_lock();
    int i;
    for (i = 0; i < CACHE_SIZE; i++) {
        if (cache_info[i].status != EMPTY)
            cache_evict(&cache_info[i]);
    }
    free(cache_info);
    free(block_cache);
    cache_unlock();
}

/*! Look for a certain sector in the cache, and return a pointer to
  the cache entry if found. Return NULL otherwise. */
struct cache_entry* cache_lookup (block_sector_t sector) {
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

/*! Load the given sector into a free cache space. */
struct cache_entry* cache_load (block_sector_t sector) {
    struct cache_entry* ent;
    ent = cache_get_empty();
    ent->status = ALIVE;
    ent->sector = sector;
    block_read(fs_block, sector, ent->data);
    return ent;
}

/*! Returns an empty cache entry, evicting if necessary. */
struct cache_entry* cache_get_empty (void) {
    int i;
    struct cache_entry* ent;
    for (i=0; i<CACHE_SIZE; i++) {
        ent = &cache_info[i];
        if (ent->status == EMPTY)
            return ent;
    }

    ent = cache_choose_evictee();
    cache_evict(ent);
    return ent;
}

/*! Selects a cache entry to be evicted. */
struct cache_entry* cache_choose_evictee (void) {
    // TODO(keegan): Better eviction algorithm.
    static int ind = 0;
    struct cache_entry* ent = &cache_info[ind];
    ind = (ind + 1) % CACHE_SIZE;
    return ent;
}

/*! Evicts a cache entry to disk. */
void cache_evict (struct cache_entry* ent) {
    ASSERT(ent != NULL);
    if (ent->status == DIRTY) {
        block_write(fs_block, ent->sector, ent->data);
    }
    ent->status = EMPTY;
}

void cache_lock(void) {
    lock_acquire(&cache_lock_);
}
void cache_unlock(void) {
    lock_release(&cache_lock_);
}
