/* Compile with: gcc -O2 -flto -c target_func.c -fuse-linker-plugin */
#include <stdint.h>

/* This function uses target_clones to force generation of internal resolver */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static uint64_t multi_version_func(uint64_t a, uint64_t b) {
    /* Use operations that benefit from different instruction sets */
    uint64_t result = a;
    for (int i = 0; i < 64; i++) {
        /* Mix of operations to prevent optimization */
        result = (result ^ b) + (result << (i & 7));
        result = result ^ (result >> 13);
    }
    return result;
}

/* Force generation of atomic helper with specific target */
__attribute__((target("avx512f"), noinline, noclone))
static uint64_t atomic_helper(uint64_t *ptr, uint64_t val) {
    /* Use __atomic builtin that may generate internal helpers */
    uint64_t expected = *ptr;
    __atomic_compare_exchange(ptr, &expected, &val, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}

/* External declaration for main to use */
uint64_t get_result(uint64_t a, uint64_t b);

uint64_t get_result(uint64_t a, uint64_t b) {
    uint64_t local = 0;
    uint64_t res1 = multi_version_func(a, b);
    uint64_t res2 = atomic_helper(&local, res1);
    return res1 ^ res2;
}
