/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that require extensive instruction scheduling.
 * 
 * Compilation options for coverage:
 * x86: gcc -O3 -fschedule-insns -fschedule-insns2 -mtune=generic -march=x86-64 -fno-omit-frame-pointer -msse4.2 test_scheduler_coverage.c -o test
 * ARM: gcc -O2 -fschedule-insns -fschedule-insns2 -mcpu=cortex-a57 -mfpu=neon test_scheduler_coverage.c -o test_arm
 * Aggressive: gcc -O3 -funroll-loops -fschedule-insns -fschedule-insns2 -march=native -fno-vect-cost-model test_scheduler_coverage.c -o test_max
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

/* Vector types for SIMD operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Inline functions to increase instruction count */
static inline int complex_int_op(int a, int b, int c) {
    return (a * b) + (c << 3) - (a / (b + 1)) + (a & b) | (c ^ a);
}

static inline float complex_float_op(float a, float b, float c) {
    return (a * b) + (c / (a + 1.0f)) - sinf(b) * cosf(c) + sqrtf(fabsf(a - b));
}

/* Function with complex dependency chains - triggers state saving */
int test_dependency_chains(int n, int* results) {
    int sum = 0;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Multiple dependent chains with branches */
    for (int i = 0; i < n; i++) {
        /* Chain 1: Integer dependencies */
        int a = i * 3;
        int b = a + i;
        int c = b * 2;
        int d = c - a;
        int e = d / (i + 1);
        int f = e | b;
        int g = f & c;
        
        /* Chain 2: Floating point dependencies */
        float fa = (float)i * 1.5f;
        float fb = fa + sinf(fa);
        float fc = fb * cosf(fa);
        float fd = fc / (fa + 1.0f);
        float fe = sqrtf(fabsf(fd));
        
        /* Chain 3: Mixed operations with memory */
        results[i] = g + (int)fe;
        
        /* Conditional that may trigger speculative scheduling */
        if (i % 7 == 0) {
            results[i] *= 2;
            barrier = results[i]; /* Memory barrier effect */
        } else if (i % 13 == 0) {
            results[i] /= 3;
            barrier = results[i];
        }
        
        /* More dependencies */
        int h = results[i] + barrier;
        int j = h * i;
        int k = j - results[i];
        
        sum += k;
        
        /* Small inner loop for software pipelining attempts */
        for (int j = 0; j < 4; j++) {
            sum += complex_int_op(i, j, k);
        }
    }
    
    return sum;
}

/* Wide basic block with many independent operations - fills instruction queue */
float test_wide_block(float* array, int size) {
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f, sum4 = 0.0f;
    float prod1 = 1.0f, prod2 = 1.0f, prod3 = 1.0f, prod4 = 1.0f;
    
    /* Unrolled loop creates wide basic block */
    for (int i = 0; i < size; i += 8) {
        /* Independent parallel chains - fills ready list */
        float a1 = array[i] * 1.1f;
        float a2 = array[i+1] * 2.2f;
        float a3 = array[i+2] * 3.3f;
        float a4 = array[i+3] * 4.4f;
        float a5 = array[i+4] * 5.5f;
        float a6 = array[i+5] * 6.6f;
        float a7 = array[i+6] * 7.7f;
        float a8 = array[i+7] * 8.8f;
        
        /* More independent operations */
        float b1 = sinf(a1) + cosf(a2);
        float b2 = sinf(a3) + cosf(a4);
        float b3 = sinf(a5) + cosf(a6);
        float b4 = sinf(a7) + cosf(a8);
        
        float c1 = sqrtf(fabsf(b1));
        float c2 = sqrtf(fabsf(b2));
        float c3 = sqrtf(fabsf(b3));
        float c4 = sqrtf(fabsf(b4));
        
        /* Accumulate results */
        sum1 += a1 + b1 + c1;
        sum2 += a2 + b2 + c2;
        sum3 += a3 + b3 + c3;
        sum4 += a4 + b4 + c4;
        
        prod1 *= (a1 + 1.0f);
        prod2 *= (a2 + 1.0f);
        prod3 *= (a3 + 1.0f);
        prod4 *= (a4 + 1.0f);
        
        /* Call inline functions to increase instruction count */
        sum1 += complex_float_op(a1, b1, c1);
        sum2 += complex_float_op(a2, b2, c2);
        sum3 += complex_float_op(a3, b3, c3);
        sum4 += complex_float_op(a4, b4, c4);
    }
    
    return (sum1 + sum2 + sum3 + sum4) * (prod1 + prod2 + prod3 + prod4);
}

/* Function with SIMD operations - triggers target-specific scheduling hooks */
#ifdef __SSE4_2__
void test_simd_operations(v4si* vec_int, v4sf* vec_float, int count) {
    v4si accum_int = {0, 0, 0, 0};
    v4sf accum_float = {0.0f, 0.0f, 0.0f, 0.0f};
    
    for (int i = 0; i < count; i++) {
        /* SIMD integer operations */
        v4si v1 = vec_int[i];
        v4si v2 = vec_int[(i + 1) % count];
        v4si v3 = vec_int[(i + 2) % count];
        
        v4si r1 = v1 * v2 + v3;
        v4si r2 = v1 & v2 | v3;
        v4si r3 = v1 << 2;
        v4si r4 = v2 >> 1;
        
        accum_int += r1 + r2 - r3 * r4;
        
        /* SIMD floating point operations */
        v4sf f1 = vec_float[i];
        v4sf f2 = vec_float[(i + 1) % count];
        
        /* Complex SIMD expression - many dependent operations */
        v4sf temp1 = f1 * f2;
        v4sf temp2 = temp1 + f1;
        v4sf temp3 = temp2 - f2;
        v4sf temp4 = __builtin_ia32_sqrtps(temp3);
        
        accum_float += temp1 * temp2 + temp3 / temp4;
        
        /* Conditional store - creates scheduling barrier */
        if (i % 5 == 0) {
            vec_int[i] = accum_int;
            vec_float[i] = accum_float;
        }
    }
    
    /* Final reduction */
    int final_int = accum_int[0] + accum_int[1] + accum_int[2] + accum_int[3];
    float final_float = accum_float[0] + accum_float[1] + accum_float[2] + accum_float[3];
    
    /* Use results to prevent optimization */
    vec_int[0][0] = final_int;
    vec_float[0][0] = final_float;
}
#endif

/* Function with computed goto - triggers frontend state saving */
int test_computed_goto(int mode, int iterations) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_6, &&case_7
    };
    
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int op = (i + mode) % 8;
        
        /* Computed goto - complex control flow */
        goto *jump_table[op];
        
    case_0:
        result += i * 2;
        result = complex_int_op(result, i, mode);
        continue;
        
    case_1:
        result -= i * 3;
        result |= (i << 4);
        continue;
        
    case_2:
        result *= (i + 1);
        result = result ^ (i * 0x5A5A);
        continue;
        
    case_3:
        result /= (i % 7 + 1);
        result += complex_int_op(result, i, mode);
        continue;
        
    case_4:
        result = (result << 3) | (i & 0xFF);
        result -= complex_int_op(i, mode, result);
        continue;
        
    case_5:
        result = result ^ (i * 0x3333);
        result += (i % 13) * 2;
        continue;
        
    case_6:
        result = (result >> 2) + (i * 7);
        result = complex_int_op(result, result, i);
        continue;
        
    case_7:
        result = result & 0xFFFF;
        result += complex_int_op(mode, i, result);
        continue;
    }
    
    return result;
}

/* Matrix operations - creates complex scheduling scenarios */
void test_matrix_operations(float* matrix_a, float* matrix_b, float* result, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float sum = 0.0f;
            
            /* Unrolled inner loop for wide basic block */
            for (int k = 0; k < size; k += 4) {
                /* Multiple independent loads and operations */
                float a1 = matrix_a[i * size + k];
                float b1 = matrix_b[k * size + j];
                float a2 = matrix_a[i * size + k + 1];
                float b2 = matrix_b[(k + 1) * size + j];
                float a3 = matrix_a[i * size + k + 2];
                float b3 = matrix_b[(k + 2) * size + j];
                float a4 = matrix_a[i * size + k + 3];
                float b4 = matrix_b[(k + 3) * size + j];
                
                /* Dependent operations */
                float p1 = a1 * b1;
                float p2 = a2 * b2;
                float p3 = a3 * b3;
                float p4 = a4 * b4;
                
                /* More complex operations */
                p1 = p1 + sinf(p1) * 0.1f;
                p2 = p2 + cosf(p2) * 0.2f;
                p3 = p3 + sqrtf(fabsf(p3)) * 0.3f;
                p4 = p4 + complex_float_op(p4, a4, b4);
                
                sum += p1 + p2 + p3 + p4;
            }
            
            result[i * size + j] = sum;
            
            /* Conditional that may trigger speculative scheduling */
            if ((i + j) % 11 == 0) {
                result[i * size + j] *= 1.5f;
            }
        }
    }
}

/* Main driver function */
int main() {
    const int ARRAY_SIZE = 256;
    const int MATRIX_SIZE = 32;
    
    /* Allocate and initialize test data */
    int* int_results = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float* matrix_a = (float*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float));
    float* matrix_b = (float*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float));
    float* matrix_result = (float*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float));
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_results[i] = i;
        float_array[i] = (float)i * 0.1f;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix_a[i] = (float)(i % 100) * 0.01f;
        matrix_b[i] = (float)((i + 7) % 100) * 0.02f;
    }
    
    printf("Starting scheduler coverage tests...\n");
    
    /* Test 1: Complex dependency chains */
    printf("Test 1: Dependency chains... ");
    int sum1 = test_dependency_chains(ARRAY_SIZE, int_results);
    printf("sum = %d\n", sum1);
    
    /* Test 2: Wide basic blocks */
    printf("Test 2: Wide basic blocks... ");
    float result2 = test_wide_block(float_array, ARRAY_SIZE);
    printf("result = %f\n", result2);
    
    /* Test 3: SIMD operations (if available) */
#ifdef __SSE4_2__
    printf("Test 3: SIMD operations... ");
    v4si* vec_int = (v4si*)malloc(64 * sizeof(v4si));
    v4sf* vec_float = (v4sf*)malloc(64 * sizeof(v4sf));
    
    for (int i = 0; i < 64; i++) {
        vec_int[i] = (v4si){i, i+1, i+2, i+3};
        vec_float[i] = (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
    }
    
    test_simd_operations(vec_int, vec_float, 64);
    printf("completed\n");
    
    free(vec_int);
    free(vec_float);
#endif
    
    /* Test 4: Computed goto */
    printf("Test 4: Computed goto... ");
    int result4 = test_computed_goto(3, 1000);
    printf("result = %d\n", result4);
    
    /* Test 5: Matrix operations */
    printf("Test 5: Matrix operations... ");
    test_matrix_operations(matrix_a, matrix_b, matrix_result, MATRIX_SIZE);
    
    /* Compute checksum to prevent optimization */
    float checksum = 0.0f;
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        checksum += matrix_result[i];
    }
    printf("checksum = %f\n", checksum);
    
    /* Cleanup */
    free(int_results);
    free(float_array);
    free(matrix_a);
    free(matrix_b);
    free(matrix_result);
    
    printf("All tests completed successfully.\n");
    
    /* Return meaningful value based on computations */
    return (sum1 + (int)result2 + result4 + (int)checksum) != 0 ? 0 : 1;
}
