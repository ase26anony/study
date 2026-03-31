/* sel-sched-trigger.c
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug sel-sched-trigger.c -lm -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#pragma GCC optimize ("O2")
#pragma GCC optimize ("unroll-loops")

/* Mixed data structure for non-contiguous access */
struct MixedData {
    int id;
    double value;
    char tag;
    float temp;
    long counter;
};

/* Helper function with varying arguments */
static double __attribute__((noinline)) compute_transform(double x, int scale, char mode) {
    if (mode == 's') {
        return sin(x) * scale;
    } else if (mode == 'c') {
        return cos(x) / (scale + 1);
    } else {
        return pow(x, 1.5) + scale;
    }
}

/* Function with complex loop-carried dependencies */
__attribute__((optimize("O2"))) 
static void nested_dependency_test(int *arr, int n) {
    for (int i = 1; i < n; ++i) {
        /* Inner loop with varying trip count based on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with conditional moves */
            int val = (arr[j] > arr[i]) ? arr[j] - arr[i] : arr[i] - arr[j];
            arr[i] = (val % 2 == 0) ? val * 2 : val / 2;
            
            /* Inline assembly with clobbers to force scheduler work */
            asm volatile ("# Dependency barrier" : : : "memory", "eax", "ebx");
        }
    }
}

/* SIMD-intensive function */
__attribute__((optimize("O3")))
static void simd_processing(float *src, float *dst, int len) {
    for (int i = 0; i < len - 3; i += 4) {
        /* Load unaligned data */
        __m128 vec = _mm_loadu_ps(&src[i]);
        
        /* SIMD operations */
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 sqrted = _mm_sqrt_ps(squared);
        __m128 result = _mm_add_ps(vec, sqrted);
        
        /* Store back */
        _mm_storeu_ps(&dst[i], result);
        
        /* Conditional operation using ternary - may generate conditional moves */
        float check = src[i] + src[i+1] + src[i+2] + src[i+3];
        dst[i] = (check > 0.0f) ? dst[i] * 1.1f : dst[i] * 0.9f;
    }
}

/* Function with computed goto for complex control flow */
static int __attribute__((noinline)) jump_table_test(int index, int value) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_default
    };
    
    if (index < 0 || index > 5) index = 6;
    
    int result = value;
    goto *jump_table[index];
    
case_0:
    result = value * 2;
    goto end;
case_1:
    result = value + 100;
    goto end;
case_2:
    result = value / 3;
    goto end;
case_3:
    result = value ^ 0xFF;
    goto end;
case_4:
    result = value << 2;
    goto end;
case_5:
    result = value >> 1;
    goto end;
case_default:
    result = ~value;
    goto end;
    
end:
    return result;
}

/* Mixed data structure traversal */
static long process_mixed_array(struct MixedData *arr, int size) {
    long total = 0;
    
    /* Non-contiguous access pattern */
    for (int i = 0; i < size; i += 2) {
        /* Pointer arithmetic and casting */
        double *val_ptr = &arr[i].value;
        char *tag_ptr = &arr[i].tag;
        
        /* Data-dependent operations */
        arr[i].value = arr[i+1].id * 0.5;
        arr[i].temp = (float)sin(arr[i].value);
        
        /* Function call within loop */
        arr[i].value = compute_transform(arr[i].value, arr[i].id, *tag_ptr);
        
        /* Update total with conditional */
        total += (arr[i].id > 0) ? (long)arr[i].value : -(long)arr[i].value;
        
        /* Another inline assembly with clobber */
        asm volatile ("# Mixed data barrier" : : : "memory", "ecx", "edx");
    }
    
    return total;
}

/* Switch statement with dense and sparse cases */
static int sparse_switch_test(int x) {
    int result = 0;
    
    switch (x) {
        /* Dense range */
        case 0:  result = x + 1; break;
        case 1:  result = x * 2; break;
        case 2:  result = x << 3; break;
        case 3:  result = x ^ 0x0F; break;
        case 4:  result = x / 2; break;
        
        /* Small gap */
        case 10: result = x - 5; break;
        case 11: result = x + 10; break;
        
        /* Large sparse gap */
        case 100: result = x % 7; break;
        case 200: result = x & 0xFF; break;
        case 300: result = ~x; break;
        
        default: result = -x; break;
    }
    
    return result;
}

/* Main computational kernel */
__attribute__((optimize("O2")))
static unsigned long benchmark_kernel(int iterations) {
    unsigned long checksum = 0;
    
    /* Allocate and initialize arrays */
    int *int_array = (int*)malloc(iterations * sizeof(int));
    float *float_src = (float*)malloc(iterations * sizeof(float));
    float *float_dst = (float*)malloc(iterations * sizeof(float));
    struct MixedData *mixed = (struct MixedData*)malloc(iterations * sizeof(struct MixedData));
    
    if (!int_array || !float_src || !float_dst || !mixed) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with patterned data */
    for (int i = 0; i < iterations; i++) {
        int_array[i] = (i * 37) % 101;
        float_src[i] = sin(i * 0.1f);
        mixed[i].id = i;
        mixed[i].value = i * 0.5;
        mixed[i].tag = (i % 26) + 'A';
        mixed[i].temp = i * 0.1f;
        mixed[i].counter = i * 2L;
    }
    
    /* Execute all test patterns to create scheduling pressure */
    
    /* 1. Nested dependency test */
    nested_dependency_test(int_array, iterations / 2);
    
    /* 2. SIMD processing */
    simd_processing(float_src, float_dst, iterations);
    
    /* 3. Mixed data structure processing */
    checksum += process_mixed_array(mixed, iterations);
    
    /* 4. Jump table tests */
    for (int i = 0; i < 100; i++) {
        checksum += jump_table_test(i % 7, int_array[i % iterations]);
    }
    
    /* 5. Sparse switch tests */
    for (int i = 0; i < iterations; i += 7) {
        checksum += sparse_switch_test(int_array[i] % 350);
    }
    
    /* 6. Additional loop with function calls and math operations */
    for (int i = 1; i < iterations; i++) {
        double x = float_src[i];
        double y = compute_transform(x, i % 10, 's');
        double z = compute_transform(y, i % 5, 'c');
        
        /* Complex expression with multiple operations */
        float_dst[i] = (float)(sin(x) * cos(y) + tan(z * 0.1));
        
        /* Update checksum to prevent dead code elimination */
        checksum ^= *(unsigned long*)&float_dst[i];
        checksum += int_array[i % iterations];
    }
    
    /* Free memory */
    free(int_array);
    free(float_src);
    free(float_dst);
    free(mixed);
    
    return checksum;
}

int main() {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Run benchmark with different sizes to trigger various scheduling decisions */
    unsigned long final_checksum = 0;
    
    final_checksum ^= benchmark_kernel(1024);
    final_checksum ^= benchmark_kernel(2048);
    final_checksum ^= benchmark_kernel(4096);
    
    printf("Final checksum: %lu\n", final_checksum);
    printf("Program completed. Check stderr for selective scheduler debug output.\n");
    
    return 0;
}
