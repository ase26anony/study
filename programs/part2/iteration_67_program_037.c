/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
#define FORCE_CPUID_EVAL(x) __builtin_cpu_supports(x)
#else
#define FORCE_CPUID_EVAL(x) 1
#endif

/* Large arrays to encourage cache consideration */
static volatile int large_array[1024 * 1024];  /* 4MB */
static volatile int medium_array[256 * 256];
static volatile int small_array[64 * 64];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static compute_func_t func_ptr = NULL;

/* Different computation patterns for different CPU features */
static int compute_sse(int stride, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += stride) {
        sum += large_array[i & 0xFFFFF] * 3;
    }
    return sum;
}

static int compute_avx(int stride, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += stride) {
        sum += medium_array[i & 0x3FFF] * 7;
    }
    return sum;
}

static int compute_avx512(int stride, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += stride) {
        sum += small_array[i & 0xFFF] * 11;
    }
    return sum;
}

static int compute_generic(int stride, int size) {
    int sum = 0;
    for (int i = 0; i < size; i += stride) {
        sum += i * 5;
    }
    return sum;
}

/* Initialize CPU features - forces driver to run CPUID */
static void init_cpu_features(void) {
    /* This triggers __builtin_cpu_init in GCC driver */
    __builtin_cpu_init();
    
    /* These conditionals force driver to evaluate CPUID features */
    volatile int has_sse2 = FORCE_CPUID_EVAL("sse2");
    volatile int has_sse3 = FORCE_CPUID_EVAL("sse3");
    volatile int has_ssse3 = FORCE_CPUID_EVAL("ssse3");
    volatile int has_sse4_1 = FORCE_CPUID_EVAL("sse4.1");
    volatile int has_sse4_2 = FORCE_CPUID_EVAL("sse4.2");
    volatile int has_avx = FORCE_CPUID_EVAL("avx");
    volatile int has_avx2 = FORCE_CPUID_EVAL("avx2");
    volatile int has_avx512f = FORCE_CPUID_EVAL("avx512f");
    
    /* Cache-specific features that might trigger different cache detection */
    volatile int has_lzcnt = FORCE_CPUID_EVAL("lzcnt");
    volatile int has_popcnt = FORCE_CPUID_EVAL("popcnt");
    volatile int has_bmi = FORCE_CPUID_EVAL("bmi");
    volatile int has_bmi2 = FORCE_CPUID_EVAL("bmi2");
    
    /* Set function pointer based on features */
    if (has_avx512f) {
        func_ptr = compute_avx512;
    } else if (has_avx2) {
        func_ptr = compute_avx;
    } else if (has_sse4_2) {
        func_ptr = compute_sse;
    } else {
        func_ptr = compute_generic;
    }
    
    /* Use volatile results to prevent dead code elimination */
    volatile int feature_sum = has_sse2 + has_sse3 + has_ssse3 + 
                              has_sse4_1 + has_sse4_2 + has_avx + 
                              has_avx2 + has_avx512f;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024 * 1024; i++) {
        large_array[i] = i & 0xFF;
    }
    for (int i = 0; i < 256 * 256; i++) {
        medium_array[i] = (i * 3) & 0xFF;
    }
    for (int i = 0; i < 64 * 64; i++) {
        small_array[i] = (i * 5) & 0xFF;
    }
}

/* Cache-sensitive computation with different strides */
static int cache_sensitive_compute(int base_stride) {
    int result = 0;
    
    /* Multiple loops with different access patterns */
    for (int stride = base_stride; stride <= 32; stride *= 2) {
        /* Access pattern 1: Sequential with stride */
        for (int i = 0; i < 1000000; i += stride) {
            result += large_array[i % (1024 * 1024)];
        }
        
        /* Access pattern 2: Reverse with stride */
        for (int i = 999999; i >= 0; i -= stride) {
            result += medium_array[i % (256 * 256)];
        }
        
        /* Access pattern 3: Random-like with prime stride */
        for (int i = 0; i < 100000; i++) {
            int idx = (i * 997) % (64 * 64);
            result += small_array[idx];
        }
    }
    
    return result;
}

int main(void) {
    /* Initialize CPU detection - triggers driver's CPUID logic */
    init_cpu_features();
    
    if (!func_ptr) {
        fprintf(stderr, "Error: No compute function selected\n");
        return 1;
    }
    
    /* Perform computations with different cache characteristics */
    int checksum = 0;
    
    /* Test different strides to exercise different cache behaviors */
    checksum += func_ptr(1, 1000000);    /* Unit stride - good spatial locality */
    checksum += func_ptr(2, 1000000);    /* Small stride */
    checksum += func_ptr(4, 1000000);    /* Cache line sized stride */
    checksum += func_ptr(8, 1000000);    /* Multiple cache lines */
    checksum += func_ptr(16, 1000000);   /* Larger stride */
    checksum += func_ptr(32, 1000000);   /* Even larger stride */
    
    /* Additional cache-sensitive computation */
    checksum += cache_sensitive_compute(1);
    checksum += cache_sensitive_compute(2);
    
    /* Mix in some conditional compilation based on CPU features */
#ifdef __SSE2__
    if (FORCE_CPUID_EVAL("sse2")) {
        checksum += 0x1234;
    }
#endif
    
#ifdef __AVX__
    if (FORCE_CPUID_EVAL("avx")) {
        checksum += 0x5678;
    }
#endif
    
#ifdef __AVX512F__
    if (FORCE_CPUID_EVAL("avx512f")) {
        checksum += 0x9ABC;
    }
#endif
    
    printf("CPU Feature Checksum: %d (0x%08x)\n", checksum, checksum);
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = checksum;
    return final_result % 256;
}

/* Additional functions to ensure various code paths exist */
#ifdef __OPTIMIZE__
/* These functions reference CPU features directly */
static int check_cache_features(void) {
    return FORCE_CPUID_EVAL("sse") | 
           FORCE_CPUID_EVAL("sse2") |
           FORCE_CPUID_EVAL("avx") |
           FORCE_CPUID_EVAL("avx2");
}
#endif
