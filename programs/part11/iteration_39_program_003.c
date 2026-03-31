/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that require:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving
 * 3. Large instruction queues and ready lists
 * 4. Complex instruction mixes with dependencies
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

/* Prevent excessive inlining to keep basic blocks large */
#define NOINLINE __attribute__((noinline))

/* Vector types for creating parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_accumulator = 0.0f;
volatile double global_double_acc = 0.0;

/* Complex arithmetic with mixed types and dependencies */
NOINLINE static float complex_arithmetic_chain(int a, int b, float c, float d) {
    /* Long dependency chain with mixed operations */
    int t1 = a * b + global_seed;
    float t2 = c * d + (float)t1;
    int t3 = t1 ^ (b << 3);
    float t4 = t2 / (c + 1.0f);
    double t5 = (double)t3 * (double)t4;
    int t6 = t3 & 0x7FFFFFFF;
    float t7 = t4 * sinf(t4);
    double t8 = t5 + cos(t5);
    float t9 = t7 + (float)t8;
    int t10 = t6 | (a & b);
    float result = t9 * (float)t10;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    return result + global_accumulator;
}

/* Function with SIMD operations for x86/ARM targets */
NOINLINE static v4sf simd_vector_math(v4sf a, v4sf b, v4sf c) {
    /* Multiple independent SIMD chains */
    v4sf r1 = a * b + c;
    v4sf r2 = b * c - a;
    v4sf r3 = a + b + c;
    v4sf r4 = r1 * r2 / r3;
    
    /* Cross-lane operations create dependencies */
    float sum1 = r1[0] + r1[1] + r1[2] + r1[3];
    float sum2 = r2[0] + r2[1] + r2[2] + r2[3];
    v4sf r5 = r4 * sum1 + sum2;
    
    /* Conditional update based on values */
    if (sum1 > sum2) {
        r5 = r5 * 2.0f;
    } else {
        r5 = r5 * 0.5f;
    }
    
    return r5 + a;
}

/* Function with loop unrolling creating wide basic block */
NOINLINE static double unrolled_loop_computation(double *array, int size) {
    double sum = 0.0;
    double prod = 1.0;
    double diff = 0.0;
    
    /* Unrolled loop with multiple independent chains */
    for (int i = 0; i < size; i += 4) {
        /* Chain 1: Summation with dependencies */
        double a1 = array[i] * 1.1;
        double b1 = a1 + sin(a1);
        double c1 = b1 * cos(b1);
        sum += c1;
        
        /* Chain 2: Product with different operations */
        double a2 = array[i+1] / 0.9;
        double b2 = a2 - tan(a2);
        double c2 = b2 * exp(-b2);
        prod *= (c2 + 1.0);
        
        /* Chain 3: Difference chain */
        double a3 = array[i+2] * 2.0;
        double b3 = a3 + log(fabs(a3) + 1.0);
        double c3 = b3 - sqrt(fabs(b3));
        diff -= c3;
        
        /* Chain 4: Mixed operations */
        double a4 = array[i+3] + 3.14;
        double b4 = sin(a4) * cos(a4);
        double c4 = tan(b4) / (fabs(b4) + 1.0);
        sum += c4 * diff;
        
        /* Cross-chain dependencies */
        prod *= (sum + 1.0);
        diff += prod * 0.01;
    }
    
    /* Final complex expression */
    return sum * prod - diff * global_double_acc;
}

/* Function with conditional branches for speculative scheduling */
NOINLINE static int speculative_scheduling_test(int *data, int n) {
    int result = 0;
    int temp1 = 0, temp2 = 0, temp3 = 0;
    
    /* Multiple conditionals in a basic block */
    for (int i = 0; i < n; i++) {
        int x = data[i];
        
        /* Chain of dependent conditionals */
        if (x & 1) {
            temp1 = x * 3 + 7;
        } else {
            temp1 = x / 2 - 5;
        }
        
        if (temp1 > 100) {
            temp2 = temp1 << 2;
        } else if (temp1 < -100) {
            temp2 = temp1 >> 2;
        } else {
            temp2 = temp1 ^ 0xABCD;
        }
        
        if (temp2 % 3 == 0) {
            temp3 = temp2 * temp1;
        } else if (temp2 % 3 == 1) {
            temp3 = temp2 + temp1;
        } else {
            temp3 = temp2 - temp1;
        }
        
        /* Memory operation with potential aliasing */
        data[i] = temp3;
        result += temp3;
        
        /* Function call with side effects */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result;
}

/* Function with mixed integer/FP and memory operations */
NOINLINE static void mixed_operations_matrix(float *mat1, float *mat2, float *result, int n) {
    /* Manual unrolling for wide basic block */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j += 4) {
            /* Four independent dot product calculations */
            float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f, sum4 = 0.0f;
            
            for (int k = 0; k < n; k++) {
                /* Multiple loads and FP operations */
                float a1 = mat1[i * n + k];
                float b1 = mat2[k * n + j];
                sum1 += a1 * b1;
                
                float a2 = mat1[i * n + k];
                float b2 = mat2[k * n + j + 1];
                sum2 += a2 * b2;
                
                float a3 = mat1[i * n + k];
                float b3 = mat2[k * n + j + 2];
                sum3 += a3 * b3;
                
                float a4 = mat1[i * n + k];
                float b4 = mat2[k * n + j + 3];
                sum4 += a4 * b4;
            }
            
            /* Store results with integer index calculations */
            result[i * n + j] = sum1 * global_accumulator;
            result[i * n + j + 1] = sum2 + (float)global_seed;
            result[i * n + j + 2] = sum3 - sqrtf(fabsf(sum3));
            result[i * n + j + 3] = sum4 * cosf(sum4);
        }
    }
}

/* Main test driver with multiple complex functions */
int main() {
    const int SIZE = 256;
    const int MAT_SIZE = 32;
    
    /* Allocate and initialize test data */
    double *array = (double*)malloc(SIZE * sizeof(double));
    int *int_data = (int*)malloc(SIZE * sizeof(int));
    float *mat1 = (float*)malloc(MAT_SIZE * MAT_SIZE * sizeof(float));
    float *mat2 = (float*)malloc(MAT_SIZE * MAT_SIZE * sizeof(float));
    float *mat_result = (float*)malloc(MAT_SIZE * MAT_SIZE * sizeof(float));
    
    /* Initialize with pseudo-random values */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
        int_data[i] = rand() % 1000 - 500;
    }
    
    for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
        mat1[i] = (float)rand() / RAND_MAX * 10.0f;
        mat2[i] = (float)rand() / RAND_MAX * 10.0f;
    }
    
    /* Initialize SIMD vectors */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    
    printf("Starting scheduler coverage test...\n");
    
    /* Test 1: Complex arithmetic chain */
    float test1_result = 0.0f;
    for (int i = 0; i < 100; i++) {
        test1_result += complex_arithmetic_chain(i, i*2, i*0.5f, i*0.25f);
    }
    printf("Test 1 result: %f\n", test1_result);
    
    /* Test 2: SIMD vector math */
    v4sf test2_result = {0.0f, 0.0f, 0.0f, 0.0f};
    for (int i = 0; i < 50; i++) {
        test2_result += simd_vector_math(vec1, vec2, vec3);
        vec1 += 0.1f;
        vec2 += 0.2f;
        vec3 += 0.3f;
    }
    printf("Test 2 result: %f %f %f %f\n", 
           test2_result[0], test2_result[1], test2_result[2], test2_result[3]);
    
    /* Test 3: Unrolled loop computation */
    double test3_result = unrolled_loop_computation(array, SIZE);
    printf("Test 3 result: %lf\n", test3_result);
    
    /* Test 4: Speculative scheduling */
    int test4_result = speculative_scheduling_test(int_data, SIZE);
    printf("Test 4 result: %d\n", test4_result);
    
    /* Test 5: Mixed operations matrix */
    clock_t start = clock();
    mixed_operations_matrix(mat1, mat2, mat_result, MAT_SIZE);
    clock_t end = clock();
    
    /* Compute checksum */
    float checksum = 0.0f;
    for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
        checksum += mat_result[i];
    }
    printf("Test 5 checksum: %f, time: %ld ms\n", 
           checksum, (end - start) * 1000 / CLOCKS_PER_SEC);
    
    /* Free allocated memory */
    free(array);
    free(int_data);
    free(mat1);
    free(mat2);
    free(mat_result);
    
    printf("All tests completed.\n");
    
    /* Return meaningful result to prevent dead code elimination */
    return (int)(test1_result + test3_result + test4_result + checksum) & 0xFF;
}
