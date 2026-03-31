/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of internal resolver/helper function */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               used, 
               visibility("hidden")))
int compute_with_features(int x, int y) {
    /* Use operations that might require different implementations per target */
    int result = x * y;
    
    /* Use atomic operation that might generate helper functions */
    __atomic_store_n(&result, x + y, __ATOMIC_RELAXED);
    
    /* Prevent constant folding */
    asm volatile("" : "+r" (result));
    
    return result;
}

/* Another function using explicit target attribute to force clone generation */
__attribute__((target("arch=x86-64-v3"), noinline, used, visibility("hidden")))
static int helper_avx2(int x) {
    int r = x * x;
    /* Use builtin that might require runtime check */
    if (__builtin_cpu_supports("avx2")) {
        r += 1;
    }
    return r;
}

/* Function that uses the helper - this may cause the helper to get the flags */
__attribute__((target_clones("default", "avx2")))
int use_helper(int x) {
    return helper_avx2(x) + 1;
}
