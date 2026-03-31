/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with internal resolver */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use atomic operation that might require helper function */
    int result = 0;
    
    /* Force compiler to generate helper by using __atomic builtin */
    __atomic_store_n(&result, x + y, __ATOMIC_SEQ_CST);
    
    /* Use CPU feature detection that might trigger internal helpers */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    /* Prevent constant folding */
    asm volatile("" : "+r" (result));
    
    return result;
}

/* Function with explicit target attribute that might need resolver */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
extern int avx512_func(int x);

/* Definition with static linkage but external declaration */
static int avx512_func(int x) {
    int result = x * 2;
    
    /* Use vector intrinsics that might require builtin expansion */
    __attribute__((vector_size(16))) int v = {x, x, x, x};
    result += v[0] + v[1];
    
    /* Memory barrier that might generate helper */
    __sync_synchronize();
    
    return result;
}

/* Export the multi-versioned function */
int get_multi_version_result(int x, int y) {
    return multi_version_func(x, y) + avx512_func(x);
}
