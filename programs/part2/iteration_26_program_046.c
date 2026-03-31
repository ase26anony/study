/* sel-sched-trigger.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Enable selective scheduling optimizations on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void complex_loop_carried_deps(int N, double* restrict out, const double* restrict in) {
    /* Nested loops with varying trip counts */
    for (int i = 1; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index - creates complex dependencies */
        for (int j = 0; j < i; ++j) {
            /* Mixed operations with data dependencies */
            acc += in[j] * (j % 3 == 0 ? 1.5 : 0.5);
            /* Conditional move operation */
            acc = (acc > 100.0) ? 100.0 : acc;
        }
        out[i] = acc;
        
        /* Inline assembly with clobbers - forces scheduler constraints */
        asm volatile ("# Selective scheduler test\n\t"
                     "nop" : : : "rax", "rbx", "memory");
    }
}

/* Mixed data structure for non-contiguous access */
struct MixedData {
    int a;
    double b;
    char c[8];
    float d;
};

__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void mixed_memory_access(struct MixedData* arr, int size) {
    /* Non-contiguous, strided access pattern */
    for (int i = 0; i < size - 1; i += 2) {
        /* Pointer arithmetic with casting */
        arr[i].b = arr[i + 1].a * 0.5;
        arr[i].d = (float)arr[i].b * 2.0f;
        
        /* Function call within loop - creates call instruction */
        arr[i].b = sin(arr[i].b) + cos(arr[i].d);
    }
}

/* SIMD operations using intrinsics */
__attribute__((target("sse2")))
static void simd_operations(float* dst, const float* src, int len) {
    /* Process with SIMD where possible */
    int i = 0;
    for (; i + 3 < len; i += 4) {
        __m128 vec = _mm_loadu_ps(&src[i]);
        __m128 result = _mm_add_ps(vec, _mm_mul_ps(vec, vec));
        _mm_storeu_ps(&dst[i], result);
    }
    
    /* Scalar tail processing */
    for (; i < len; ++i) {
        dst[i] = src[i] + src[i] * src[i];
    }
}

/* Complex control flow with computed goto */
__attribute__((optimize("O2")))
static int computed_goto_switch(int x) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_default
    };
    
    int idx = (x >= 0 && x < 6) ? x : 6;
    int result = 0;
    
    goto *jump_table[idx];
    
case_0:
    result = x * 2;
    goto end;
case_1:
    result = x + 100;
    goto end;
case_2:
    result = x / 2;
    goto end;
case_3:
    result = x * x;
    goto end;
case_4:
    result = x | 0xFF;
    goto end;
case_5:
    result = x & 0x0F;
    goto end;
case_default:
    result = -x;
    goto end;
    
end:
    return result;
}

/* Loop with pragma unrolling */
#pragma GCC unroll 4
static void unrolled_loop(int* data, int n) {
    for (int i = 0; i < n; ++i) {
        /* Complex expression with multiple operations */
        data[i] = (data[i] * 3 + 7) % 256;
        data[i] ^= (data[i] >> 4);
        data[i] = data[i] < 128 ? data[i] * 2 : data[i] / 2;
    }
}

/* Sparse switch statement */
static int sparse_switch(int val) {
    switch (val) {
        case 1:   return val * 10;
        case 10:  return val + 100;
        case 100: return val / 10;
        case 1000:return val - 500;
        case 10000: return val % 100;
        default:  return -val;
    }
}

/* Main computational kernel */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static unsigned long benchmark_kernel(int iterations) {
    const int N = 1024;
    double* array1 = malloc(N * sizeof(double));
    double* array2 = malloc(N * sizeof(double));
    struct MixedData* mixed = malloc(N * sizeof(struct MixedData));
    float* simd_src = malloc(N * sizeof(float));
    float* simd_dst = malloc(N * sizeof(float));
    int* int_data = malloc(N * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        array1[i] = sin(i * 0.1);
        mixed[i].a = i;
        mixed[i].b = cos(i * 0.05);
        simd_src[i] = (float)(i % 100) * 0.1f;
        int_data[i] = i;
    }
    
    unsigned long checksum = 0;
    
    for (int iter = 0; iter < iterations; ++iter) {
        /* Call all complex functions to create scheduling pressure */
        complex_loop_carried_deps(N, array2, array1);
        mixed_memory_access(mixed, N);
        simd_operations(simd_dst, simd_src, N);
        unrolled_loop(int_data, N);
        
        /* Mix in control flow operations */
        for (int i = 0; i < 100; ++i) {
            int val = computed_goto_switch(i % 7);
            val += sparse_switch((i * 17) % 10001);
            checksum ^= (unsigned long)val;
        }
        
        /* Update data for next iteration */
        for (int i = 0; i < N; ++i) {
            array1[i] = array2[i] * 0.99;
            int_data[i] = (int_data[i] + iter) % 1000;
        }
        
        /* Function calls with varying arguments */
        checksum += (unsigned long)pow(2.0, (iter % 10) + 1);
    }
    
    /* Final aggregation to prevent dead code elimination */
    for (int i = 0; i < N; ++i) {
        checksum ^= (unsigned long)array2[i];
        checksum += (unsigned long)mixed[i].a;
        checksum ^= (unsigned long)simd_dst[i];
        checksum += int_data[i];
    }
    
    free(array1);
    free(array2);
    free(mixed);
    free(simd_src);
    free(simd_dst);
    free(int_data);
    
    return checksum;
}

int main() {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Seed RNG for variability */
    srand(time(NULL));
    
    /* Run benchmark with multiple iterations */
    int iterations = 50;
    unsigned long result = benchmark_kernel(iterations);
    
    printf("Result checksum: %lu\n", result);
    printf("Program completed.\n");
    
    return 0;
}
