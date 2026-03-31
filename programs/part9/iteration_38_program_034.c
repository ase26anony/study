/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with internal resolver */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use operations that might benefit from different ISA extensions */
    int result = x * y;
    
    /* Add some computation that prevents constant folding */
    for (int i = 0; i < 10; i++) {
        result += (x ^ y) >> i;
    }
    
    /* Use __atomic builtin which might require helper functions */
    int atomic_var = 0;
    __atomic_compare_exchange_n(&atomic_var, &x, y, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result + atomic_var;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), 
               noinline,
               noclone,
               used,
               visibility("hidden")))
extern int avx512_func(int x) {
    /* Use __builtin_cpu_supports to potentially trigger helper generation */
    if (__builtin_cpu_supports("avx512f")) {
        return x * x + 42;
    }
    return x + 42;
}

/* Function that will be called from main */
int get_computation(int x, int y) {
    /* Call both to ensure they're not optimized away */
    return multi_version_func(x, y) + avx512_func(x);
}
