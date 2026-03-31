/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of internal resolver/helper functions */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use architecture-specific intrinsics to ensure different codegen */
    int result = x * y;
    
    /* Add some computation that can't be optimized away */
    for (int i = 0; i < (x % 8); i++) {
        result += y;
    }
    
    return result;
}

/* Another approach: Use __atomic builtin which may generate helper functions */
__attribute__((target("arch=core-avx2")))
__attribute__((noinline, used, visibility("hidden")))
static long atomic_helper(long *ptr, long val) {
    /* This may generate internal atomic helper functions */
    long expected = *ptr;
    __atomic_compare_exchange(ptr, &expected, &val, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}

/* Force declaration with unusual linkage combination */
__attribute__((target("arch=skylake-avx512")))
extern int external_but_static(int x);  /* DECL_EXTERNAL=1 but will be static */

__attribute__((target("arch=skylake-avx512")))
static int external_but_static(int x) {  /* TREE_STATIC=1 */
    volatile int result = x;  /* TREE_THIS_VOLATILE=1 on some internal representation? */
    result += __builtin_cpu_supports("avx512f") ? 100 : 0;
    return result;
}

/* Interface function that will be called from main */
int get_multi_version_result(int x, int y) {
    int r1 = multi_version_func(x, y);
    int r2 = external_but_static(x);
    return r1 + r2;
}
