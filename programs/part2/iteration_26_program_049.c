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
static void complex_loop_carried_deps(int N, double* restrict out, const double* restrict in) {
    /* Nested loops with varying trip counts */
    for (int i = 0; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed types */
            acc += in[j] * (j % 2 ? 0.5 : 1.5);
            /* Conditional move operation */
            double temp = (j > i/2) ? acc * 0.3 : acc * 0.7;
            acc = temp + sin(in[j] * 0.01);
        }
        out[i] = acc;
        
        /* Inline assembly with clobbers to force scheduler work */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : : : "rax", "rbx", "rcx", "memory"
        );
    }
}

/* Mixed data structure for non-contiguous access */
struct MixedData {
    int a;
    double b;
    char c;
    float d;
    long e;
};

__attribute__((optimize("O3", "fsel-sched-pipelining", "funroll-loops")))
static void mixed_struct_operations(struct MixedData* arr, int size) {
    /* Non-contiguous memory access pattern */
    for (int i = 0; i < size; i += 2) {
        /* Pointer arithmetic with casting */
        struct MixedData* ptr1 = &arr[i];
        struct MixedData* ptr2 = &arr[i + 1];
        
        /* Mixed type operations */
        ptr1->b = ptr2->a * 0.5 + sin(ptr1->e);
        ptr2->d = (float)(ptr1->a % 256) * 0.1f;
        
        /* Complex conditional */
        ptr1->c = (ptr1->b > ptr2->d) ? 'A' : 'B';
        
        /* Function call with varying arguments */
        ptr1->e = (long)pow(fabs(ptr2->b), 2.0);
    }
}

/* SIMD operations with intrinsics */
__attribute__((target("sse2")))
static void simd_processing(float* restrict dst, const float* restrict src, int len) {
    #pragma GCC unroll 4
    for (int i = 0; i < len; i += 4) {
        /* Load unaligned data */
        __m128 vec = _mm_loadu_ps(&src[i]);
        
        /* SIMD operations */
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 scaled = _mm_mul_ps(squared, _mm_set1_ps(0.5f));
        
        /* Conditional-like operation using masks */
        __m128 mask = _mm_cmpgt_ps(vec, _mm_setzero_ps());
        __m128 result = _mm_or_ps(
            _mm_and_ps(mask, scaled),
            _mm_andnot_ps(mask, vec)
        );
        
        /* Store result */
        _mm_storeu_ps(&dst[i], result);
    }
}

/* Control flow with computed goto */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static int computed_goto_switch(int value) {
    static const void* jumptable[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_default
    };
    
    int idx = (value >= 0 && value <= 5) ? value : 6;
    goto *jumptable[idx];
    
case_0:
    return value * 2;
case_1:
    return value + 10;
case_2:
    return value / 2;
case_3:
    return value * value;
case_4:
    return value | 0xFF;
case_5:
    return value & 0x0F;
case_default:
    return -1;
}

/* Dense and sparse switch cases */
__attribute__((optimize("O2")))
static int mixed_switch(int x) {
    switch (x) {
        /* Dense range */
        case 0: return x + 1;
        case 1: return x * 2;
        case 2: return x << 1;
        case 3: return x >> 1;
        case 4: return x ^ 0x55;
        
        /* Sparse range */
        case 10: return x + 100;
        case 50: return x - 25;
        case 100: return x * 3;
        case 200: return x / 4;
        case 500: return x % 37;
        
        default:
            /* Complex default with loop */
            int sum = 0;
            for (int i = 0; i < (x % 10); i++) {
                sum += i * i;
            }
            return sum;
    }
}

/* Main computational kernel */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static double compute_kernel(int iterations) {
    const int N = 256;
    double* array1 = (double*)aligned_alloc(64, N * sizeof(double));
    double* array2 = (double*)aligned_alloc(64, N * sizeof(double));
    float* simd_src = (float*)aligned_alloc(16, N * sizeof(float));
    float* simd_dst = (float*)aligned_alloc(16, N * sizeof(float));
    struct MixedData* mixed = (struct MixedData*)aligned_alloc(64, N * sizeof(struct MixedData));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        array1[i] = sin(i * 0.1);
        simd_src[i] = (float)(i % 100) * 0.01f;
        mixed[i].a = i;
        mixed[i].b = i * 0.5;
        mixed[i].c = 'A' + (i % 26);
        mixed[i].d = (float)i * 0.1f;
        mixed[i].e = i * 100L;
    }
    
    double total = 0.0;
    
    /* Perform multiple iterations of mixed computations */
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary parameters to create different scheduling scenarios */
        int mod_iter = iter % 10;
        
        /* Call functions with complex patterns */
        complex_loop_carried_deps(N - mod_iter, array2, array1);
        
        mixed_struct_operations(mixed, N);
        
        simd_processing(simd_dst, simd_src, N);
        
        /* Use computed goto and switch in hot path */
        int switch_val = computed_goto_switch(iter % 7);
        int mixed_switch_val = mixed_switch(iter % 600);
        
        /* Combine results to prevent dead code elimination */
        double iter_sum = 0.0;
        for (int i = 0; i < N; i++) {
            iter_sum += array2[i] + mixed[i].b + simd_dst[i];
        }
        
        total += iter_sum * (switch_val + 1) * (mixed_switch_val + 1);
        
        /* Rotate data to create varying dependencies */
        double temp = array1[0];
        memmove(array1, array1 + 1, (N - 1) * sizeof(double));
        array1[N - 1] = temp;
    }
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(simd_src);
    free(simd_dst);
    free(mixed);
    
    return total;
}

int main(void) {
    /* Seed RNG for variability */
    srand(time(NULL));
    
    printf("Starting selective scheduler trigger program...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    double warmup = compute_kernel(10);
    
    /* Main computation with more iterations */
    printf("Main computation phase...\n");
    double result = compute_kernel(100);
    
    /* Additional phase with different patterns */
    printf("Additional patterns...\n");
    
    /* Create array for final processing */
    const int FINAL_SIZE = 1000;
    int* final_array = (int*)malloc(FINAL_SIZE * sizeof(int));
    
    /* Complex loop with data-dependent break */
    int sum = 0;
    for (int i = 0; i < FINAL_SIZE; i++) {
        final_array[i] = rand() % 1000;
        
        /* Early exit condition that's hard to predict */
        if (i > 10 && final_array[i] < final_array[i-1] && final_array[i-1] < final_array[i-2]) {
            /* Insert inline assembly barrier */
            asm volatile("" ::: "memory");
            break;
        }
        
        /* Mix of operations */
        sum += final_array[i] * ((i % 3 == 0) ? 2 : 1);
        sum ^= (final_array[i] << (i % 8));
        
        /* Periodic function call */
        if (i % 50 == 0) {
            sum += (int)sqrt(fabs((double)final_array[i]));
        }
    }
    
    /* Combine all results */
    result += warmup + sum;
    
    printf("Final checksum: %f\n", result);
    printf("Selective scheduler test completed.\n");
    
    free(final_array);
    
    return 0;
}
