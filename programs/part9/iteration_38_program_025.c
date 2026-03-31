/* Force generation of artificial helper/resolver functions */
#include <stdint.h>
#include <string.h>

/* Option 1: Multi-versioned function with target clones */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Complex enough to not be optimized away */
    volatile int result = x;
    for (int i = 0; i < y; i++) {
        result ^= (result << 13);
        result ^= (result >> 17);
        result ^= (result << 5);
    }
    return result;
}

/* Option 2: Function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
static int avx512_func(int x) {
    /* Use builtin that might require runtime support */
    if (__builtin_cpu_supports("avx512f")) {
        return x * 3;
    }
    return x * 2;
}

/* Option 3: Atomic operation that might need helper */
__attribute__((noinline, used, visibility("hidden")))
static long atomic_helper(void* ptr) {
    long val = 0;
    /* 16-byte atomic compare-exchange might need libcall */
    __int128_t expected = 0;
    __int128_t desired = 1;
    __atomic_compare_exchange((__int128_t*)ptr, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return (long)expected;
}

/* Public interface function */
int target_function(int x, int y) {
    int a = multi_version_func(x, y);
    int b = avx512_func(x);
    long c = atomic_helper((void*)&x);
    return a + b + (int)c;
}
