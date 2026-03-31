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
#define ITERATIONS 1000

/* Mixed data type structure for complex memory access patterns */
struct MixedData {
    int id;
    double value;
    char tag;
    float weight;
    long timestamp;
};

/* Helper function with varying arguments to create call instructions */
static double __attribute__((optimize("O2"))) compute_transform(double x, int scale, char mode) {
    if (mode == 's') {
        return sin(x * scale);
    } else if (mode == 'c') {
        return cos(x / (scale + 1));
    } else {
        return pow(x, 1.0 / (scale + 2));
    }
}

/* Function with loop-carried dependencies */
void __attribute__((optimize("O2"))) nested_dependency_test(int *result) {
    int acc = 0;
    
    /* Outer loop with varying trip count for inner loop */
    for (int i = 0; i < N; ++i) {
        int limit = (i % 32) + 8;  /* Varying inner loop bound */
        
        /* Inner loop with data-dependent operations */
        for (int j = 0; j < limit; ++j) {
            /* Mixed arithmetic operations */
            acc += (i * j) - (i / (j + 1));
            acc ^= (i << 3) | (j & 0xF);
            
            /* Conditional move operations */
            int temp = (j > i/2) ? acc * 2 : acc / 2;
            acc = (i % 3 == 0) ? temp : acc;
        }
        
        /* Inline assembly with clobbers to force scheduler constraints */
        asm volatile (
            "movl %0, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0"
            : "+r" (acc)
            : 
            : "eax", "memory"
        );
    }
    
    *result = acc;
}

/* Function with non-contiguous memory access and pointer arithmetic */
void __attribute__((optimize("O2"))) mixed_memory_access(struct MixedData *data, int size) {
    for (int i = 0; i < size; i += 2) {
        /* Non-contiguous access pattern */
        struct MixedData *curr = &data[i];
        struct MixedData *next = &data[i + 1];
        
        /* Pointer arithmetic and type mixing */
        curr->value = next->id * 0.5 + (curr->tag * 0.01);
        next->weight = (float)(curr->value * next->timestamp) / 1000.0f;
        
        /* Function call with varying arguments */
        double transformed = compute_transform(curr->value, i, i % 2 ? 's' : 'c');
        curr->value = transformed * (i % 10 + 1);
    }
}

/* SIMD processing using SSE/AVX intrinsics */
void __attribute__((optimize("O3"))) simd_processing(float *src, float *dst, int len) {
    #pragma GCC unroll 4
    for (int i = 0; i < len; i += 4) {
        /* Load unaligned data */
        __m128 a = _mm_loadu_ps(&src[i]);
        __m128 b = _mm_loadu_ps(&src[i + 4]);
        
        /* SIMD operations */
        __m128 sum = _mm_add_ps(a, b);
        __m128 prod = _mm_mul_ps(a, b);
        __m128 result = _mm_add_ps(sum, prod);
        
        /* Conditional select using comparison */
        __m128 mask = _mm_cmpgt_ps(a, b);
        result = _mm_or_ps(_mm_and_ps(mask, a), _mm_andnot_ps(mask, result));
        
        /* Store result */
        _mm_storeu_ps(&dst[i], result);
    }
}

/* Function with computed goto for indirect branching */
int __attribute__((optimize("O2"))) computed_goto_test(int selector) {
    static void* jump_table[] = {
        &&label_0, &&label_1, &&label_2, &&label_3,
        &&label_4, &&label_5, &&label_6, &&label_7
    };
    
    int result = 0;
    
    if (selector >= 0 && selector < 8) {
        goto *jump_table[selector];
    }
    
label_0:
    result = selector * 2;
    goto end;
label_1:
    result = selector + 100;
    goto end;
label_2:
    result = selector << 3;
    goto end;
label_3:
    result = selector | 0xFF;
    goto end;
label_4:
    result = selector / 2;
    goto end;
label_5:
    result = selector ^ 0x55;
    goto end;
label_6:
    result = selector * selector;
    goto end;
label_7:
    result = selector % 7;
    goto end;
    
end:
    return result;
}

/* Switch statement with mixed dense/sparse cases */
int __attribute__((optimize("O2"))) switch_pattern(int value) {
    int result = 0;
    
    switch (value) {
        /* Dense range */
        case 0:  result = value + 1; break;
        case 1:  result = value * 2; break;
        case 2:  result = value << 2; break;
        case 3:  result = value | 0xF0; break;
        case 4:  result = value / 2; break;
        
        /* Small gap */
        case 10: result = value - 5; break;
        case 11: result = value + 10; break;
        
        /* Large sparse gap */
        case 100: result = value % 13; break;
        case 200: result = value ^ 0xAA; break;
        case 300: result = value * value; break;
        
        /* Very large sparse value */
        case 1000: result = 1; break;
        
        default:
            /* Complex default computation */
            for (int i = 0; i < (value % 8); i++) {
                result += (value << i) & 0xFF;
            }
            break;
    }
    
    return result;
}

/* Main computational kernel with all patterns combined */
unsigned long __attribute__((optimize("O2"))) compute_kernel() {
    unsigned long checksum = 0;
    int temp_results[8];
    float src_array[N], dst_array[N];
    struct MixedData mixed_array[M];
    
    /* Initialize data with patterns */
    for (int i = 0; i < N; i++) {
        src_array[i] = (i * 1.5f) / (i + 1);
        dst_array[i] = 0.0f;
    }
    
    for (int i = 0; i < M; i++) {
        mixed_array[i].id = i;
        mixed_array[i].value = i * 0.25;
        mixed_array[i].tag = 'A' + (i % 26);
        mixed_array[i].weight = (i % 10) * 0.1f;
        mixed_array[i].timestamp = i * 1000L;
    }
    
    /* Execute various computation patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Pattern 1: Nested loops with dependencies */
        nested_dependency_test(&temp_results[0]);
        
        /* Pattern 2: Mixed memory access */
        mixed_memory_access(mixed_array, M);
        
        /* Pattern 3: SIMD processing */
        simd_processing(src_array, dst_array, N);
        
        /* Pattern 4: Computed goto */
        temp_results[1] = computed_goto_test(iter % 8);
        
        /* Pattern 5: Switch with mixed cases */
        temp_results[2] = switch_pattern(iter % 1001);
        
        /* Combine results into checksum */
        for (int i = 0; i < 3; i++) {
            checksum ^= (unsigned long)temp_results[i] << (i * 8);
        }
        
        /* Update source array for next iteration */
        for (int i = 0; i < N; i++) {
            src_array[i] = dst_array[i] * 0.9f + src_array[i] * 0.1f;
        }
        
        /* Another inline assembly to force scheduling complexity */
        asm volatile (
            "cpuid\n\t"
            : 
            : "a" (0)
            : "ebx", "ecx", "edx", "memory"
        );
    }
    
    return checksum;
}

int main() {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Force initialization of various data structures */
    unsigned long final_result = compute_kernel();
    
    /* Additional complex computation to ensure scheduler activity */
    int matrix[16][16];
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            /* Complex indexing pattern */
            matrix[i][j] = (i * j) + ((i + j) % 3 ? i : j);
            
            /* Conditional operation */
            matrix[i][j] = (matrix[i][j] > 100) ? 
                          matrix[i][j] / 2 : 
                          matrix[i][j] * 2;
        }
    }
    
    /* Final mixing of results */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            final_result += matrix[i][j];
            final_result = (final_result << 1) | (final_result >> 63);
        }
    }
    
    printf("Final checksum: %lu\n", final_result);
    printf("Program completed. Check compiler debug output for sel-sched-dump messages.\n");
    
    return 0;
}
