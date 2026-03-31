/* target_func.c - Contains function with target attributes to trigger compiler-generated helpers */

/* Force generation of hidden, artificial declarations through multi-versioning */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
__attribute__((noinline, used, visibility("hidden")))
int multi_version_func(int x, int y) {
    /* Use operations that might be optimized differently per target */
    int result = x * y;
    
    /* Use atomic operation that might require helper function */
    __atomic_store_n(&result, x + y, __ATOMIC_RELAXED);
    
    /* Prevent constant folding */
    asm volatile("" : "+r" (result));
    
    return result;
}

/* Another approach: Function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
static int avx512_specific(int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Memory barrier to prevent optimization */
        __sync_synchronize();
    }
    return sum;
}

/* Force generation of exception handling helpers (C++ would be better) */
__attribute__((noinline, used))
void* atomic_helper(void* ptr, void* desired) {
    /* Complex atomic operation that might need compiler helper */
    void* expected = ptr;
    __atomic_compare_exchange(&ptr, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return ptr;
}

/* OpenMP target region to trigger data mapping helpers */
#ifdef _OPENMP
#pragma omp declare target
__attribute__((noinline, used))
int device_func(int x) {
    return x * 2;
}
#pragma omp end declare target
#endif
