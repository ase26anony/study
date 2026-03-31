/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of artificial helper/resolver function */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               used, 
               visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Complex enough to prevent constant folding */
    volatile int result = 0;
    
    /* Use operations that might benefit from different ISA extensions */
    for (int i = 0; i < x; i++) {
        result += y * i;
        result ^= (result << 3);
        result ^= (result >> 5);
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r" (result));
    
    return result;
}

/* Another approach: explicit target attribute with ifunc-like behavior */
#ifdef __cplusplus
extern "C" {
#endif

/* This should trigger creation of artificial clone */
__attribute__((target("avx512f"), noinline, used))
int avx512_version(int x) {
    volatile int sum = 0;
    for (int i = 0; i < x; i++) {
        sum += i * i;
    }
    asm volatile("" : "+r" (sum));
    return sum;
}

/* Force generation of atomic helper function */
__attribute__((noinline))
long long atomic_helper(void) {
    long long var = 0;
    long long expected = 0;
    long long desired = 1;
    
    /* This may generate internal atomic helper function */
    __atomic_compare_exchange(&var, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return var;
}

#ifdef __cplusplus
}
#endif

/* Entry point for target_func.c */
int get_multi_version_result(int x, int y) {
    return multi_version_func(x, y) + avx512_version(x) + atomic_helper();
}
