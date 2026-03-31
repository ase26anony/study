/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of artificial resolver function with target_clones */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
int compute_with_features(int a, int b) {
    /* Simple computation that can't be constant folded */
    volatile int base = a;
    int result = base + b;
    
    /* Use inline assembly to prevent optimization */
    asm volatile("" : "+r"(result) : : "memory");
    
    /* Atomic operation that might need helper functions */
    int expected = result;
    int desired = result + 1;
    __atomic_compare_exchange(&result, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), 
               noinline,
               used,
               visibility("hidden")))
static int avx512_specific_compute(int x) {
    /* This static function with target attribute and hidden visibility
       might trigger the creation of artificial declarations */
    volatile int v = x;
    asm volatile("" : "+r"(v) : : "memory");
    
    /* Use __builtin_cpu_supports to potentially generate runtime dispatch */
    if (__builtin_cpu_supports("avx512f")) {
        v = v * 2;
    }
    
    return v;
}

/* External declaration to force linkage considerations */
extern int external_helper(void);

/* Function that uses the target-specific function */
__attribute__((noinline))
int public_interface(int x) {
    int result = compute_with_features(x, x + 1);
    result += avx512_specific_compute(result);
    return result;
}
