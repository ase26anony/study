/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of artificial helper/resolver function */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               used, 
               visibility("hidden"),
               noclone))
static int multi_version_func(int x, int y) {
    /* Complex enough to not be optimized away, but simple for demonstration */
    volatile int result = 0;
    
    /* Use operations that might benefit from different architectures */
    for (int i = 0; i < x; i++) {
        result += y * i;
        result ^= (y << 3);
    }
    
    /* Prevent constant folding */
    asm volatile("" : "+r" (result));
    
    return result;
}

/* Another approach: Use __builtin_cpu_supports to force runtime dispatch */
__attribute__((target("default"), noinline, used, visibility("hidden")))
static int default_version(int x) {
    int sum = 0;
    for (int i = 0; i < x; i++) {
        sum += i * i;
    }
    return sum;
}

__attribute__((target("avx2"), noinline, used, visibility("hidden")))
static int avx2_version(int x) {
    int sum = 0;
    for (int i = 0; i < x; i++) {
        sum += i * i * 2;
    }
    return sum;
}

/* Force compiler to generate internal dispatch logic */
int get_optimized_value(int x) {
    if (__builtin_cpu_supports("avx512f")) {
        return multi_version_func(x, x * 2);
    } else if (__builtin_cpu_supports("avx2")) {
        return avx2_version(x);
    } else {
        return default_version(x);
    }
}
