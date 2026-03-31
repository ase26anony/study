/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug output in GCC's sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug sel-sched-trigger.c -o sel-sched-trigger -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Enable selective scheduling optimizations on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static double process_mixed_data(int N, double* restrict result);

__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void simd_intensive_processing(float* restrict src, float* restrict dst, int size);

/* Structure with mixed data types to create complex memory access patterns */
struct MixedData {
    int id;
    double value;
    char tag;
    float vec[4];
    long long timestamp;
};

/* Global jump table for computed goto */
static void* jumptable[] = { &&case0, &&case1, &&case2, &&case3, &&case4 };

/* Helper function with varying arguments to create call instructions */
__attribute__((noinline))
static double complex_math(double x, int n) {
    double result = x;
    for (int i = 0; i < n % 5; ++i) {
        result = sin(result) * cos(result);
        /* Inline assembly with clobbers to force scheduler constraints */
        asm volatile ("# Force register pressure" : : : "rax", "rbx", "rcx", "xmm0", "xmm1", "memory");
    }
    return result;
}

/* Function with loop-carried dependencies and varying trip counts */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static double nested_dependency_loop(int N) {
    double sum = 0.0;
    
    /* Outer loop with inner loop dependent on outer index */
    for (int i = 1; i < N; ++i) {
        double temp = i * 0.01;
        
        /* Inner loop with data-dependent trip count */
        for (int j = 0; j < i % 7; ++j) {
            /* Mixed operations creating dependency chain */
            temp = (temp > 0.5) ? temp * 0.9 : temp * 1.1;  /* Conditional move */
            temp += sin(j * 0.1);
            sum += temp;
            
            /* Function call within dependency chain */
            if (j % 3 == 0) {
                sum += complex_math(temp, j);
            }
        }
        
        /* Switch statement with sparse cases */
        switch (i % 13) {
            case 0: sum *= 0.99; break;
            case 1: sum += 1.01; break;
            case 5: sum = pow(sum, 1.001); break;
            case 7: sum = fmod(sum, 100.0); break;
            case 12: sum = -sum; break;
            default: sum += i * 0.001;
        }
    }
    return sum;
}

/* SIMD intensive processing with unrolled loops */
#pragma GCC unroll 4
__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void simd_intensive_processing(float* restrict src, float* restrict dst, int size) {
    /* Process with SSE intrinsics */
    for (int i = 0; i < size - 3; i += 4) {
        __m128 a = _mm_loadu_ps(&src[i]);
        __m128 b = _mm_loadu_ps(&src[(i + 1) % size]);
        
        /* Multiple SIMD operations in dependency chain */
        __m128 c = _mm_add_ps(a, b);
        __m128 d = _mm_mul_ps(c, _mm_set1_ps(0.5f));
        __m128 e = _mm_sub_ps(d, _mm_set1_ps(1.0f));
        
        /* Conditional select using comparison */
        __m128 mask = _mm_cmpgt_ps(a, b);
        __m128 result = _mm_or_ps(_mm_and_ps(mask, a), 
                                 _mm_andnot_ps(mask, e));
        
        _mm_storeu_ps(&dst[i], result);
        
        /* Inline assembly that clobbers XMM registers */
        asm volatile ("# SIMD clobber" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    }
    
    /* Handle remainder with scalar operations */
    for (int i = size - (size % 4); i < size; ++i) {
        dst[i] = (src[i] > 0.5f) ? src[i] * 0.9f : src[i] * 1.1f;
    }
}

/* Function using computed goto for indirect branching */
__attribute__((noinline, optimize("O2")))
static int computed_goto_dispatch(int idx, int value) {
    int result = value;
    
    if (idx >= 0 && idx < 5) {
        goto *jumptable[idx];
    }
    
    return result;
    
case0:
    result += complex_math(value, 1);
    goto end;
case1:
    result *= 2;
    /* Fall through */
case2:
    result = (result > 100) ? result - 50 : result + 50;
    goto end;
case3:
    result = result ^ 0x55AA55AA;
    goto end;
case4:
    result = result >> (value % 4);
    goto end;
end:
    return result;
}

/* Process array of structures with non-contiguous access */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
static double process_mixed_data(int N, double* restrict result) {
    struct MixedData* data = malloc(N * sizeof(struct MixedData));
    double total = 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        data[i].id = i;
        data[i].value = sin(i * 0.1);
        data[i].tag = 'A' + (i % 26);
        for (int j = 0; j < 4; ++j) {
            data[i].vec[j] = (i + j) * 0.25f;
        }
        data[i].timestamp = i * 1000LL;
    }
    
    /* Process with stride and pointer arithmetic */
    for (int i = 0; i < N - 1; i += 2) {
        /* Access non-contiguous structure members */
        double val1 = data[i].value;
        float* vec_ptr = data[i + 1].vec;
        
        /* Complex pointer arithmetic and casting */
        val1 *= *(double*)((char*)&data[i].timestamp + 4);
        
        /* Mixed-type operations */
        for (int j = 0; j < 3; ++j) {
            val1 += vec_ptr[j] * (j + 1);
            
            /* Conditional move in hot loop */
            val1 = (val1 > 100.0) ? val1 * 0.99 : val1 * 1.01;
        }
        
        total += val1;
        
        /* Computed goto every 8th iteration */
        if (i % 8 == 0) {
            total += computed_goto_dispatch(i % 5, (int)val1);
        }
    }
    
    *result = total;
    free(data);
    return total;
}

/* Main benchmark function combining all patterns */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
int main(void) {
    const int N = 512;
    const int M = 1024;
    double final_result = 0.0;
    
    printf("Starting selective scheduler trigger...\n");
    
    /* 1. Nested loops with dependencies */
    final_result += nested_dependency_loop(N);
    
    /* 2. SIMD processing */
    float* src_floats = malloc(M * sizeof(float));
    float* dst_floats = malloc(M * sizeof(float));
    
    for (int i = 0; i < M; ++i) {
        src_floats[i] = sin(i * 0.01f) * 100.0f;
    }
    
    #pragma GCC unroll 2
    for (int iter = 0; iter < 10; ++iter) {
        simd_intensive_processing(src_floats, dst_floats, M);
        /* Use result to prevent elimination */
        for (int i = 0; i < M; i += 64) {
            final_result += dst_floats[i];
        }
    }
    
    /* 3. Mixed data structure processing */
    double struct_result;
    final_result += process_mixed_data(N / 2, &struct_result);
    
    /* 4. Switch statement with dense and sparse cases */
    long switch_sum = 0;
    for (int i = 0; i < 1000; ++i) {
        switch (i % 20) {
            case 0:  switch_sum += i * 2; break;
            case 1:  switch_sum += i / 2; break;
            case 2:  switch_sum += i + 100; break;
            case 3:  switch_sum += i - 50; break;
            case 4:  switch_sum += i ^ 0xFF; break;
            case 5:  switch_sum += i << 2; break;
            case 6:  switch_sum += i >> 1; break;
            case 7:  switch_sum += ~i; break;
            case 8:  switch_sum += i % 13; break;
            case 9:  switch_sum += i * i; break;
            case 10: switch_sum += (int)sqrt(i); break;
            case 11: switch_sum += (int)log(i + 1); break;
            case 12: switch_sum += i | 0xAA; break;
            case 13: switch_sum += i & 0x55; break;
            case 14: switch_sum += i * 3; break;
            case 15: switch_sum += i / 3; break;
            case 16: switch_sum += i + 200; break;
            case 17: switch_sum += i - 100; break;
            case 18: switch_sum += i % 7; break;
            case 19: switch_sum += -i; break;
        }
        
        /* Inline assembly to create scheduling barriers */
        if (i % 17 == 0) {
            asm volatile ("# Scheduling barrier %0" : : "r"(i) : "rax", "rbx", "rcx", "rdx", "memory");
        }
    }
    final_result += switch_sum;
    
    /* Final checksum to prevent dead code elimination */
    printf("Final result: %f\n", final_result);
    
    free(src_floats);
    free(dst_floats);
    
    return (final_result > 0) ? 0 : 1;
}
