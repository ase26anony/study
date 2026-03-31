/* test_cache_detection.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#if defined(__OPTIMIZE__)
#define USE_CPU_BUILTINS 1
#else
#define USE_CPU_BUILTINS 0
#endif

/* Large static arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static compute_func_t func_ptr = NULL;

/* Different computation strategies based on CPU features */
static int compute_simple(int a, int b) {
    return a + b;
}

static int compute_sse_style(int a, int b) {
    /* Simulate SSE-like operations */
    return (a & 0xFFFF) + (b & 0xFFFF);
}

static int compute_avx_style(int a, int b) {
    /* Simulate wider operations */
    return (a * 3 + b * 2) / 5;
}

/* Cache-sensitive traversal patterns */
static uint64_t traverse_array_linear(volatile int* arr, int size, int stride) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i += stride) {
        sum += arr[i];
        arr[i] = (int)(sum & 0x7FFFFFFF);
    }
    return sum;
}

static uint64_t traverse_array_random(volatile int* arr, int size, int* indices) {
    uint64_t sum = 0;
    for (int i = 0; i < size; i++) {
        int idx = indices[i] % size;
        sum += arr[idx];
        arr[idx] ^= (int)sum;
    }
    return sum;
}

int main(void) {
    /* Initialize CPU detection - this triggers driver's CPUID logic */
    __builtin_cpu_init();
    
    /* Store CPU feature checks in volatile to prevent constant folding */
    volatile int has_sse2 = 0;
    volatile int has_sse3 = 0;
    volatile int has_ssse3 = 0;
    volatile int has_sse4_1 = 0;
    volatile int has_sse4_2 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512f = 0;
    
    /* These builtin calls force driver to execute CPUID cache detection */
#if USE_CPU_BUILTINS
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse3 = __builtin_cpu_supports("sse3");
    has_ssse3 = __builtin_cpu_supports("ssse3");
    has_sse4_1 = __builtin_cpu_supports("sse4.1");
    has_sse4_2 = __builtin_cpu_supports("sse4.2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512f = __builtin_cpu_supports("avx512f");
#endif
    
    /* Choose computation strategy based on CPU features */
    if (has_avx512f) {
        func_ptr = compute_avx_style;
    } else if (has_avx || has_avx2) {
        func_ptr = compute_avx_style;
    } else if (has_sse4_1 || has_sse4_2) {
        func_ptr = compute_sse_style;
    } else {
        func_ptr = compute_simple;
    }
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 1024 * 1024; i++) {
        array1[i] = (i * 1103515245 + 12345) & 0x7FFF;
        if (i < 512 * 512) {
            array3[i] = (i * 1664525 + 1013904223) & 0x7FFF;
        }
    }
    
    /* Create index array for random access pattern */
    int* indices = (int*)malloc(sizeof(int) * 1024 * 1024);
    if (indices) {
        for (int i = 0; i < 1024 * 1024; i++) {
            indices[i] = (i * 1103515245 + 12345) % (1024 * 1024);
        }
    }
    
    uint64_t checksum = 0;
    
    /* Perform cache-sensitive operations with different strides */
    int stride = 1;
    if (has_sse2) stride = 2;
    if (has_sse4_1) stride = 4;
    if (has_avx) stride = 8;
    if (has_avx512f) stride = 16;
    
    /* Linear traversal - sensitive to cache line size */
    checksum += traverse_array_linear(array1, 1024 * 1024, stride);
    
    /* Random traversal - sensitive to cache size and associativity */
    if (indices) {
        checksum += traverse_array_random(array2, 1024 * 1024, indices);
    }
    
    /* Mixed access pattern */
    for (int i = 0; i < 512 * 512; i += 64) {
        for (int j = 0; j < 64; j++) {
            int idx = i + ((j * 13) % 64);
            checksum = checksum * 31 + array3[idx];
            array3[idx] = (int)(checksum & 0x7FFF);
        }
    }
    
    /* Use CPU feature-dependent computation */
    int result = 0;
    for (int i = 0; i < 1000; i++) {
        result = func_ptr(result, (int)(checksum & 0xFF));
        checksum = (checksum >> 8) | ((uint64_t)result << 56);
    }
    
    if (indices) {
        free(indices);
    }
    
    printf("CPU Feature Summary:\n");
    printf("SSE2: %d, SSE3: %d, SSSE3: %d\n", has_sse2, has_sse3, has_ssse3);
    printf("SSE4.1: %d, SSE4.2: %d\n", has_sse4_1, has_sse4_2);
    printf("AVX: %d, AVX2: %d, AVX512F: %d\n", has_avx, has_avx2, has_avx512f);
    printf("Final checksum: 0x%016llX\n", (unsigned long long)checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* Additional compilation conditionals to force driver evaluation */
#ifdef __SSE2__
/* This section only compiled if SSE2 is available */
static void __attribute__((used)) sse2_helper(void) {
    volatile int check = __builtin_cpu_supports("sse2") ? 1 : 0;
    (void)check;
}
#endif

#ifdef __AVX__
/* This section only compiled if AVX is available */
static void __attribute__((used)) avx_helper(void) {
    volatile int check = __builtin_cpu_supports("avx") ? 1 : 0;
    (void)check;
}
#endif

#ifdef __AVX512F__
/* This section only compiled if AVX512F is available */
static void __attribute__((used)) avx512f_helper(void) {
    volatile int check = __builtin_cpu_supports("avx512f") ? 1 : 0;
    (void)check;
}
#endif
