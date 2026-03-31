/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Volatile results to prevent optimization */
volatile int cpu_feature_flags = 0;
volatile int cache_sensitive_result = 0;

/* Function pointers to prevent constant folding */
typedef int (*compute_func_t)(int, int);
volatile compute_func_t compute_func = NULL;

/* Force CPU initialization early */
__attribute__((constructor)) void init_cpu(void) {
    __builtin_cpu_init();
}

/* Different computation patterns based on CPU features */
int compute_sse2(int stride, int limit) {
    int sum = 0;
    for (int i = 0; i < limit; i += stride) {
        array1[i] = array2[i] + array3[i % 8192];
        sum += array1[i];
    }
    return sum;
}

int compute_avx(int stride, int limit) {
    int sum = 0;
    /* More complex access pattern */
    for (int i = 0; i < limit; i += stride) {
        int idx = (i * 7) % (1024 * 1024);
        array1[idx] = array2[idx] * 2 - array3[idx % 8192];
        sum += array1[idx];
    }
    return sum;
}

int compute_avx512(int stride, int limit) {
    int sum = 0;
    /* Even larger strides to test different cache lines */
    for (int i = 0; i < limit; i += stride * 2) {
        int idx1 = i % (1024 * 1024);
        int idx2 = (i + 64) % (1024 * 1024);
        array1[idx1] = array2[idx2] + array3[(idx1 + idx2) % 8192];
        sum += array1[idx1] - array2[idx2];
    }
    return sum;
}

int compute_baseline(int stride, int limit) {
    int sum = 0;
    for (int i = 0; i < limit; i += stride) {
        sum += i;
    }
    return sum;
}

/* Conditional compilation blocks that force driver to evaluate CPUID */
#ifdef __OPTIMIZE__
/* This macro forces the driver to check CPU features during compilation */
#define CHECK_FEATURE_AND_SET(feature, flag) \
    do { \
        if (__builtin_cpu_supports(feature)) { \
            cpu_feature_flags |= (flag); \
        } \
    } while(0)
#else
#define CHECK_FEATURE_AND_SET(feature, flag) \
    do { \
        cpu_feature_flags |= (flag); \
    } while(0)
#endif

/* Architecture-specific conditional blocks */
#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX512F__)
#define HAS_VECTOR_EXTENSIONS 1
#else
#define HAS_VECTOR_EXTENSIONS 0
#endif

int main(void) {
    int result = 0;
    int stride = 1;
    int limit = 100000;
    
    /* Initialize arrays */
    for (int i = 0; i < 1024 * 1024; i++) {
        array1[i] = i % 256;
        array2[i] = (i * 3) % 256;
    }
    for (int i = 0; i < 512 * 512; i++) {
        array3[i] = (i * 5) % 256;
    }
    
    /* Force driver to evaluate CPUID for various features */
    CHECK_FEATURE_AND_SET("sse2", 0x01);
    CHECK_FEATURE_AND_SET("sse4.2", 0x02);
    CHECK_FEATURE_AND_SET("avx", 0x04);
    CHECK_FEATURE_AND_SET("avx2", 0x08);
    CHECK_FEATURE_AND_SET("avx512f", 0x10);
    CHECK_FEATURE_AND_SET("bmi2", 0x20);
    
    /* Select computation based on detected features */
    if (cpu_feature_flags & 0x10) { /* AVX512 */
        compute_func = compute_avx512;
        stride = 16;  /* Larger stride for wider vectors */
    } else if (cpu_feature_flags & 0x04) { /* AVX */
        compute_func = compute_avx;
        stride = 8;
    } else if (cpu_feature_flags & 0x01) { /* SSE2 */
        compute_func = compute_sse2;
        stride = 4;
    } else {
        compute_func = compute_baseline;
        stride = 2;
    }
    
    /* Perform cache-sensitive computation */
    if (compute_func) {
        result = compute_func(stride, limit);
    }
    
    /* Additional architecture-specific paths */
#if HAS_VECTOR_EXTENSIONS
    /* This code only compiled if vector extensions are enabled,
       forcing driver to consider cache for vectorized code */
    volatile int vector_sum = 0;
    for (int i = 0; i < 65536; i += 64) {  /* Cache line sized steps */
        vector_sum += array1[i] * array2[i];
    }
    result += vector_sum;
#endif
    
    /* Mix in results based on specific microarchitecture features */
#ifdef __tune_core2__
    /* Core2-specific optimization hint */
    for (int i = 0; i < 32768; i += 128) {
        result ^= array1[i];
    }
#endif
    
#ifdef __tune_nehalem__
    /* Nehalem-specific pattern */
    for (int i = 0; i < 65536; i += 64) {
        result += array2[i] - array1[i];
    }
#endif
    
#ifdef __tune_sandybridge__
    /* Sandy Bridge pattern */
    for (int i = 0; i < 131072; i += 32) {
        result *= (array1[i] + 1);
    }
#endif
    
#ifdef __tune_skylake_avx512__
    /* Skylake AVX512 pattern */
    for (int i = 0; i < 262144; i += 256) {
        result |= array1[i] & array2[i];
    }
#endif
    
    printf("Result: %d (CPU features: 0x%x)\n", result, cpu_feature_flags);
    
    /* Prevent dead code elimination */
    cache_sensitive_result = result;
    
    return result != 0 ? 0 : 1;
}

/* Additional function to ensure CPUID is used in multiple contexts */
__attribute__((noinline)) int check_cache_sensitive(void) {
    static int counter = 0;
    
    /* Use CPU features in a way that might affect cache behavior */
    if (__builtin_cpu_supports("sse2")) {
        counter += 1;
    }
    if (__builtin_cpu_supports("avx")) {
        counter += 2;
    }
    if (__builtin_cpu_supports("avx512f")) {
        counter += 4;
    }
    
    /* Cache-unfriendly access pattern */
    int sum = 0;
    for (int i = 0; i < 1000; i++) {
        int idx = (i * 17) % (1024 * 1024);
        sum += array1[idx];
    }
    
    return sum + counter;
}
