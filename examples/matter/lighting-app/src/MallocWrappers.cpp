/*
 * MallocWrappers.cpp — overflow-detecting malloc shim.
 *
 * Layout of each allocation (raw = sys_heap_alloc() return value):
 *
 *  raw + 0   : AllocHeader { magic, alloc_id, size, _pad }  (16 B)
 *  raw + 16  : user data (requested 'size' bytes)
 *  raw + 16 + size  : canary word (4 B)
 *
 * On free:
 *  1. Check AllocHeader magic — rejects non-heap pointers (bad/double-free).
 *  2. Check canary — catches write overflows past the end of the allocation.
 *  3. Call sys_heap_free.
 *
 * NOTE: We do NOT pre-validate the sys_heap chunk header before calling
 * sys_heap_free.  The chunk header encoding changed in Zephyr 4.4 and any
 * attempt to decode it here produces false positives that skip legitimate
 * frees, causing a memory leak and OOM.  CONFIG_SYS_HEAP_VALIDATE=y in
 * prj.conf enables Zephyr's own heap structural validation.
 *
 * APP_MALLOC_HEAP_SIZE = 12 KB: sufficient for Server::Init (~8-10 KB peak).
 * alloc_id is a monotonic counter so crash messages can be correlated
 * with "[HEAP-ALLOC] #N" trace lines enabled by chip_heap_enable_trace().
 */

#include <zephyr/sys/sys_heap.h>
#include <zephyr/sys/printk.h>
#include <zephyr/spinlock.h>
#include <zephyr/init.h>
#include <cstddef>
#include <cstring>

/* ---------- Heap buffer ---------- */
#define APP_MALLOC_HEAP_SIZE  12288

static uint8_t s_heap_buf[APP_MALLOC_HEAP_SIZE] __aligned(8);
static struct sys_heap s_heap;
static struct k_spinlock s_lock;

static int s_heap_init(void)
{
    sys_heap_init(&s_heap, s_heap_buf, sizeof(s_heap_buf));
    return 0;
}
SYS_INIT(s_heap_init, PRE_KERNEL_1, 1);

/* ---------- Canary / header layout ---------- */
struct AllocHeader {
    uint32_t magic;     /* MAGIC_ALLOC or MAGIC_FREE */
    uint32_t alloc_id;  /* monotonic counter         */
    uint32_t size;      /* original requested bytes  */
    uint32_t _pad;      /* keeps struct size 16 B    */
};
static_assert(sizeof(AllocHeader) == 16, "AllocHeader must be 16 bytes");

static constexpr uint32_t MAGIC_ALLOC = 0xA110CA7Eu;
static constexpr uint32_t MAGIC_FREE  = 0xF4EEF4EEu;
static constexpr uint32_t CANARY_VAL  = 0xC0FFEE01u;
static constexpr size_t   HDR_SZ      = sizeof(AllocHeader); /* 16 B */
static constexpr size_t   CANARY_SZ   = sizeof(uint32_t);    /* 4 B */

/* ---------- Stats ---------- */
static uint32_t s_alloc_id;   /* monotonic allocation counter */
static unsigned s_alloc_cnt;
static unsigned s_free_cnt;
static unsigned s_oom_cnt;
static unsigned s_overflow_cnt;  /* canary tripped  */
static unsigned s_bad_free_cnt;  /* magic wrong     */

#ifdef CONFIG_RT583_HEAP_TRACE
static bool s_trace_enabled;  /* enable per-alloc trace via chip_heap_enable_trace() */
#endif

/* ---------- Helpers ---------- */
static inline AllocHeader * raw_to_hdr(void * raw)
{
    return static_cast<AllocHeader *>(raw);
}
static inline void * hdr_to_user(AllocHeader * hdr)
{
    return static_cast<uint8_t *>(static_cast<void *>(hdr)) + HDR_SZ;
}
static inline AllocHeader * user_to_hdr(void * user_ptr)
{
    return reinterpret_cast<AllocHeader *>(
        static_cast<uint8_t *>(user_ptr) - HDR_SZ);
}
static inline uint32_t * user_to_canary(void * user_ptr, size_t size)
{
    return reinterpret_cast<uint32_t *>(
        static_cast<uint8_t *>(user_ptr) + size);
}

/* ---------- Alloc / free ---------- */
static void * do_alloc(size_t size)
{
    size_t raw_size = HDR_SZ + size + CANARY_SZ;
    k_spinlock_key_t key = k_spin_lock(&s_lock);
    void * raw = sys_heap_alloc(&s_heap, raw_size);
    k_spin_unlock(&s_lock, key);

    if (!raw) {
        s_oom_cnt++;
        return nullptr;
    }

    uint32_t id = ++s_alloc_id;
    AllocHeader * hdr = raw_to_hdr(raw);
    hdr->magic    = MAGIC_ALLOC;
    hdr->alloc_id = id;
    hdr->size     = static_cast<uint32_t>(size);
    hdr->_pad     = 0;

    void * user_ptr = hdr_to_user(hdr);
    *user_to_canary(user_ptr, size) = CANARY_VAL;

    s_alloc_cnt++;
#ifdef CONFIG_RT583_HEAP_TRACE
    if (s_trace_enabled) {
    }
#endif
    return user_ptr;
}

static void do_free(void * user_ptr)
{
    if (!user_ptr) return;

    void * raw = static_cast<uint8_t *>(user_ptr) - HDR_SZ;

    /* 1. Check our wrapper magic.
     *    MAGIC_ALLOC: normal free path.
     *    MAGIC_FREE:  double-free — already freed by us.
     *    other:       not our allocation (static buffer, different allocator, etc.). */
    AllocHeader * hdr = raw_to_hdr(raw);
    if (hdr->magic == MAGIC_FREE) {
        s_bad_free_cnt++;
        return;
    }
    if (hdr->magic != MAGIC_ALLOC) {
        s_bad_free_cnt++;
        /* Capture LR (return address) to identify caller */
        void * lr;
        __asm__ volatile("mov %0, lr" : "=r"(lr));
        return;
    }

    /* 2. Check canary. */
    uint32_t size     = hdr->size;
    uint32_t canary   = *user_to_canary(user_ptr, size);
    if (canary != CANARY_VAL) {
        s_overflow_cnt++;
    }

    hdr->magic = MAGIC_FREE;
#ifdef CONFIG_RT583_HEAP_TRACE
    if (s_trace_enabled) {
    }
#endif
    k_spinlock_key_t key = k_spin_lock(&s_lock);
    sys_heap_free(&s_heap, raw);
    k_spin_unlock(&s_lock, key);
    s_free_cnt++;
}

/* ---------- __wrap_* exports ---------- */
extern "C" {

void * __wrap_malloc(size_t size)   { return do_alloc(size); }

void * __wrap_calloc(size_t n, size_t size)
{
    size_t total = n * size;
    void * p = do_alloc(total);
    if (p) memset(p, 0, total);
    return p;
}

static void * do_realloc(void * ptr, size_t new_size)
{
    if (!ptr)      return do_alloc(new_size);
    if (!new_size) { do_free(ptr); return nullptr; }

    AllocHeader * hdr = user_to_hdr(ptr);
    if (hdr->magic != MAGIC_ALLOC) {
        return nullptr;
    }
    void * np = do_alloc(new_size);
    if (np) {
        size_t copy = (new_size < hdr->size) ? new_size : hdr->size;
        memcpy(np, ptr, copy);
        do_free(ptr);
    }
    return np;
}

void * __wrap_realloc(void * ptr, size_t new_size) { return do_realloc(ptr, new_size); }

void __wrap_free(void * p) { do_free(p); }

/* ── newlib reentrant variants ────────────────────────────────────────────────
 * strdup(), printf internals, and other newlib functions call _malloc_r /
 * _free_r directly, bypassing the public malloc symbol.  Without these wraps
 * they fall through to sbrk → k_heap (2 KB, shared with BT GATT), which
 * exhausts immediately and returns NULL → CHIP_ERROR_NO_MEMORY in Server::Init.
 * The _reent* parameter carries newlib per-thread state; we ignore it since
 * our heap is global and thread-safe via s_lock. */
void * __wrap__malloc_r(void * /*reent*/, size_t size)            { return do_alloc(size); }
void   __wrap__free_r(void * /*reent*/, void * p)                 { do_free(p); }
void * __wrap__calloc_r(void * /*reent*/, size_t n, size_t size)
{
    size_t total = n * size;
    void * p = do_alloc(total);
    if (p) memset(p, 0, total);
    return p;
}
void * __wrap__realloc_r(void * /*reent*/, void * ptr, size_t sz) { return do_realloc(ptr, sz); }

/* Enable per-alloc/free trace output (very verbose — use only around suspect area). */
#ifdef CONFIG_RT583_HEAP_TRACE
void chip_heap_enable_trace(bool on) { s_trace_enabled = on; }
#else
void chip_heap_enable_trace(bool /* on */) { /* compiled out */ }
#endif

/* Snapshot of heap stats — call from AppTask for diagnostics. */
void chip_heap_print_stats(void)
{
}

} /* extern "C" */
