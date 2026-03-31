/* sel-sched-test.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <emmintrin.h>  /* SSE2 intrinsics */
#include <xmmintrin.h>  /* SSE intrinsics */

/* Enable selective scheduling optimizations on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void complex_loop_carried_deps(int N, double* restrict out, const double* restrict in) {
    /* Nested loops with varying trip counts creating complex dependencies */
    for (int i = 1; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operation with mixed computations */
            acc += in[j] * (j % 2 ? 0.5 : 2.0) + sin(in[j] * 0.01);
        }
        /* Conditional move/select operation */
        out[i] = (acc > 100.0) ? acc * 0.9 : acc * 1.1;
        
        /* Inline assembly with clobbers to force scheduling constraints */
        asm volatile ("# Selective scheduler test\n\t"
                      "nop" : : : "rax", "rbx", "memory");
    }
}

/* Mixed data type structure for non-contiguous access */
struct MixedData {
    int id;
    double value;
    char tag;
    float extra[3];
};

__attribute__((optimize("O3", "fsel-sched-pipelining", "funroll-loops")))
static void mixed_data_access(struct MixedData* arr, int size) {
    /* Non-contiguous, strided access pattern */
    for (int i = 0; i < size - 1; i += 2) {
        /* Pointer arithmetic with casting */
        double* dbl_ptr = &arr[i].value;
        float* flt_ptr = arr[i + 1].extra;
        
        /* Complex data flow with conditionals */
        double temp = *dbl_ptr * (arr[i].id % 10);
        for (int k = 0; k < 3; ++k) {
            flt_ptr[k] = (float)temp * (k + 1) * 0.25f;
        }
        
        /* Function call within loop - creates call instruction */
        arr[i].tag = (char)fmod(temp, 256.0);
    }
}

/* SIMD-intensive computation */
__attribute__((target("sse2")))
static void simd_processing(float* restrict dst, const float* restrict src, int len) {
    /* Process in SIMD chunks */
    int i;
    for (i = 0; i + 4 <= len; i += 4) {
        /* Load unaligned data */
        __m128 vec = _mm_loadu_ps(&src[i]);
        
        /* Mixed SIMD operations */
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 scaled = _mm_mul_ps(squared, _mm_set1_ps(0.5f));
        
        /* Conditional blend using comparison */
        __m128 threshold = _mm_set1_ps(100.0f);
        __m128 mask = _mm_cmpgt_ps(scaled, threshold);
        __m128 result = _mm_blendv_ps(scaled, threshold, mask);
        
        /* Store result */
        _mm_storeu_ps(&dst[i], result);
    }
    
    /* Scalar tail processing */
    for (; i < len; ++i) {
        dst[i] = src[i] * src[i] * 0.5f;
        dst[i] = dst[i] > 100.0f ? 100.0f : dst[i];
    }
}

/* Control flow with computed goto */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static int computed_goto_dispatch(int op, int x) {
    static const void* jump_table[] = {
        &&add_case, &&sub_case, &&mul_case, &&div_case, 
        &&mod_case, &&and_case, &&or_case, &&xor_case
    };
    
    if (op < 0 || op >= 8) return x;
    
    goto *jump_table[op];
    
add_case:
    return x + 7;
sub_case:
    return x - 3;
mul_case:
    return x * 2;
div_case:
    return x / 2;
mod_case:
    return x % 13;
and_case:
    return x & 0xFF;
or_case:
    return x | 0x55;
xor_case:
    return x ^ 0xAA;
}

/* Switch with mixed dense/sparse cases */
__attribute__((optimize("O2")))
static int complex_switch(int val) {
    switch (val) {
        /* Dense range */
        case 0:  return val * 2;
        case 1:  return val + 10;
        case 2:  return val - 5;
        case 3:  return val / 2;
        case 4:  return val % 3;
        
        /* Small gap */
        case 10: return val << 1;
        case 11: return val >> 1;
        
        /* Large sparse gap */
        case 100: return val ^ 0x1234;
        case 200: return val & 0xABCD;
        case 300: return val | 0x5678;
        
        default:
            /* Ternary operator generating conditional move */
            return (val > 0) ? val * 3 : val * (-2);
    }
}

/* Main computational kernel with loop unrolling pragma */
#pragma GCC unroll 4
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static double computational_kernel(const double* a, const double* b, int n) {
    double sum = 0.0;
    
    /* Unrolled loop with mixed operations */
    for (int i = 0; i < n; ++i) {
        double ai = a[i];
        double bi = b[i];
        
        /* Multiple dependent operations */
        double t1 = ai * bi;
        double t2 = sin(ai) * cos(bi);
        double t3 = sqrt(fabs(ai - bi));
        
        /* Complex expression with function calls */
        sum += t1 * 0.3 + t2 * 0.5 + t3 * 0.2;
        
        /* Periodic inline assembly */
        if (i % 8 == 0) {
            asm volatile ("# Loop iteration checkpoint\n\t"
                          "nop" : : : "rcx", "rdx", "memory");
        }
    }
    
    return sum;
}

/* Main function orchestrating all computations */
int main(void) {
    const int N = 1024;
    const int M = 512;
    
    /* Allocate and initialize data */
    double* data1 = (double*)aligned_alloc(32, N * sizeof(double));
    double* data2 = (double*)aligned_alloc(32, N * sizeof(double));
    float* fdata1 = (float*)aligned_alloc(16, M * sizeof(float));
    float* fdata2 = (float*)aligned_alloc(16, M * sizeof(float));
    struct MixedData* mixed = (struct MixedData*)malloc(M * sizeof(struct MixedData));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        data1[i] = (i * 1.5) / (i + 1);
        data2[i] = sin(i * 0.1) * 100.0;
    }
    
    for (int i = 0; i < M; ++i) {
        fdata1[i] = (float)(i * 0.7);
        mixed[i].id = i;
        mixed[i].value = i * 2.5;
        mixed[i].tag = (char)(i % 256);
        for (int j = 0; j < 3; ++j) {
            mixed[i].extra[j] = (float)(i + j) * 0.3f;
        }
    }
    
    double checksum = 0.0;
    
    /* Execute all computational patterns */
    complex_loop_carried_deps(N, data2, data1);
    checksum += data2[N-1];
    
    mixed_data_access(mixed, M);
    checksum += mixed[M-1].value;
    
    simd_processing(fdata2, fdata1, M);
    checksum += fdata2[M-1];
    
    int int_result = 0;
    for (int i = 0; i < 8; ++i) {
        int_result ^= computed_goto_dispatch(i, i * 17);
    }
    checksum += int_result;
    
    for (int i = 0; i < 10; ++i) {
        int_result += complex_switch(i * 30);
    }
    checksum += int_result;
    
    double kernel_result = computational_kernel(data1, data2, N);
    checksum += kernel_result;
    
    /* Final output to prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    printf("Results: data2[0]=%f, fdata2[0]=%f, mixed[0].tag=%d\n", 
           data2[0], fdata2[0], (int)mixed[0].tag);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(fdata1);
    free(fdata2);
    free(mixed);
    
    return 0;
}
