/* test_cache_detection.c - Exercise CPU cache detection through various compilation scenarios */

#ifndef NOINLINE_ATTR
#define NOINLINE_ATTR __attribute__((noinline))
#endif

/* Function that forces CPUID usage and cache-sensitive computation */
NOINLINE_ATTR static int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins trigger CPUID and cache initialization */
    if (__builtin_cpu_is("intel")) {
        features |= 1;
    }
    if (__builtin_cpu_is("amd")) {
        features |= 2;
    }
    if (__builtin_cpu_supports("sse")) {
        features |= 4;
    }
    if (__builtin_cpu_supports("sse2")) {
        features |= 8;
    }
    if (__builtin_cpu_supports("sse3")) {
        features |= 16;
    }
    if (__builtin_cpu_supports("ssse3")) {
        features |= 32;
    }
    if (__builtin_cpu_supports("sse4.1")) {
        features |= 64;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        features |= 128;
    }
    if (__builtin_cpu_supports("avx")) {
        features |= 256;
    }
    if (__builtin_cpu_supports("avx2")) {
        features |= 512;
    }
    
    return features;
}

/* Cache-sensitive computation - size tuned to trigger different cache behaviors */
NOINLINE_ATTR static long cache_sensitive_loop(int size_kb) {
    volatile long sum = 0;
    int elements = (size_kb * 1024) / sizeof(int);
    static int array[1024 * 1024]; /* 4MB static array */
    
    /* Access pattern that stresses cache */
    for (int i = 0; i < elements; i += 64) {
        sum += array[i % (1024 * 1024)];
    }
    
    return sum;
}

int main(void) {
    int cpu_features = detect_cpu_features();
    long result = 0;
    
    /* Test different loop sizes to potentially trigger different cache optimizations */
    result += cache_sensitive_loop(8);   /* L1 cache size test */
    result += cache_sensitive_loop(64);  /* L2 cache size test */
    result += cache_sensitive_loop(512); /* L3 cache size test */
    
    /* Use the result to prevent optimization */
    return (int)(result + cpu_features) % 256;
}
