/*
 * safe_mem.c — byte-at-a-time comparison routines, linked ahead of IRIX libc.
 *
 * WHY THIS EXISTS. IRIX's libc comparison routines are hand-written MIPS
 * assembly that load a word (or doubleword) at a time. That is faster, and it
 * is safe on IRIX's own allocator, because reading past the end of an object
 * stays inside a page the object already touches. It is *not* safe when the
 * object was placed by a different allocator: dlmalloc hands out mmap'd
 * regions, so an object can end exactly at a page boundary with nothing mapped
 * after it, and the tail read of an aligned word load faults. The symptom is a
 * SIGSEGV inside memcmp/strcmp with both pointers looking perfectly valid.
 *
 * These read one byte at a time and therefore never touch a byte the caller
 * did not ask about. irix-ld puts this object on the link line ahead of -lc,
 * so these definitions win over libc's.
 *
 * FREESTANDING ON PURPOSE. It declares its own size_t and includes nothing.
 * scripts/build-runtime-objects.sh therefore builds it with raw clang rather
 * than irix-cc, whose force-included sysroot headers would collide with that
 * typedef. Keep both properties together: adding an #include here breaks the
 * build, and switching the build to irix-cc breaks the typedef.
 *
 * -fno-builtin is passed as insurance rather than as a fix for something
 * observed. A compiler is allowed to recognise the loop in memcmp() as memcmp()
 * and rewrite it into a call to itself, which links and then recurses until the
 * stack is gone. Checked on clang 18.1.3 at -O2: it does not do it, and the
 * object is byte-identical with and without the flag. The flag costs nothing
 * and the failure it prevents is a stack overflow inside memcmp with no clue.
 *
 * NOT PROVIDED HERE: memcpy/memmove/memset, which do not overread -- they write
 * exactly what was asked for -- and replacing IRIX's tuned versions with byte
 * loops would be a large silent regression on anything that moves buffers.
 * Also not provided: _Hash_bytes, which libstdc++ reads word-at-a-time and
 * which upstream's version of this file covers. It is C++-only surface, so a
 * C-and-Rust toolchain never reaches it; without a definition here a C++
 * program simply gets libstdc++'s own, losing only this protection.
 *
 * ASCII only, C89-compatible declarations, no libc calls.
 */

/* n32: 32-bit pointers, so size_t is unsigned int. Matching the sysroot's
 * definition matters because these symbols override libc's. */
typedef unsigned int safe_size_t;

int memcmp(const void *s1, const void *s2, safe_size_t n);
int bcmp(const void *s1, const void *s2, safe_size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, safe_size_t n);

int
memcmp(const void *s1, const void *s2, safe_size_t n)
{
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;
    safe_size_t i;

    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            /* The standard requires the sign of the difference of the first
             * differing bytes *interpreted as unsigned char*. Widening to int
             * before subtracting is what makes 0xff compare greater than 0x01
             * rather than less. */
            return (int)a[i] - (int)b[i];
        }
    }
    return 0;
}

/* BSD spelling, still referenced by older sources on IRIX. Its contract is
 * only zero / non-zero, but returning memcmp's value is a superset of that. */
int
bcmp(const void *s1, const void *s2, safe_size_t n)
{
    return memcmp(s1, s2, n);
}

int
strcmp(const char *s1, const char *s2)
{
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;

    while (*a != '\0' && *a == *b) {
        a++;
        b++;
    }
    return (int)*a - (int)*b;
}

int
strncmp(const char *s1, const char *s2, safe_size_t n)
{
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;
    safe_size_t i;

    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return (int)a[i] - (int)b[i];
        }
        if (a[i] == '\0') {
            break;      /* both ended here; the rest of n is not examined */
        }
    }
    return 0;
}
