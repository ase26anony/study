/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
#include <stddef.h>

/* Force generation of multi-versioned function with internal resolver */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, used, visibility("hidden")))
static size_t multi_version_func(const char* data, size_t len) {
    size_t hash = 0;
    
    /* Simple computation that can't be optimized away */
    for (size_t i = 0; i < len; i++) {
        hash = hash * 31 + data[i];
    }
    
    /* Use target-specific intrinsics if available */
    #ifdef __AVX512F__
    /* This branch will only be taken for AVX512 version */
    hash |= 0x8000000000000000ULL;
    #elif defined(__AVX2__)
    /* This branch for AVX2 version */
    hash |= 0x4000000000000000ULL;
    #endif
    
    return hash;
}

/* Force generation of atomic helper */
__attribute__((noinline, used))
size_t atomic_helper(void* ptr, size_t old_val, size_t new_val) {
    /* Use __atomic builtin that may require helper function */
    __atomic_compare_exchange((size_t*)ptr, &old_val, &new_val, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return old_val;
}

/* Function with explicit target attribute that needs clone */
__attribute__((target("avx512f"), noinline, used))
static size_t avx512_specific(const char* data, size_t len) {
    size_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum * 2;
}

/* External declaration to force external linkage */
extern size_t multi_version_func(const char*, size_t);
