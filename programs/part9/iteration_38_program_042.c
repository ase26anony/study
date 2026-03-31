/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of artificial helper through multi-versioning */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use operations that might require different implementations per target */
    int result = x * y;
    
    /* Add some computation to prevent constant folding */
    for (int i = 0; i < (x & 0x3); i++) {
        result += y;
    }
    
    return result;
}

/* Another approach: Use atomic operations that might generate helpers */
__attribute__((target("avx512f")))
__attribute__((noinline, used, visibility("hidden")))
static long atomic_helper(long *ptr, long val) {
    /* This might generate internal atomic helper functions */
    __atomic_add_fetch(ptr, val, __ATOMIC_SEQ_CST);
    return __atomic_load_n(ptr, __ATOMIC_RELAXED);
}

/* Force declaration with unusual linkage combination */
extern int public_external_static(void);

__attribute__((target("arch=core-avx2")))
__attribute__((noinline, used))
static int public_external_static(void) {
    /* This static function with external declaration might trigger
       the unusual TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1 combination */
    return 42;
}

/* Interface function that will be called from main */
__attribute__((noinline))
int target_operations(int x, int y) {
    int result = multi_version_func(x, y);
    
    long atomic_var = 0;
    result += atomic_helper(&atomic_var, y);
    
    result += public_external_static();
    
    return result;
}
