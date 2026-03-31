/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               used, 
               visibility("hidden"),
               noipa))
static int multi_version_func(int x, int y) {
    /* Use operations that might benefit from different ISA extensions */
    int result = x * y;
    
    /* Add some computation to prevent constant folding */
    for (int i = 0; i < (x & 0xF); i++) {
        result += (y >> i);
    }
    
    /* Use __atomic builtin which might generate helper functions */
    int atomic_val = 0;
    __atomic_compare_exchange_n(&atomic_val, &x, y, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result + atomic_val;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("arch=core-avx2"), 
               noinline,
               used,
               visibility("hidden")))
int avx2_specific_func(float* a, float* b, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return (int)sum;
}

/* Function that uses CPU dispatch */
__attribute__((target_clones("default", "sse4.2", "avx", "avx512f")))
int cpu_dispatch_func(int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    /* Use __builtin_cpu_supports which may generate internal helpers */
    if (__builtin_cpu_supports("avx")) {
        sum += 1;
    }
    
    return sum;
}

/* External declaration for main.c */
int get_multi_version_result(int x, int y);
