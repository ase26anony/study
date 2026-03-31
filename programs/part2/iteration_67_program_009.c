/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
#define USE_CPU_BUILTINS 1
#else
#define USE_CPU_BUILTINS 0
#endif

/* Architecture-specific conditionals to trigger different CPUID paths */
#if defined(__i386__) || defined(__x86_64__)
#define X86_ARCH 1
#else
#define X86_ARCH 0
#endif

/* Large array to encourage cache-aware optimizations */
static volatile int data_array[1024 * 1024];

/* Function pointers to prevent optimization */
typedef void (*compute_func_t)(int*, size_t, int);
static compute_func_t func_ptr = NULL;

/* Different computation patterns for different CPU features */
static void compute_simple(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    for (size_t i = 0; i < size; i += stride) {
        arr[i] = arr[i] * 3 + 7;
        sum += arr[i];
    }
    /* Use sum to prevent dead code elimination */
    if (sum == 0x1234) printf("Impossible\n");
}

static void compute_sse_optimized(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    /* Non-linear access pattern to stress cache */
    for (size_t i = 0; i < size; i += stride) {
        size_t idx = (i * 17) % size;  /* Pseudo-random stride */
        arr[idx] = arr[idx] * 5 - 3;
        sum += arr[idx];
    }
    if (sum == 0x5678) printf("Impossible\n");
}

static void compute_avx_optimized(int* arr, size_t size, int stride) {
    volatile int sum = 0;
    /* Larger stride for potential prefetching */
    for (size_t i = 0; i < size; i += stride * 2) {
        arr[i] = arr[i] * 11 + arr[size - i - 1] * 13;
        sum += arr[i];
    }
    if (sum == 0x9ABC) printf("Impossible\n");
}

/* Initialize CPU detection - this triggers driver's __builtin_cpu_init */
static void init_cpu_features(void) {
    /* These builtins force GCC driver to execute CPUID detection */
    __builtin_cpu_init();
    
    /* Store results in volatile to prevent constant folding */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Use volatile results to select computation function */
    if (has_avx512f) {
        func_ptr = compute_avx_optimized;
    } else if (has_avx2) {
        func_ptr = compute_avx_optimized;
    } else if (has_sse4_2) {
        func_ptr = compute_sse_optimized;
    } else if (has_sse2) {
        func_ptr = compute_sse_optimized;
    } else {
        func_ptr = compute_simple;
    }
    
    /* Prevent unused variable warnings */
    (void)has_sse3;
    (void)has_ssse3;
    (void)has_sse4_1;
}

int main(void) {
    size_t array_size = sizeof(data_array) / sizeof(data_array[0]);
    int stride = 1;
    
    /* Initialize CPU features - triggers driver's cache detection */
    init_cpu_features();
    
    /* Choose stride based on "runtime" CPU feature checks */
    volatile int check_sse2 = __builtin_cpu_supports("sse2");
    volatile int check_avx = __builtin_cpu_supports("avx");
    
    if (check_avx) {
        stride = 8;  /* Larger stride for AVX */
    } else if (check_sse2) {
        stride = 4;  /* Medium stride for SSE */
    } else {
        stride = 2;  /* Smaller stride for generic */
    }
    
    /* Execute computation using selected function */
    if (func_ptr) {
        func_ptr((int*)data_array, array_size, stride);
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (size_t i = 0; i < array_size && i < 1000; i += 16) {
        checksum += data_array[i];
    }
    
    printf("CPU feature detection test complete. Checksum: %d\n", checksum);
    
    /* Additional architecture-specific conditionals */
#if USE_CPU_BUILTINS && X86_ARCH
    /* These conditionals force driver to evaluate CPUID during compilation */
    #ifdef __SSE2__
    volatile int sse2_available = __builtin_cpu_supports("sse2");
    #endif
    
    #ifdef __AVX__
    volatile int avx_available = __builtin_cpu_supports("avx");
    #endif
    
    #ifdef __AVX2__
    volatile int avx2_available = __builtin_cpu_supports("avx2");
    #endif
    
    /* Cache size dependent code - encourages driver to query cache topology */
    #if defined(__OPTIMIZE__) && __OPTIMIZE__ > 0
    static char cache_block[32768];  /* 32KB - typical L1 cache size */
    for (int i = 0; i < sizeof(cache_block); i += 64) {  /* 64-byte cache line */
        cache_block[i] = (char)(i % 256);
    }
    #endif
#endif
    
    return 0;
}
