/* test_cache_detection.c - Force CPUID cache detection through various paths */

#ifndef NOINLINE_ATTR
#define NOINLINE_ATTR __attribute__((noinline))
#endif

/* Function that forces CPUID usage */
NOINLINE_ATTR int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins force CPUID execution */
    if (__builtin_cpu_supports("sse")) features |= 1;
    if (__builtin_cpu_supports("sse2")) features |= 2;
    if (__builtin_cpu_supports("sse3")) features |= 4;
    if (__builtin_cpu_supports("ssse3")) features |= 8;
    if (__builtin_cpu_supports("sse4.1")) features |= 16;
    if (__builtin_cpu_supports("sse4.2")) features |= 32;
    if (__builtin_cpu_supports("avx")) features |= 64;
    if (__builtin_cpu_supports("avx2")) features |= 128;
    
    /* Force cache-related detection */
    if (__builtin_cpu_is("intel")) features |= 256;
    if (__builtin_cpu_is("amd")) features |= 512;
    if (__builtin_cpu_is("core2")) features |= 1024;
    if (__builtin_cpu_is("nehalem")) features |= 2048;
    if (__builtin_cpu_is("sandybridge")) features |= 4096;
    if (__builtin_cpu_is("haswell")) features |= 8192;
    if (__builtin_cpu_is("skylake")) features |= 16384;
    
    return features;
}

/* Cache-sensitive computation */
NOINLINE_ATTR long cache_sensitive_loop(int size) {
    volatile long sum = 0;
    char *buffer = __builtin_alloca(size);
    
    /* Initialize buffer */
    for (int i = 0; i < size; i++) {
        buffer[i] = (char)(i % 256);
    }
    
    /* Access pattern that stresses cache */
    for (int iter = 0; iter < 1000; iter++) {
        for (int i = 0; i < size; i += 64) { /* Common cache line size */
            sum += buffer[i];
        }
    }
    
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    
    /* Test different cache sizes to trigger different optimizations */
    long result1 = cache_sensitive_loop(1024);    /* L1 size */
    long result2 = cache_sensitive_loop(8192);    /* Small L1 */
    long result3 = cache_sensitive_loop(32768);   /* Typical L1 */
    long result4 = cache_sensitive_loop(262144);  /* L2 size */
    long result5 = cache_sensitive_loop(1048576); /* L3 size */
    
    /* Use results to prevent optimization */
    volatile long total = result1 + result2 + result3 + result4 + result5 + features;
    
    return (int)(total % 256);
}
