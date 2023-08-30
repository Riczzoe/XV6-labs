// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// Physical Address start point(4KB aligned)
#define PA_START PGROUNDUP((uint64)end)
#define PA_PAGE_MAX_COUNT (PHYSTOP - PA_START) / PGSIZE
#define PA_PAGE_INDEX(pa) (((PGROUNDUP((uint64)pa)-(uint64)PA_START) >> 12))
#define REF_ARRAY_END (PA_START + PA_PAGE_MAX_COUNT)
char *PA_REF_COUNT;
struct spinlock refcount_lock;

void
kinit()
{
    initlock(&kmem.lock, "kmem");
    initlock(&refcount_lock, "refcount");
    PA_REF_COUNT = (char *)PA_START;
    freerange((void *)REF_ARRAY_END, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
      PA_REF_COUNT[PA_PAGE_INDEX(p)] = 1;
      kfree(p);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
    struct run *r;

    if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
        panic("kfree");

    acquire(&refcount_lock);

    if (PA_REF_COUNT[PA_PAGE_INDEX(pa)] > 1) {
        PA_REF_COUNT[PA_PAGE_INDEX(pa)]--;
        release(&refcount_lock);
        return;
    }
    if (PA_REF_COUNT[PA_PAGE_INDEX(pa)] < 1) {
        panic("kfree reference count\n");
    }
    release(&refcount_lock);

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
}

void
inc_ref_count(void *pa)
{
    if (pa >= (void*)PHYSTOP || pa < (void *)end) {
        return;
    }

    uint64 index = PA_PAGE_INDEX(pa);

    acquire(&refcount_lock);
    PA_REF_COUNT[index]++;
    release(&refcount_lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
    struct run *r;

    acquire(&kmem.lock);
    r = kmem.freelist;
    if(r)
        kmem.freelist = r->next;
    release(&kmem.lock);

    if(r)  memset((char*)r, 5, PGSIZE); // fill with junk
    return (void*)r;
}
