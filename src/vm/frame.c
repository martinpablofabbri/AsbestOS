#include "frame.h"

#include <stdio.h>

#include "threads/malloc.h"
#include "threads/palloc.h"
#include "vm/page.h"

static struct list all_frames;


void init_frame_entry (struct frame_entry* entry);
struct frame_entry* frame_create (void);
struct frame_entry* frame_free (void);
struct frame_entry* frame_select_evictee (void);

/*! Initialize the frame subsystem. */
void frame_init (void) {
    list_init(&all_frames);
}

/*! Attempts to acquire an unused frame. On success, returns
  the kernel virtual address of the acquired page. On failure, returns NULL.
 */
struct frame_entry* frame_acquire (void) {
    struct frame_entry* frame;

    /* Try to allocate another page from the user pool. */
    frame = frame_create();

    /* If that fails, try to evict an old page. */
    if (frame == NULL)
        frame = frame_free();

    /* If that fails, give up. */
    if (frame == NULL) {
        printf("Unable to allocate any more pages!\n");
	return NULL;
    }

    return frame;
}

/*! Initialize the variables in a frame_entry struct. */
void init_frame_entry (struct frame_entry* entry) {
    ASSERT(entry != NULL);
    entry->spt = NULL;
}

/*! Attempts to allocate a new page from the user pool.
  If successful, return a pointer to the new frame_entry. */
struct frame_entry* frame_create () {
    void* kpage;
    kpage = palloc_get_page(PAL_USER);

    if (kpage == NULL)
        return NULL;

    struct frame_entry* entry;
    entry = (struct frame_entry*)malloc(sizeof(struct frame_entry));
    // TODO(keegan): free this somewhere
    if (entry) {
	list_push_back(&all_frames, &entry->elem);
	entry->kpage = kpage;
    } else {
	palloc_free_page(kpage);
	kpage = NULL;
    }

    return entry;
}

/*! Attempts to free a page by writing out its contents. If
  successful, return a pointer to the freed frame entry. */
struct frame_entry* frame_free () {
    /* Select a frame to evict. */
    struct frame_entry* evictee = frame_select_evictee();

    /* Evict it. */
    if (page_evict(evictee->spt)) {
        evictee->spt = NULL;
    } else {
        PANIC("Unable to evict page!");
    }

    return evictee;
}

/*! Select the frame containing the page which will be evicted
  next. Returns NULL on failure. */
struct frame_entry* frame_select_evictee () {
    if (list_empty(&all_frames))
        return NULL;

    struct frame_entry* frame = list_entry(list_pop_front(&all_frames),
                                           struct frame_entry,
                                           elem);
    // Push to end of queue
    list_push_back(&all_frames, &frame->elem);
    return frame;
}
