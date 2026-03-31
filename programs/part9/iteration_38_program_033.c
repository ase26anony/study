/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of a multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use architecture-specific intrinsics to ensure different codegen */
    int result = x * y;
    
    /* Add some computation that can't be optimized away */
    for (int i = 0; i < 100; i++) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result;
}

/* Another approach: Use atomic operations that may need helper functions */
__attribute__((target("arch=core-avx2"), noinline, used))
static long atomic_helper(void) {
    long val = 0;
    long desired = 1;
    long expected = 0;
    
    /* This may generate internal helper functions for atomic operations */
    __atomic_compare_exchange(&val, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return val;
}

/* Function that will be called from main */
int get_computation(int x, int y) {
    /* Mix both approaches to increase chances of triggering the hook */
    int a = multi_version_func(x, y);
    long b = atomic_helper();
    return a + (int)b;
}
