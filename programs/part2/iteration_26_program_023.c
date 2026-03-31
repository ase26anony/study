/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug output in GCC,
 * specifically targeting dump_insn_rtx_1 with switch_dump(stderr)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Enable selective scheduling optimizations on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void complex_loop_carried_deps(int N, double* restrict out, const double* restrict in) {
    /* Loop-carried dependencies with varying trip counts */
    for (int i = 0; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index - creates complex deps */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed math */
            acc += in[j] * sin(in[i] * 0.01) + cos(in[j] * 0.02);
        }
        /* Conditional move operation */
        out[i] = (acc > 0) ? acc * 1.5 : acc * 0.5;
    }
}

/* Mixed data types and non-contiguous access */
struct MixedData {
    int a;
    double b;
    char c;
    float d;
};

__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void mixed_data_processing(struct MixedData* arr, int size) {
    /* Non-contiguous memory access pattern */
    for (int i = 0; i < size; i += 2) {
        /* Pointer arithmetic with casting */
        double* ptr = (double*)((char*)&arr[i].b + (i % 16));
        arr[i].b = arr[i + 1].a * 0.5 + *ptr;
        
        /* Inline assembly with clobbers - forces scheduler constraints */
        asm volatile ("# Start barrier\n\t"
                      "mfence\n\t"
                      "# End barrier" : : : "memory");
    }
}

/* SIMD operations with intrinsics */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
static void simd_processing(float* restrict dst, const float* restrict src, int len) {
    int i;
    /* Process with SSE intrinsics */
    for (i = 0; i + 4 <= len; i += 4) {
        __m128 a = _mm_loadu_ps(src + i);
        __m128 b = _mm_add_ps(a, a);
        __m128 c = _mm_mul_ps(b, _mm_set1_ps(1.5f));
        _mm_storeu_ps(dst + i, c);
        
        /* Another inline asm with register clobbers */
        asm volatile ("nop\n\tnop\n\tnop" : : : "eax", "ebx");
    }
    
    /* Scalar tail processing */
    for (; i < len; ++i) {
        dst[i] = src[i] * 3.0f;
    }
}

/* Complex control flow with computed goto */
__attribute__((optimize("O2")))
static int computed_goto_example(int x) {
    static void* jtable[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int result = 0;
    int idx = x % 5;
    
    /* Indirect branch - challenging for scheduler */
    goto *jtable[idx];
    
label0:
    result = x * 2;
    goto end;
label1:
    result = x + 100;
    goto end;
label2:
    result = x / 3;
    goto end;
label3:
    result = x * x;
    goto end;
label4:
    result = 0;
    goto end;
    
end:
    return result;
}

/* Switch with mixed dense/sparse cases */
__attribute__((optimize("O3", "funroll-loops")))
static int switch_mixed_cases(int val) {
    int result = 0;
    
    /* Dense cases 0-9 */
    switch (val) {
        case 0: result = 1; break;
        case 1: result = 2; break;
        case 2: result = 3; break;
        case 3: result = 5; break;
        case 4: result = 8; break;
        case 5: result = 13; break;
        case 6: result = 21; break;
        case 7: result = 34; break;
        case 8: result = 55; break;
        case 9: result = 89; break;
        /* Sparse cases */
        case 100: result = 1000; break;
        case 200: result = 2000; break;
        case 300: result = 3000; break;
        case 1000: result = 10000; break;
        default: result = -1; break;
    }
    
    return result;
}

/* Function with loop unrolling pragma */
#pragma GCC unroll 4
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static void unrolled_processing(int* data, int n) {
    for (int i = 0; i < n; ++i) {
        /* Complex expression with multiple operations */
        data[i] = (data[i] * 3 + 7) & 0xFF;
        data[i] ^= (data[i] >> 4) | (data[i] << 4);
        data[i] = (data[i] > 128) ? data[i] - 64 : data[i] + 64;
    }
}

/* Main computational kernel */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static unsigned long benchmark_kernel(int iterations) {
    const int N = 1024;
    double* array1 = (double*)aligned_alloc(64, N * sizeof(double));
    double* array2 = (double*)aligned_alloc(64, N * sizeof(double));
    struct MixedData* mixed = (struct MixedData*)aligned_alloc(64, N * sizeof(struct MixedData));
    float* fdata1 = (float*)aligned_alloc(64, N * sizeof(float));
    float* fdata2 = (float*)aligned_alloc(64, N * sizeof(float));
    int* idata = (int*)aligned_alloc(64, N * sizeof(int));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; ++i) {
        array1[i] = sin(i * 0.01);
        mixed[i].a = i;
        mixed[i].b = i * 0.5;
        mixed[i].c = (char)(i & 0xFF);
        mixed[i].d = (float)(i * 0.1f);
        fdata1[i] = (float)cos(i * 0.02);
        idata[i] = i;
    }
    
    unsigned long checksum = 0;
    
    for (int iter = 0; iter < iterations; ++iter) {
        /* Call all complex functions to create varied scheduling scenarios */
        complex_loop_carried_deps(N, array2, array1);
        mixed_data_processing(mixed, N);
        simd_processing(fdata2, fdata1, N);
        unrolled_processing(idata, N);
        
        /* Mix in control flow functions */
        for (int i = 0; i < 100; ++i) {
            checksum ^= computed_goto_example(i);
            checksum += switch_mixed_cases(i % 1001);
        }
        
        /* Update data for next iteration to prevent dead code elimination */
        for (int i = 0; i < N; ++i) {
            array1[i] = array2[i] * 0.99;
            fdata1[i] = fdata2[i] * 0.95f;
            idata[i] = (idata[i] + iter) & 0xFFFF;
        }
    }
    
    /* Final checksum computation to ensure all work is used */
    for (int i = 0; i < N; ++i) {
        checksum += (unsigned long)(array2[i] * 1000);
        checksum ^= (unsigned long)(fdata2[i] * 1000);
        checksum += mixed[i].a;
        checksum ^= idata[i];
    }
    
    free(array1);
    free(array2);
    free(mixed);
    free(fdata1);
    free(fdata2);
    free(idata);
    
    return checksum;
}

int main() {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Seed RNG for variability */
    srand(time(NULL));
    
    /* Run benchmark with multiple iterations to ensure sustained scheduling activity */
    int iterations = 10;
    unsigned long result = benchmark_kernel(iterations);
    
    printf("Result checksum: %lu\n", result);
    printf("Program completed successfully.\n");
    
    return 0;
}
