/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of artificial helper functions through multi-versioning */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               used, 
               visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Complex enough to prevent constant folding, but simple for demonstration */
    volatile int result = 0;
    
    /* Use operations that might benefit from different architectures */
    for (int i = 0; i < x; i++) {
        result += y * i;
        result ^= (result << 3);
        result ^= (result >> 5);
    }
    
    /* Force volatile access to prevent optimization */
    asm volatile("" : "+r" (result) : : "memory");
    
    return result;
}

/* Another approach: explicit target attribute that may create resolver */
__attribute__((target("avx512f"), noinline, used))
int avx512_specific(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Force compiler to consider AVX512 operations */
        asm volatile("" : "+r" (sum) : : "memory");
    }
    return sum;
}

/* Function using atomic operations that may need helper functions */
__attribute__((noinline, used))
long atomic_helper(long *ptr, long val) {
    long old;
    /* __atomic_compare_exchange may generate internal helpers */
    __atomic_compare_exchange(ptr, &old, &val, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return old;
}
