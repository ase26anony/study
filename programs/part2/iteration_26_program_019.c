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

#define N 256
#define M 128

/* Mixed data type structure for complex memory access */
struct MixedData {
    int id;
    double value;
    char tag;
    float weight;
    long timestamp;
};

/* Helper function with attribute to force specific optimization */
static double __attribute__((optimize("O2"))) compute_transform(double x, int iter) {
    /* Mix of operations that create varied RTL */
    if (iter & 1) {
        return sin(x) * cos(x);
    } else {
        return (x > 0.5) ? x * x : x * 0.5;  /* Conditional move pattern */
    }
}

/* Function with loop-carried dependencies */
static long __attribute__((optimize("O2"))) nested_dependency_test(int *data, int size) {
    long result = 0;
    
    /* Outer loop with varying trip count for inner loop */
    for (int i = 0; i < size; ++i) {
        int limit = (i % 16) + 1;  /* Varying inner loop count */
        
        /* Inner loop with data dependency */
        for (int j = 0; j < limit; ++j) {
            /* Complex dependency chain */
            data[j] = data[j] + (i * j) - (data[(j + 1) % limit] >> 2);
            result += data[j];
            
            /* Inline assembly with clobbers to force scheduler work */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : : : "eax", "ebx", "memory"
            );
        }
        
        /* Function call within loop */
        result += (long)compute_transform(data[i % limit] * 0.01, i);
    }
    
    return result;
}

/* SIMD processing function */
static void __attribute__((optimize("O3"))) simd_process(float *src, float *dst, int len) {
    /* Process with SIMD intrinsics */
    #pragma GCC unroll 4
    for (int i = 0; i < len - 3; i += 4) {
        __m128 vec = _mm_loadu_ps(&src[i]);
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 scaled = _mm_mul_ps(squared, _mm_set1_ps(0.5f));
        _mm_storeu_ps(&dst[i], scaled);
        
        /* Mix with scalar operations */
        dst[i] += (i % 2) ? 1.0f : -1.0f;  /* Conditional operator */
    }
    
    /* Handle remainder */
    for (int i = len - (len % 4); i < len; ++i) {
        dst[i] = src[i] * src[i] * 0.5f;
    }
}

/* Function with computed goto for complex control flow */
static int __attribute__((optimize("O2"))) jump_table_test(int idx, int val) {
    static void* jumptable[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&default_case
    };
    
    int result = val;
    
    if (idx >= 0 && idx < 6) {
        goto *jumptable[idx];
    } else {
        goto *jumptable[6];
    }
    
case_0:
    result = val * 2;
    goto end;
case_1:
    result = val + 37;
    goto end;
case_2:
    result = val >> 1;
    goto end;
case_3:
    result = val ^ 0xFF;
    goto end;
case_4:
    result = abs(val);
    goto end;
case_5:
    result = val % 17;
    goto end;
default_case:
    result = -val;
    goto end;
    
end:
    return result;
}

/* Matrix-style operation with mixed access patterns */
static double matrix_style_test(struct MixedData *arr, int count) {
    double total = 0.0;
    
    /* Non-contiguous memory access pattern */
    for (int i = 0; i < count; i += 2) {
        /* Pointer arithmetic with casting */
        double *val_ptr = &arr[i].value;
        float *weight_ptr = &arr[i + 1].weight;
        
        *val_ptr = *weight_ptr * 3.14159 + arr[i].id;
        total += *val_ptr;
        
        /* Another inline asm with different clobbers */
        asm volatile (
            "movl $0, %%eax\n\t"
            "addl $1, %%eax\n\t"
            : : : "eax", "cc"
        );
    }
    
    return total;
}

/* Switch statement with mixed dense/sparse cases */
static int switch_test(int value) {
    int result = 0;
    
    switch (value) {
        /* Dense range */
        case 0:  result = 1; break;
        case 1:  result = 2; break;
        case 2:  result = 3; break;
        case 3:  result = 5; break;
        case 4:  result = 8; break;
        case 5:  result = 13; break;
        
        /* Sparse range */
        case 10: result = 21; break;
        case 20: result = 34; break;
        case 30: result = 55; break;
        case 100: result = 89; break;
        
        default:
            /* Complex default computation */
            result = (value % 2) ? value * 3 : value / 2;
            result += jump_table_test(value % 7, result);
            break;
    }
    
    return result;
}

/* Main computational kernel */
static unsigned long __attribute__((optimize("O2"))) compute_kernel() {
    unsigned long checksum = 0;
    
    /* Initialize data arrays */
    int *int_data = (int*)malloc(N * sizeof(int));
    float *float_src = (float*)malloc(M * sizeof(float));
    float *float_dst = (float*)malloc(M * sizeof(float));
    struct MixedData *mixed_arr = (struct MixedData*)malloc(N * sizeof(struct MixedData));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; ++i) {
        int_data[i] = (i * 37) % 101;
        mixed_arr[i].id = i;
        mixed_arr[i].value = sin(i * 0.1);
        mixed_arr[i].tag = 'A' + (i % 26);
        mixed_arr[i].weight = (i % 10) * 0.1f;
        mixed_arr[i].timestamp = i * 1000L;
    }
    
    for (int i = 0; i < M; ++i) {
        float_src[i] = (i % 20) * 0.05f;
    }
    
    /* Execute all test patterns multiple times */
    for (int iter = 0; iter < 5; ++iter) {
        /* 1. Nested loops with dependencies */
        checksum ^= nested_dependency_test(int_data, N);
        
        /* 2. SIMD processing */
        simd_process(float_src, float_dst, M);
        for (int i = 0; i < M; i += 8) {
            checksum += (unsigned long)(float_dst[i] * 1000);
        }
        
        /* 3. Mixed data structure processing */
        double matrix_result = matrix_style_test(mixed_arr, N);
        checksum += (unsigned long)(matrix_result * 100);
        
        /* 4. Switch and jump table tests */
        for (int i = 0; i < 50; ++i) {
            checksum += switch_test(i % 105);
        }
        
        /* 5. Additional math library calls in loop */
        for (int i = 1; i < 20; ++i) {
            checksum += (unsigned long)(pow(2.0, i % 10) * 100);
            checksum += (unsigned long)(log(i + 1) * 1000);
        }
        
        /* Modify data for next iteration */
        for (int i = 0; i < N; ++i) {
            int_data[i] = (int_data[i] * 13 + 7) % 97;
        }
    }
    
    /* Cleanup */
    free(int_data);
    free(float_src);
    free(float_dst);
    free(mixed_arr);
    
    return checksum;
}

int main() {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Enable optimization for main as well */
    #pragma GCC optimize("O2")
    
    unsigned long final_result = compute_kernel();
    
    printf("Computation checksum: %lu\n", final_result);
    printf("Program completed.\n");
    
    return (final_result > 0) ? 0 : 1;
}
