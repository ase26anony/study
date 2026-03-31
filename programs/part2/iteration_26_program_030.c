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
    /* Nested loops with varying trip counts creating complex dependencies */
    for (int i = 1; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed computations */
            double val = in[j] * (j % 3 == 0 ? 2.0 : (j % 3 == 1 ? 1.5 : 0.5));
            acc += val * sin(val * 0.01);
            
            /* Conditional move/select operations */
            double threshold = (i + j) % 100;
            acc = (val > threshold) ? acc * 1.1 : acc * 0.9;
            
            /* Inline assembly with clobbers to force scheduler constraints */
            asm volatile ("# Selective scheduler test\n\t"
                         "nop\n\t"
                         : : : "rax", "rbx", "rcx", "memory");
        }
        out[i] = acc + pow(acc, 1.0 / (i % 5 + 1));
    }
}

/* Mixed data structure with non-contiguous access patterns */
struct MixedData {
    int a;
    double b;
    char c[8];
    float d;
    long e;
};

__attribute__((optimize("O3", "fsel-sched-pipelining", "funroll-loops")))
static void mixed_data_access(struct MixedData* arr, int size) {
    /* Non-contiguous, strided access pattern */
    for (int i = 0; i < size - 1; i += 2) {
        /* Pointer arithmetic with casting */
        double* b_ptr = &arr[i].b;
        float* d_ptr = &arr[i + 1].d;
        
        /* Complex data flow with type conversions */
        arr[i].b = arr[i + 1].a * 0.5 + sin(*d_ptr);
        arr[i + 1].d = (float)(*b_ptr * arr[i].e) / 256.0f;
        
        /* String operations that may generate various instructions */
        memcpy(arr[i].c, arr[i + 1].c, sizeof(arr[i].c));
        arr[i].c[0] = (char)(arr[i].a % 256);
        
        /* Function call with varying arguments */
        arr[i].e = (long)pow(fabs(arr[i].b), 2.0 + (i % 3));
    }
}

/* SIMD operations using intrinsics */
__attribute__((target("sse2,avx"), optimize("O2", "fsel-sched-pipelining")))
static void simd_operations(float* dst, const float* src, int len) {
    int i;
    /* Process with SSE */
    for (i = 0; i + 4 <= len; i += 4) {
        __m128 a = _mm_loadu_ps(src + i);
        __m128 b = _mm_add_ps(a, _mm_set1_ps(1.0f));
        __m128 c = _mm_mul_ps(b, _mm_set1_ps(0.5f));
        _mm_storeu_ps(dst + i, c);
        
        /* Mix with scalar operations */
        dst[i] = dst[i] > 0.0f ? dst[i] : -dst[i];
    }
    
    /* Remainder loop */
    for (; i < len; ++i) {
        dst[i] = src[i] * 0.5f + 0.5f;
        /* Ternary operator generating conditional moves */
        dst[i] = (i % 2 == 0) ? dst[i] * 2.0f : dst[i] / 2.0f;
    }
}

/* Complex control flow with computed goto */
__attribute__((optimize("O2")))
static int computed_goto_pattern(int x) {
    static const void* jtable[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    
    int idx = x % 5;
    int result = 0;
    
    /* Indirect branch */
    goto *jtable[idx];
    
case0:
    result = x * 2;
    goto end;
case1:
    result = x + 100;
    goto end;
case2:
    result = x / 2;
    goto end;
case3:
    result = x - 50;
    goto end;
default_case:
    result = x ^ 0xFF;
    goto end;
    
end:
    return result;
}

/* Switch with mixed dense/sparse cases */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static int mixed_switch(int val) {
    int result = 0;
    
    switch (val) {
        /* Dense range */
        case 0:  result = val + 1; break;
        case 1:  result = val * 2; break;
        case 2:  result = val << 3; break;
        case 3:  result = val | 0xF0; break;
        case 4:  result = val & 0x0F; break;
        
        /* Sparse range */
        case 10: result = val / 2; break;
        case 50: result = val % 7; break;
        case 100: result = val ^ 0xAA; break;
        case 255: result = ~val; break;
        
        default:
            /* Complex default computation */
            for (int i = 0; i < (val % 8); ++i) {
                result += (val << i);
            }
            break;
    }
    
    return result;
}

/* Main computational kernel with loop unrolling pragma */
#pragma GCC push_options
#pragma GCC optimize ("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")
static double computational_kernel(int iterations) {
    double sum = 0.0;
    double a = 1.0, b = 2.0, c = 3.0;
    
    #pragma GCC unroll 4
    for (int i = 0; i < iterations; ++i) {
        /* Complex dependency chain */
        double t1 = a * sin(b);
        double t2 = b * cos(c);
        double t3 = c * tan(a * 0.01);
        
        a = t1 + t2 * 0.5;
        b = t2 - t3 * 0.3;
        c = t3 + t1 * 0.7;
        
        sum += a + b + c;
        
        /* Periodic function call */
        if (i % 7 == 0) {
            sum += pow(fabs(a), 1.0 / (i % 5 + 2));
        }
        
        /* More inline assembly */
        asm volatile ("# Kernel computation\n\t"
                     "add $1, %%eax\n\t"
                     : : : "eax", "memory");
    }
    
    return sum;
}
#pragma GCC pop_options

int main(void) {
    const int N = 512;
    const int M = 100;
    
    /* Initialize data */
    double* data1 = aligned_alloc(64, N * sizeof(double));
    double* data2 = aligned_alloc(64, N * sizeof(double));
    float* fdata1 = aligned_alloc(64, N * sizeof(float));
    float* fdata2 = aligned_alloc(64, N * sizeof(float));
    struct MixedData* mixed = aligned_alloc(64, M * sizeof(struct MixedData));
    
    if (!data1 || !data2 || !fdata1 || !fdata2 || !mixed) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        data1[i] = sin(i * 0.1);
        fdata1[i] = (float)(i * 0.01);
    }
    
    for (int i = 0; i < M; ++i) {
        mixed[i].a = i;
        mixed[i].b = i * 0.5;
        mixed[i].d = (float)(i * 0.1);
        mixed[i].e = i * 10L;
        snprintf(mixed[i].c, sizeof(mixed[i].c), "val%d", i);
    }
    
    /* Execute all patterns to trigger scheduler activity */
    double checksum = 0.0;
    
    /* Pattern 1: Loop-carried dependencies */
    complex_loop_carried_deps(N, data2, data1);
    for (int i = 0; i < N; ++i) checksum += data2[i];
    
    /* Pattern 2: Mixed data access */
    mixed_data_access(mixed, M);
    for (int i = 0; i < M; ++i) checksum += mixed[i].b + mixed[i].d;
    
    /* Pattern 3: SIMD operations */
    simd_operations(fdata2, fdata1, N);
    for (int i = 0; i < N; ++i) checksum += fdata2[i];
    
    /* Pattern 4: Complex control flow */
    int control_sum = 0;
    for (int i = 0; i < 100; ++i) {
        control_sum += computed_goto_pattern(i);
        control_sum += mixed_switch(i % 300);
    }
    checksum += control_sum;
    
    /* Pattern 5: Computational kernel */
    checksum += computational_kernel(1000);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %.15f\n", checksum);
    printf("Control sum: %d\n", control_sum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(fdata1);
    free(fdata2);
    free(mixed);
    
    return 0;
}
