/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use architecture-specific intrinsics to prevent optimization */
    int result = x * y;
    
    /* Add volatile assembly to prevent dead code elimination */
    asm volatile ("" : "+r" (result) : : "memory");
    
    /* Use atomic operation that might require helper function */
    if (result > 1000) {
        /* This may trigger generation of internal helpers */
        __atomic_store_n(&result, x + y, __ATOMIC_RELAXED);
    }
    
    return result;
}

/* External declaration for main.c */
extern int get_computation(int, int);

/* Wrapper with external linkage */
int get_computation(int x, int y) {
    return multi_version_func(x, y);
}

/* Another function using explicit target attribute */
__attribute__((target("avx512f"), noinline, used))
static void avx512_helper(void* ptr) {
    /* Complex operation that might need runtime checking */
    volatile int* p = (volatile int*)ptr;
    for (int i = 0; i < 16; i++) {
        p[i] = __builtin_popcount(p[i]);
    }
}

/* Function that uses the helper */
void process_data(void* data, int size) {
    /* Runtime check for CPU features */
    if (__builtin_cpu_supports("avx512f")) {
        avx512_helper(data);
    }
}
