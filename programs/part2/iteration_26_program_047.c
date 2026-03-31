/* sel-sched-trigger.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Enable selective scheduling optimizations on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void complex_loop_dependencies(int N, double* restrict out, const double* restrict in) {
    /* Loop-carried dependencies with varying trip counts */
    for (int i = 0; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed math */
            acc += in[j] * sin(in[i] * 0.01) + cos(in[j] * 0.02);
        }
        /* Conditional move operation */
        out[i] = (acc > 0.0) ? acc * 1.5 : acc * 0.5;
    }
}

/* Mixed data structure for non-contiguous access */
struct MixedData {
    int a;
    double b;
    char c;
    float d;
};

__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void mixed_data_access(struct MixedData* arr, int size) {
    /* Non-contiguous memory access pattern */
    for (int i = 0; i < size - 1; i += 2) {
        arr[i].b = arr[i + 1].a * 0.5 + sin(arr[i].d);
        arr[i + 1].d = arr[i].b * 0.3f + cos(arr[i + 1].a);
        
        /* Pointer arithmetic with casting */
        char* ptr = (char*)&arr[i];
        for (int j = 0; j < 4; ++j) {
            ptr[j] ^= (i + j) & 0xFF;
        }
    }
}

/* SIMD operations with intrinsics */
__attribute__((target("sse2")))
static void simd_operations(float* dst, const float* src, int len) {
    /* Process with SIMD where possible, scalar for remainder */
    int i = 0;
    for (; i + 3 < len; i += 4) {
        __m128 a = _mm_loadu_ps(&src[i]);
        __m128 b = _mm_add_ps(a, a);
        __m128 c = _mm_mul_ps(b, _mm_set1_ps(1.5f));
        _mm_storeu_ps(&dst[i], c);
    }
    
    /* Scalar tail processing */
    for (; i < len; ++i) {
        dst[i] = src[i] * 2.0f * 1.5f;
    }
}

/* Function with computed goto for complex control flow */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
static int computed_goto_example(int x) {
    static const void* jtable[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    
    int idx = x % 5;
    int result = 0;
    
    /* Inline assembly with clobbered registers */
    asm volatile ("nop" : : : "eax", "memory");
    
    goto *jtable[idx];
    
case0:
    result = x * 2;
    /* Another inline asm with different clobbers */
    asm volatile ("nop" : : : "ebx", "memory");
    goto end;
    
case1:
    result = x + x;
    goto end;
    
case2:
    result = x | 0xFF;
    goto end;
    
case3:
    result = x ^ (x >> 3);
    goto end;
    
default_case:
    result = -x;
    goto end;
    
end:
    return result;
}

/* Switch statement with mixed density */
__attribute__((optimize("O2")))
static int dense_sparse_switch(int val) {
    int result = 0;
    
    switch (val) {
        /* Dense range */
        case 0:  result = val * 2; break;
        case 1:  result = val + 10; break;
        case 2:  result = val - 5; break;
        case 3:  result = val | 0x0F; break;
        case 4:  result = val ^ 0xFF; break;
        
        /* Sparse jump */
        case 10: result = val << 2; break;
        case 20: result = val >> 1; break;
        case 50: result = val * val; break;
        case 100: result = val % 13; break;
        
        default: result = -val; break;
    }
    
    return result;
}

/* Matrix-like operation with loop unrolling hint */
#pragma GCC unroll 4
__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void matrix_style_ops(int size, double mat[][64]) {
    for (int i = 0; i < size; ++i) {
        double row_sum = 0.0;
        for (int j = 0; j < 64; ++j) {
            /* Mix of operations */
            mat[i][j] = mat[i][j] * 1.1 + sin(mat[i][j] * 0.01);
            row_sum += mat[i][j];
            
            /* Conditional operation */
            if (j % 8 == 0) {
                mat[i][j] = pow(mat[i][j], 1.5);
            }
        }
        /* Use row_sum to prevent dead code elimination */
        mat[i][0] += row_sum * 0.001;
    }
}

/* Main computational kernel */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static unsigned long run_computations(void) {
    const int N = 512;
    const int M = 100;
    
    /* Allocate and initialize data */
    double* array1 = aligned_alloc(64, N * sizeof(double));
    double* array2 = aligned_alloc(64, N * sizeof(double));
    struct MixedData* mixed = aligned_alloc(64, M * sizeof(struct MixedData));
    float* simd_src = aligned_alloc(16, N * sizeof(float));
    float* simd_dst = aligned_alloc(16, N * sizeof(float));
    double matrix[32][64];
    
    srand(time(NULL));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; ++i) {
        array1[i] = (i * 1.5 + sin(i * 0.1)) / (i + 1);
        simd_src[i] = (float)((i % 100) * 0.01f);
    }
    
    for (int i = 0; i < M; ++i) {
        mixed[i].a = i;
        mixed[i].b = i * 0.5;
        mixed[i].c = (char)(i & 0xFF);
        mixed[i].d = (float)(i * 0.1f);
    }
    
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 64; ++j) {
            matrix[i][j] = (i * 64 + j) * 0.01;
        }
    }
    
    unsigned long checksum = 0;
    
    /* Execute all computational patterns */
    complex_loop_dependencies(N, array2, array1);
    
    mixed_data_access(mixed, M);
    
    simd_operations(simd_dst, simd_src, N);
    
    matrix_style_ops(32, matrix);
    
    /* Combine results into checksum */
    for (int i = 0; i < N; ++i) {
        checksum ^= *(unsigned long*)&array2[i];
        checksum += (unsigned long)(simd_dst[i] * 1000);
    }
    
    for (int i = 0; i < M; ++i) {
        checksum ^= mixed[i].a;
        checksum += *(unsigned long*)&mixed[i].b;
    }
    
    for (int i = 0; i < 1000; ++i) {
        checksum += computed_goto_example(i);
        checksum += dense_sparse_switch(i % 150);
    }
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(mixed);
    free(simd_src);
    free(simd_dst);
    
    return checksum;
}

int main(void) {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Run multiple iterations to increase scheduling opportunities */
    unsigned long final_checksum = 0;
    for (int iter = 0; iter < 3; ++iter) {
        final_checksum ^= run_computations();
        printf("Iteration %d complete\n", iter);
    }
    
    printf("Final checksum: 0x%016lx\n", final_checksum);
    printf("Program completed successfully.\n");
    
    return 0;
}
