/* test_sched_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * basic blocks that require extensive instruction scheduling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for SIMD-like operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex arithmetic chains to create dependency chains */
static inline ALWAYS_INLINE float complex_float_chain(float a, float b, float c, float d) {
    /* Long dependency chain with mixed operations */
    float t1 = a + b;
    float t2 = t1 * c;
    float t3 = t2 - d;
    float t4 = t3 / (a + 1.0f);
    float t5 = t4 * t4;
    float t6 = sqrtf(fabsf(t5));
    float t7 = t6 + sinf(t3);
    float t8 = cosf(t7) * expf(t4);
    return t8;
}

static inline ALWAYS_INLINE int complex_int_chain(int a, int b, int c, int d) {
    /* Integer dependency chain with bit operations */
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 ^ d;
    int t4 = t3 << (a & 3);
    int t5 = t4 >> (b & 7);
    int t6 = t5 | (c << 8);
    int t7 = t6 & 0xFFFF;
    int t8 = t7 * t3;
    int t9 = t8 % (d + 1);
    return t9;
}

/* Function with memory aliasing to prevent reordering */
static inline ALWAYS_INLINE void memory_aliasing_ops(int* arr1, int* arr2, int size) {
    for (int i = 1; i < size - 1; i++) {
        /* Potential aliasing between arr1 and arr2 */
        arr1[i] = arr1[i-1] + arr2[i];
        arr2[i] = arr1[i] * arr2[i+1];
        arr1[i+1] = arr2[i] - arr1[i];
    }
}

/* Function with speculative scheduling opportunities */
static ALWAYS_INLINE int speculative_operations(int x, int y, int* data, int n) {
    int result = 0;
    
    /* Multiple independent chains that could be scheduled speculatively */
    int chain1 = complex_int_chain(x, y, data[0], data[1]);
    int chain2 = complex_int_chain(y, x, data[2], data[3]);
    float chain3 = complex_float_chain(x, y, data[4], data[5]);
    
    /* Conditional that might trigger speculative scheduling */
    if (chain1 > chain2) {
        result = chain1 * (int)chain3;
    } else {
        result = chain2 + (int)chain3;
    }
    
    /* More operations after the branch */
    for (int i = 0; i < 4; i++) {
        result += complex_int_chain(result, data[i], i, n);
    }
    
    return result;
}

/* Wide basic block with unrolled loop - creates large instruction queue */
void wide_basic_block(int* input, int* output, int size) {
    /* Unroll manually to create wide block */
    for (int i = 0; i < size; i += 8) {
        /* Eight independent chains - fills ready list */
        output[i]   = complex_int_chain(input[i],   input[i+1], i, size);
        output[i+1] = complex_int_chain(input[i+1], input[i+2], i+1, size);
        output[i+2] = complex_int_chain(input[i+2], input[i+3], i+2, size);
        output[i+3] = complex_int_chain(input[i+3], input[i+4], i+3, size);
        output[i+4] = complex_int_chain(input[i+4], input[i+5], i+4, size);
        output[i+5] = complex_int_chain(input[i+5], input[i+6], i+5, size);
        output[i+6] = complex_int_chain(input[i+6], input[i+7], i+6, size);
        output[i+7] = complex_int_chain(input[i+7], input[i],   i+7, size);
        
        /* Memory operations with potential aliasing */
        if (i > 0) {
            output[i] += output[i-1];
        }
    }
}

/* Function with mixed operations and SIMD-like vector ops */
void mixed_vector_operations(float* a, float* b, float* c, int n) {
    /* Mix of scalar and vector-like operations */
    for (int i = 0; i < n; i++) {
        /* Scalar FP operations */
        float t1 = complex_float_chain(a[i], b[i], c[i], i);
        
        /* Integer operations interleaved */
        int it1 = complex_int_chain((int)a[i], (int)b[i], i, n);
        
        /* More FP operations */
        float t2 = sinf(t1) * cosf(b[i]);
        float t3 = expf(t2) / (fabsf(c[i]) + 1.0f);
        
        /* Conditional with side effects */
        if (t3 > 0.5f) {
            a[i] = t1 * t2 + t3;
            b[i] = it1 * t3;
        } else {
            a[i] = t1 - t2 * t3;
            b[i] = it1 / (float)(n + 1);
        }
        
        /* Memory operation */
        c[i] = a[i] + b[i];
    }
}

/* Complex function with nested loops and conditionals */
int complex_scheduling_pattern(int seed, int iterations) {
    int buffer[64];
    int temp[64];
    int result = seed;
    
    /* Initialize buffers */
    for (int i = 0; i < 64; i++) {
        buffer[i] = (i * seed) & 0xFF;
        temp[i] = 0;
    }
    
    /* Outer loop with small iteration count - might trigger software pipelining */
    for (int iter = 0; iter < iterations; iter++) {
        /* Inner unrolled operations */
        for (int i = 0; i < 64; i += 4) {
            /* Four parallel dependency chains */
            int r1 = speculative_operations(buffer[i], buffer[i+1], buffer, 64);
            int r2 = speculative_operations(buffer[i+1], buffer[i+2], buffer, 64);
            int r3 = speculative_operations(buffer[i+2], buffer[i+3], buffer, 64);
            int r4 = speculative_operations(buffer[i+3], buffer[i], buffer, 64);
            
            /* Cross-dependent operations */
            temp[i]   = r1 ^ r2;
            temp[i+1] = r2 | r3;
            temp[i+2] = r3 & r4;
            temp[i+3] = r4 + r1;
        }
        
        /* Memory aliasing operations */
        memory_aliasing_ops(buffer, temp, 64);
        
        /* Update result with complex chain */
        result = complex_int_chain(result, temp[iter & 63], iter, iterations);
    }
    
    return result;
}

/* Function with inline assembly to create scheduling barriers */
void asm_barrier_operations(int* data, int n) {
    for (int i = 0; i < n; i++) {
        int val = data[i];
        
        /* Inline assembly acts as scheduling barrier */
        __asm__ volatile (
            "addl $1, %0\n\t"
            "rorl $3, %0\n\t"
            : "+r" (val)
            :
            : "cc"
        );
        
        /* More operations after barrier */
        val = complex_int_chain(val, i, n, data[(i+1)%n]);
        
        /* Another barrier */
        __asm__ volatile (
            "bsfl %0, %0\n\t"
            : "+r" (val)
            :
            : "cc"
        );
        
        data[i] = val;
    }
}

/* Main test driver */
int main() {
    const int SIZE = 1024;
    const int ITERATIONS = 100;
    
    /* Allocate test arrays */
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    float* float_data_a = (float*)malloc(SIZE * sizeof(float));
    float* float_data_b = (float*)malloc(SIZE * sizeof(float));
    float* float_data_c = (float*)malloc(SIZE * sizeof(float));
    int* output = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data_a[i] = (float)(rand() % 1000) / 100.0f;
        float_data_b[i] = (float)(rand() % 1000) / 100.0f;
        float_data_c[i] = (float)(rand() % 1000) / 100.0f;
    }
    
    int checksum = 0;
    
    /* Test 1: Wide basic block with unrolled operations */
    printf("Running wide basic block test...\n");
    wide_basic_block(int_data, output, SIZE);
    for (int i = 0; i < SIZE; i++) {
        checksum ^= output[i];
    }
    
    /* Test 2: Mixed vector operations */
    printf("Running mixed vector operations test...\n");
    mixed_vector_operations(float_data_a, float_data_b, float_data_c, SIZE);
    for (int i = 0; i < SIZE; i++) {
        checksum ^= (int)(float_data_a[i] * 1000);
        checksum ^= (int)(float_data_b[i] * 1000);
    }
    
    /* Test 3: Complex scheduling pattern */
    printf("Running complex scheduling pattern test...\n");
    int pattern_result = complex_scheduling_pattern(checksum, ITERATIONS);
    checksum ^= pattern_result;
    
    /* Test 4: Assembly barrier operations */
    printf("Running assembly barrier operations test...\n");
    asm_barrier_operations(int_data, SIZE);
    for (int i = 0; i < SIZE; i += 16) {
        checksum += int_data[i];
    }
    
    /* Test 5: Multiple calls to create different scheduling contexts */
    printf("Running multiple scheduling context test...\n");
    for (int i = 0; i < 10; i++) {
        int small_data[32];
        for (int j = 0; j < 32; j++) small_data[j] = (i * j) & 0xFF;
        
        int small_output[32];
        wide_basic_block(small_data, small_output, 32);
        
        for (int j = 0; j < 32; j++) {
            checksum += speculative_operations(small_output[j], j, small_data, 32);
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(int_data);
    free(float_data_a);
    free(float_data_b);
    free(float_data_c);
    free(output);
    
    return 0;
}
