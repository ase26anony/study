/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force driver to evaluate CPUID during compilation */
#if defined(__OPTIMIZE__) && defined(__i386__)
#define USE_CPUID_DETECTION 1
#elif defined(__OPTIMIZE__) && defined(__x86_64__)
#define USE_CPUID_DETECTION 1
#else
#define USE_CPUID_DETECTION 0
#endif

/* Large arrays to encourage cache consideration */
static volatile int data_array[1024 * 1024];  /* 4MB */
static volatile int temp_array[512 * 512];    /* 1MB */

/* Function pointers to prevent optimization */
typedef void (*ComputeFunc)(int*, int, int);
volatile ComputeFunc func_ptr = NULL;

/* Different computation patterns for different CPU features */
void compute_sse2(int* arr, int size, int stride) {
    for (int i = 0; i < size; i += stride) {
        arr[i] = arr[i] * 3 + 7;
    }
}

void compute_avx(int* arr, int size, int stride) {
    for (int i = 0; i < size; i += stride) {
        arr[i] = (arr[i] << 2) | (arr[i] >> 30);
    }
}

void compute_sse4(int* arr, int size, int stride) {
    for (int i = 0; i < size; i += stride) {
        arr[i] = arr[i] ^ (arr[i] * 2);
    }
}

void compute_default(int* arr, int size, int stride) {
    for (int i = 0; i < size; i += stride) {
        arr[i] = arr[i] + i;
    }
}

int main(void) {
    /* Initialize CPU detection - this triggers driver's __builtin_cpu_init */
    __builtin_cpu_init();
    
    /* Volatile results to prevent constant folding */
    volatile int has_sse2 = 0;
    volatile int has_sse4 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512 = 0;
    
    /* Check various CPU features - each requires driver to evaluate CPUID */
#if USE_CPUID_DETECTION
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse4 = __builtin_cpu_supports("sse4.2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512 = __builtin_cpu_supports("avx512f");
#endif
    
    /* Choose computation based on CPU features */
    ComputeFunc compute = compute_default;
    int stride = 1;
    
    if (has_avx512) {
        compute = compute_avx;
        stride = 16;  /* Larger stride for AVX512 */
    } else if (has_avx2) {
        compute = compute_avx;
        stride = 8;
    } else if (has_avx) {
        compute = compute_avx;
        stride = 4;
    } else if (has_sse4) {
        compute = compute_sse4;
        stride = 2;
    } else if (has_sse2) {
        compute = compute_sse2;
        stride = 1;
    }
    
    /* Store in volatile pointer to prevent optimization */
    func_ptr = compute;
    
    /* Perform computation on large arrays - cache-sensitive pattern */
    int array_size = sizeof(data_array) / sizeof(data_array[0]);
    int temp_size = sizeof(temp_array) / sizeof(temp_array[0]);
    
    /* Initialize arrays */
    for (int i = 0; i < array_size; i++) {
        data_array[i] = i & 0xFF;
    }
    
    /* Compute checksum using selected function */
    int checksum = 0;
    
    /* Multiple passes with different access patterns */
    for (int pass = 0; pass < 3; pass++) {
        /* Call through function pointer */
        func_ptr((int*)data_array, array_size, stride);
        
        /* Cross-array access pattern */
        for (int i = 0; i < temp_size && i < array_size; i++) {
            temp_array[i] = data_array[i * stride % array_size];
        }
        
        /* Compute simple checksum */
        for (int i = 0; i < temp_size; i += 64) {  /* Cache line sized jumps */
            checksum += temp_array[i];
        }
    }
    
    printf("CPU Feature Checksum: %d\n", checksum);
    printf("Features detected: SSE2=%d, SSE4=%d, AVX=%d, AVX2=%d, AVX512=%d\n",
           (int)has_sse2, (int)has_sse4, (int)has_avx, (int)has_avx2, (int)has_avx512);
    
    return checksum & 0xFF;
}

/* Additional compilation-only paths to force driver evaluation */
#if defined(__SSE2__) && USE_CPUID_DETECTION
/* This section only compiled if SSE2 is available */
static volatile int __attribute__((used)) sse2_marker = 
    __builtin_cpu_supports("sse2") ? 1 : 0;
#endif

#if defined(__AVX__) && USE_CPUID_DETECTION
static volatile int __attribute__((used)) avx_marker = 
    __builtin_cpu_supports("avx") ? 2 : 0;
#endif

#if defined(__AVX2__) && USE_CPUID_DETECTION
static volatile int __attribute__((used)) avx2_marker = 
    __builtin_cpu_supports("avx2") ? 3 : 0;
#endif
