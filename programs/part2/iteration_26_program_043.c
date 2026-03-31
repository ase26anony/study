/* sel-sched-trigger.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Enable selective scheduling optimizations on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void complex_loop_carried_deps(int N, double* restrict out, const double* restrict in) {
    /* Loop-carried dependencies with varying trip counts */
    for (int i = 0; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed computations */
            acc += in[j] * (j % 2 ? 0.5 : 2.0) + sin(in[i] * 0.01);
        }
        /* Conditional move/select operation */
        out[i] = (acc > 100.0) ? sqrt(acc) : acc * acc;
    }
}

/* Mixed data types and non-contiguous memory access */
struct MixedData {
    int a;
    double b;
    char c;
    float d;
};

__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void mixed_data_access(struct MixedData* arr, int size) {
    /* Non-contiguous access pattern */
    for (int i = 0; i < size - 1; i += 2) {
        /* Pointer arithmetic with casting */
        double* ptr = &arr[i].b;
        float* fptr = &arr[i + 1].d;
        
        /* Complex dependency chain */
        arr[i].b = arr[i + 1].a * 0.5 + *fptr;
        arr[i + 1].d = (float)(*ptr * 2.0);
        
        /* Inline assembly with clobbers to force scheduling constraints */
        asm volatile ("# Mixed data assembly\n\t"
                     "nop" : : : "rax", "rbx", "memory");
    }
}

/* SIMD operations with intrinsics */
__attribute__((target("sse2,avx"), optimize("O2", "fsel-sched-pipelining")))
static void simd_operations(float* restrict dst, const float* restrict src, int len) {
    int i;
    /* Process with SSE */
    for (i = 0; i + 4 <= len; i += 4) {
        __m128 a = _mm_loadu_ps(src + i);
        __m128 b = _mm_add_ps(a, a);
        __m128 c = _mm_mul_ps(b, _mm_set1_ps(0.5f));
        _mm_storeu_ps(dst + i, c);
    }
    
    /* Process remainder with AVX if available */
    #ifdef __AVX__
    for (; i + 8 <= len; i += 8) {
        __m256 a = _mm256_loadu_ps(src + i);
        __m256 b = _mm256_add_ps(a, a);
        _mm256_storeu_ps(dst + i, b);
    }
    #endif
    
    /* Scalar tail processing */
    for (; i < len; ++i) {
        dst[i] = src[i] * (i % 3 ? 1.5f : 0.75f);
    }
}

/* Complex control flow with computed goto */
__attribute__((optimize("O2")))
static int computed_goto_pattern(int x) {
    static const void* jtable[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7
    };
    
    int idx = x & 7;
    int result = 0;
    
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
    result = x ^ 0xFF;
    goto end;
label4:
    result = x << 2;
    goto end;
label5:
    result = x >> 1;
    goto end;
label6:
    result = ~x;
    goto end;
label7:
    result = x * x;
    goto end;
    
end:
    return result;
}

/* Switch with mixed dense/sparse cases */
__attribute__((optimize("O3", "funroll-loops")))
static int switch_pattern(int val) {
    int result = 0;
    
    /* Mixed case pattern */
    switch (val) {
        /* Dense range */
        case 0: case 1: case 2: case 3: case 4:
            result = val * 10;
            break;
        /* Sparse cases */
        case 10: case 20: case 30: case 40:
            result = val / 2;
            break;
        case 100: case 200: case 300:
            result = val + 5;
            break;
        /* Large gap */
        case 1000:
            result = 999;
            break;
        default:
            result = -val;
    }
    
    return result;
}

/* Loop with pragma unrolling */
#pragma GCC push_options
#pragma GCC optimize ("O3")
#pragma GCC unroll 4
static void unrolled_loop(int* data, int size) {
    for (int i = 0; i < size; ++i) {
        /* Complex operation encouraging pipelining */
        data[i] = (data[i] * 3 + 7) ^ (data[i] >> 4);
        
        /* Function call in loop */
        data[i] += abs(data[i] % 17 - 8);
    }
}
#pragma GCC pop_options

/* Main computational kernel */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static unsigned long long run_computations(void) {
    const int N = 512;
    unsigned long long checksum = 0;
    
    /* Allocate and initialize data */
    double* array1 = aligned_alloc(64, N * sizeof(double));
    double* array2 = aligned_alloc(64, N * sizeof(double));
    float* farray1 = aligned_alloc(32, N * sizeof(float));
    float* farray2 = aligned_alloc(32, N * sizeof(float));
    struct MixedData* mdata = aligned_alloc(32, N * sizeof(struct MixedData));
    int* intdata = aligned_alloc(64, N * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        array1[i] = sin(i * 0.1);
        farray1[i] = i * 0.25f;
        mdata[i].a = i;
        mdata[i].b = i * 0.5;
        mdata[i].c = (char)(i & 0xFF);
        mdata[i].d = i * 0.75f;
        intdata[i] = i ^ 0x55AA55AA;
    }
    
    /* Run all computation patterns */
    complex_loop_carried_deps(N, array2, array1);
    mixed_data_access(mdata, N);
    simd_operations(farray2, farray1, N);
    unrolled_loop(intdata, N);
    
    /* Combine results into checksum */
    for (int i = 0; i < N; ++i) {
        checksum ^= *(unsigned long long*)&array2[i];
        checksum += mdata[i].a + (int)mdata[i].b;
        checksum ^= *(unsigned int*)&farray2[i];
        checksum += intdata[i];
        
        /* Call pattern functions */
        checksum += computed_goto_pattern(i);
        checksum += switch_pattern(i % 1001);
    }
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(farray1);
    free(farray2);
    free(mdata);
    free(intdata);
    
    return checksum;
}

int main(void) {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Run multiple iterations to increase scheduling opportunities */
    unsigned long long final_checksum = 0;
    for (int iter = 0; iter < 3; ++iter) {
        final_checksum ^= run_computations();
        printf("Iteration %d complete\n", iter);
    }
    
    printf("Final checksum: 0x%016llX\n", final_checksum);
    return 0;
}
