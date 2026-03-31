/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of multi-versioned function with internal resolver */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               used, 
               visibility("hidden")))
int compute_value(int x, int y) {
    /* Use operations that benefit from vectorization */
    int result = 0;
    for (int i = 0; i < 32; i++) {
        result += x * y + i;
    }
    return result;
}

/* Another approach: Use atomic builtin that may need helper function */
__attribute__((target("arch=x86-64-v3"), noinline, used))
long atomic_helper(void* ptr) {
    long val = 0;
    /* This may generate internal helper for atomic operations */
    __atomic_load(ptr, &val, __ATOMIC_SEQ_CST);
    return val;
}
