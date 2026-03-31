/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of internal resolver/helper functions */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use operations that might need different implementations per target */
    int result = x * y;
    
    /* Use atomic operation that might need helper function */
    __atomic_store_n(&result, x + y, __ATOMIC_RELAXED);
    
    return result;
}

/* Another function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used))
int avx512_specific(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Force generation of exception handling helpers (if compiled as C++) */
#ifdef __cplusplus
__attribute__((noinline, target("default")))
void throw_helper() {
    throw 42;
}
#endif
