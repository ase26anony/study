/*
 * Test program to trigger GCC driver's CPUID cache detection logic
 * Compile with various -march= options to exercise different switch cases
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static array to encourage cache-sensitive optimizations */
static volatile int array[1024 * 1024];

/* Volatile variables to prevent optimization of CPU feature checks */
static volatile int cpu_features[16];

/* Function pointers to prevent constant folding */
typedef int (*compute_func_t)(int stride, int iterations);
static compute_func_t compute_func = NULL;

/* Initialize CPU detection - forces driver to execute CPUID */
__attribute__((constructor)) 
static void init_cpu_detection(void) {
    __builtin_cpu_init();
    
    /* Store CPU feature checks in volatile array to prevent optimization */
    cpu_features[0] = __builtin_cpu_supports("sse");
    cpu_features[1] = __builtin_cpu_supports("sse2");
    cpu_features[2] = __builtin_cpu_supports("sse3");
    cpu_features[3] = __builtin_cpu_supports("ssse3");
    cpu_features[4] = __builtin_cpu_supports("sse4.1");
    cpu_features[5] = __builtin_cpu_supports("sse4.2");
    cpu_features[6] = __builtin_cpu_supports("avx");
    cpu_features[7] = __builtin_cpu_supports("avx2");
    cpu_features[8] = __builtin_cpu_supports("avx512f");
    cpu_features[9] = __builtin_cpu_supports("fma");
    cpu_features[10] = __builtin_cpu_supports("aes");
    cpu_features[11] = __builtin_cpu_supports("pclmul");
    
    /* Force evaluation of cache-related features */
#ifdef __OPTIMIZE__
    if (__builtin_cpu_supports("sse2")) {
        compute_func = compute_sse2;
    } else if (__builtin_cpu_supports("sse")) {
        compute_func = compute_sse;
    } else {
        compute_func = compute_generic;
    }
#endif
}

/* Different computation functions for different CPU features */
static int compute_generic(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        sum += array[i & 0xFFFFF]; /* Mask to prevent out-of-bounds */
    }
    return sum;
}

#ifdef __SSE2__
static int compute_sse2(int stride, int iterations) {
    int sum = 0;
    /* Use different access patterns based on stride */
    switch (stride) {
        case 1:  /* Sequential access */
            for (int i = 0; i < iterations; i++) {
                sum += array[i & 0xFFFFF];
            }
            break;
        case 2:  /* Every other element */
            for (int i = 0; i < iterations; i += 2) {
                sum += array[i & 0xFFFFF];
            }
            break;
        case 4:  /* Cache line sized jumps */
            for (int i = 0; i < iterations; i += 4) {
                sum += array[i & 0xFFFFF];
            }
            break;
        case 8:  /* Larger strides */
            for (int i = 0; i < iterations; i += 8) {
                sum += array[i & 0xFFFFF];
            }
            break;
        case 16: /* Page sized jumps */
            for (int i = 0; i < iterations; i += 16) {
                sum += array[i & 0xFFFFF];
            }
            break;
        default:
            sum = compute_generic(stride, iterations);
    }
    return sum;
}
#endif

#ifdef __SSE__
static int compute_sse(int stride, int iterations) {
    int sum = 0;
    /* Simple SSE-friendly computation */
    for (int i = 0; i < iterations; i += stride) {
        sum += array[i & 0xFFFFF] * (i % 256);
    }
    return sum;
}
#endif

/* Conditional compilation blocks that reference CPU builtins */
#if defined(__OPTIMIZE__) && (defined(__i386__) || defined(__x86_64__))
/* This section forces the driver to evaluate CPUID during compilation */
static int select_optimal_stride(void) {
    int stride = 1;
    
    /* These checks force CPUID evaluation in the driver */
    if (__builtin_cpu_supports("avx512f")) {
        stride = 16;  /* Large cache lines */
    } else if (__builtin_cpu_supports("avx2")) {
        stride = 8;
    } else if (__builtin_cpu_supports("avx")) {
        stride = 4;
    } else if (__builtin_cpu_supports("sse4.2")) {
        stride = 2;
    }
    
    return stride;
}
#endif

/* Main function with cache-sensitive operations */
int main(void) {
    int result = 0;
    int optimal_stride = 1;
    
    /* Initialize array with pseudo-random values */
    for (int i = 0; i < 1024 * 1024; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Force CPU initialization if not already done */
    if (!cpu_features[0]) {
        __builtin_cpu_init();
    }
    
#if defined(__OPTIMIZE__) && (defined(__i386__) || defined(__x86_64__))
    /* This triggers driver-side CPUID evaluation */
    optimal_stride = select_optimal_stride();
#endif
    
    /* Perform computations with different strides to exercise cache logic */
    for (int stride = 1; stride <= 16; stride *= 2) {
        int iterations = 1000000;
        
        /* Use function pointer to prevent optimization */
        if (compute_func) {
            result ^= compute_func(stride, iterations);
        } else {
            result ^= compute_generic(stride, iterations);
        }
    }
    
    /* Additional architecture-specific paths */
#ifdef __AVX512F__
    if (__builtin_cpu_supports("avx512f")) {
        /* Force evaluation of AVX512 features */
        result += 0x512;
    }
#endif
    
#ifdef __AVX2__
    if (__builtin_cpu_supports("avx2")) {
        result += 0x256;
    }
#endif
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        result += 0x128;
    }
#endif
    
    printf("Result: %d (CPU features: ", result);
    for (int i = 0; i < 6; i++) {
        printf("%d", cpu_features[i]);
    }
    printf(")\n");
    
    return result & 0xFF;
}

/* Additional functions to prevent dead code elimination */
static void unused_but_present(void) {
    /* Reference various CPU features to ensure they're evaluated */
    volatile int check = 0;
    
    if (__builtin_cpu_supports("mmx")) check |= 1;
    if (__builtin_cpu_supports("3dnow")) check |= 2;
    if (__builtin_cpu_supports("sse4a")) check |= 4;
    if (__builtin_cpu_supports("fma4")) check |= 8;
    if (__builtin_cpu_supports("xop")) check |= 16;
    
    (void)check;
}
