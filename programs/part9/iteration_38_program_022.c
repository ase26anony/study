/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with internal helper */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               used, 
               visibility("hidden"),
               noclone))
static int multi_version_func(int x, int y) {
    /* Use atomic operation that may require helper function */
    int result = 0;
    
    /* Force volatile memory access to prevent optimization */
    volatile int* ptr = &result;
    
    /* Use __atomic builtin that might generate internal helpers */
    __atomic_store_n(ptr, x + y, __ATOMIC_RELAXED);
    
    /* Use CPU feature detection that may generate internal calls */
    if (__builtin_cpu_supports("avx2")) {
        *ptr = *ptr * 2;
    }
    
    return *ptr;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), 
               noinline,
               used,
               visibility("hidden")))
int avx512_specific(int* arr, int n) {
    /* Complex enough to not be optimized away */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Memory barrier to prevent reordering */
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
    }
    return sum;
}

/* Force generation of exception handling helpers (if compiled as C++) */
#ifdef __cplusplus
__attribute__((noinline, target("default")))
void throw_helper() {
    throw 42;
}

__attribute__((target("avx2"), noinline))
int exception_test() {
    try {
        throw_helper();
    } catch (int e) {
        return e;
    }
    return 0;
}
#endif
