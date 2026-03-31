/* test_cache_detect.c - Force CPUID cache detection through various paths */

#ifndef NOINLINE_ATTR
#define NOINLINE_ATTR __attribute__((noinline))
#endif

/* Function that forces CPUID initialization */
NOINLINE_ATTR int detect_cpu_features(void) {
    int features = 0;
    
    /* These builtins force CPUID calls and cache initialization */
    if (__builtin_cpu_is("intel")) {
        features |= 1;
    }
    if (__builtin_cpu_is("amd")) {
        features |= 2;
    }
    
    /* Test various CPUID feature flags to trigger different detection paths */
    if (__builtin_cpu_supports("sse")) features |= 4;
    if (__builtin_cpu_supports("sse2")) features |= 8;
    if (__builtin_cpu_supports("sse3")) features |= 16;
    if (__builtin_cpu_supports("ssse3")) features |= 32;
    if (__builtin_cpu_supports("sse4.1")) features |= 64;
    if (__builtin_cpu_supports("sse4.2")) features |= 128;
    if (__builtin_cpu_supports("avx")) features |= 256;
    if (__builtin_cpu_supports("avx2")) features |= 512;
    if (__builtin_cpu_supports("avx512f")) features |= 1024;
    
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
    for (int i = 0; i < size * 10; i++) {
        sum += buffer[(i * 7) % size];
    }
    
    return sum;
}

int main(void) {
    int features = detect_cpu_features();
    
    /* Perform computations with different working set sizes */
    long result = 0;
    
    /* Small working set (likely L1) */
    result += cache_sensitive_loop(8192);   /* 8KB */
    
    /* Medium working set (likely L2) */
    result += cache_sensitive_loop(262144); /* 256KB */
    
    /* Larger working set (likely L3 or memory) */
    result += cache_sensitive_loop(2097152); /* 2MB */
    
    /* Prevent optimization */
    __asm__ __volatile__("" : "+r" (result));
    
    return (int)(result % 256);
}
