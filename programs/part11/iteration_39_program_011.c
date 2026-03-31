/* test_sched_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (free_sched_block function in haifa-sched.cc lines 4681-4691).
 * It creates complex basic blocks requiring extensive instruction scheduling
 * with target hooks, frontend state saving, and large instruction queues.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Define vector types for SIMD operations to increase scheduling complexity */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Inline functions to increase instruction count in basic blocks */
static inline int complex_int_op(int a, int b, int c) {
    /* Multiple dependent operations creating scheduling pressure */
    int t1 = a * b + c;
    int t2 = (t1 >> 3) | (t1 << 29);
    int t3 = t2 ^ (b * 0x5A827999);
    return t3 * 2 - (a & c);
}

static inline float complex_float_op(float a, float b, float c) {
    /* Mixed FP operations using different execution units */
    float t1 = a * b + c;
    float t2 = sinf(t1) * cosf(b);
    float t3 = t2 / (1.0f + fabsf(c));
    return t3 * 2.0f - a;
}

/* Function with memory aliasing to prevent reordering */
static void memory_aliasing_ops(int* arr1, int* arr2, int size) {
    /* Potential aliasing creates scheduling barriers */
    for (int i = 1; i < size - 1; i++) {
        arr1[i] = arr1[i-1] + arr2[i+1];
        arr2[i] = arr1[i] * arr2[i-1];
        /* Add memory barrier-like effect */
        asm volatile("" ::: "memory");
    }
}

/* Function with speculative scheduling opportunities */
static int speculative_scheduling(int* data, int n, int threshold) {
    int sum = 0;
    int count = 0;
    
    /* Loop with conditional updates - may trigger frontend state saving */
    for (int i = 0; i < n; i++) {
        int val = data[i];
        
        /* Complex conditional chain */
        if (val > threshold) {
            sum += val * 2;
            count++;
        } else if (val < -threshold) {
            sum -= val * 3;
            count += 2;
        } else {
            sum += complex_int_op(val, i, threshold);
        }
        
        /* Additional dependent operations */
        data[i] = (sum >> 1) ^ 0xAAAAAAAA;
    }
    
    return sum / (count ? count : 1);
}

/* Wide basic block with unrolled loop - creates large instruction queue */
static void wide_basic_block(float* input, float* output, int size) {
    /* Unroll manually to create wide basic block */
    int i = 0;
    for (; i + 3 < size; i += 4) {
        /* Independent parallel chains to fill ready list */
        float a1 = complex_float_op(input[i], input[i+1], 1.234f);
        float a2 = complex_float_op(input[i+1], input[i+2], 2.345f);
        float a3 = complex_float_op(input[i+2], input[i+3], 3.456f);
        float a4 = complex_float_op(input[i+3], input[i], 4.567f);
        
        /* More dependent operations */
        output[i] = a1 * 0.5f + sinf(a2);
        output[i+1] = a2 * 0.7f + cosf(a3);
        output[i+2] = a3 * 0.3f + tanf(a4);
        output[i+3] = a4 * 0.9f + sqrtf(fabsf(a1));
        
        /* Integer operations mixed in */
        int idx = i & 0xFF;
        output[i] += complex_int_op(idx, i, (int)a1);
    }
    
    /* Remainder loop */
    for (; i < size; i++) {
        output[i] = complex_float_op(input[i], 1.0f, 0.0f);
    }
}

/* Function using SIMD vector operations */
static void simd_vector_ops(v4sf* vec_in, v4sf* vec_out, int count) {
    v4sf accum = {0.0f, 0.0f, 0.0f, 0.0f};
    v4sf multiplier = {1.1f, 2.2f, 3.3f, 4.4f};
    
    for (int i = 0; i < count; i++) {
        /* Multiple vector operations creating scheduling pressure */
        v4sf v1 = vec_in[i] * multiplier;
        v4sf v2 = __builtin_ia32_sqrtps(v1);  /* SSE intrinsic */
        v4sf v3 = v1 + v2;
        v4sf v4 = v3 * accum;
        
        vec_out[i] = v4;
        accum = accum + v1;
        
        /* Scalar operations mixed with vector ops */
        float f = ((float*)&accum)[0];
        ((float*)&accum)[0] = complex_float_op(f, 0.5f, 1.0f);
    }
}

/* Complex function with switch statement for state tracking */
static int complex_control_flow(int mode, int iterations) {
    int result = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with multiple cases - may require state saving */
        switch (mode) {
            case 0:
                result += complex_int_op(i, state, result);
                state = (state + 1) & 3;
                break;
            case 1:
                result -= complex_int_op(state, i, result);
                state = (state * 2) & 7;
                break;
            case 2:
                result ^= complex_int_op(result, i, state);
                state = (state << 1) | (result & 1);
                break;
            case 3:
                result *= complex_int_op(state, result, i);
                state = (state + result) & 0xF;
                break;
            default:
                result = complex_int_op(result, i, mode);
                state = 0;
        }
        
        /* Additional computation based on state */
        if (state & 1) {
            result += i * 2;
        } else {
            result -= i * 3;
        }
    }
    
    return result;
}

/* Main test driver with multiple scheduling scenarios */
int main(int argc, char** argv) {
    const int SIZE = 256;
    const int VEC_SIZE = 64;
    
    /* Allocate test data */
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    float* float_in = (float*)malloc(SIZE * sizeof(float));
    float* float_out = (float*)malloc(SIZE * sizeof(float));
    v4sf* vec_data = (v4sf*)malloc(VEC_SIZE * sizeof(v4sf));
    v4sf* vec_result = (v4sf*)malloc(VEC_SIZE * sizeof(v4sf));
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = rand() % 1000 - 500;
        float_in[i] = (rand() % 1000) / 100.0f - 5.0f;
    }
    
    for (int i = 0; i < VEC_SIZE; i++) {
        for (int j = 0; j < 4; j++) {
            ((float*)&vec_data[i])[j] = (rand() % 1000) / 100.0f;
        }
    }
    
    int checksum = 0;
    
    /* Test 1: Memory aliasing operations */
    printf("Test 1: Memory aliasing operations\n");
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    memcpy(arr2, int_data, SIZE * sizeof(int));
    memory_aliasing_ops(int_data, arr2, SIZE);
    
    for (int i = 0; i < SIZE; i++) {
        checksum ^= int_data[i];
    }
    free(arr2);
    
    /* Test 2: Speculative scheduling */
    printf("Test 2: Speculative scheduling\n");
    int spec_result = speculative_scheduling(int_data, SIZE, 250);
    checksum += spec_result;
    
    /* Test 3: Wide basic block with unrolled operations */
    printf("Test 3: Wide basic block operations\n");
    wide_basic_block(float_in, float_out, SIZE);
    
    for (int i = 0; i < SIZE; i++) {
        checksum += (int)(float_out[i] * 1000);
    }
    
    /* Test 4: SIMD vector operations */
    printf("Test 4: SIMD vector operations\n");
    simd_vector_ops(vec_data, vec_result, VEC_SIZE);
    
    for (int i = 0; i < VEC_SIZE; i++) {
        for (int j = 0; j < 4; j++) {
            checksum += (int)(((float*)&vec_result[i])[j] * 100);
        }
    }
    
    /* Test 5: Complex control flow with switch */
    printf("Test 5: Complex control flow\n");
    for (int mode = 0; mode < 4; mode++) {
        int cf_result = complex_control_flow(mode, 100);
        checksum += cf_result;
    }
    
    /* Test 6: Mixed integer/float with function calls */
    printf("Test 6: Mixed operations with function calls\n");
    float mixed_acc = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        int int_val = complex_int_op(int_data[i], i, checksum & 0xFF);
        float float_val = complex_float_op(float_in[i], mixed_acc, int_val);
        mixed_acc = mixed_acc * 0.9f + float_val * 0.1f;
        
        /* Call math functions that act as scheduling barriers */
        mixed_acc = sinf(mixed_acc) + cosf(float_val);
    }
    checksum += (int)(mixed_acc * 10000);
    
    /* Final verification */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(int_data);
    free(float_in);
    free(float_out);
    free(vec_data);
    free(vec_result);
    
    return 0;
}
