/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that force the scheduler to allocate and use the full
 * scheduling context, including target-specific hooks and frontend state.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for creating parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic operations to create dependency chains */
static inline ALWAYS_INLINE double complex_math_chain(double a, double b, double c, double d) {
    double t1 = a * b + c;
    double t2 = t1 / (d + 1.0);
    double t3 = sin(t2) * cos(t1);
    double t4 = t3 * t3 + t2 * t2;
    double t5 = sqrt(fabs(t4)) + log(fabs(t3) + 1.0);
    return t5;
}

/* Integer operations with dependencies */
static inline ALWAYS_INLINE int integer_chain(int a, int b, int c, int d) {
    int t1 = a * b + c;
    int t2 = t1 ^ (d << 3);
    int t3 = t2 * 0x5A827999;
    int t4 = (t3 >> 16) | (t3 << 16);
    int t5 = t4 + a * c - b * d;
    return t5;
}

/* Memory operations with potential aliasing */
static inline ALWAYS_INLINE void memory_ops(int* arr1, int* arr2, int* arr3, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = arr2[i] * arr3[i];
        arr3[i] = arr1[i] + arr2[i];
        arr2[i] = arr3[i] - arr1[i];
    }
}

/* Function with mixed operations and control flow */
static ALWAYS_INLINE double mixed_operations_block(double a, double b, int* counter) {
    double result = 0.0;
    
    /* Create a complex dependency chain */
    double x = a;
    double y = b;
    
    for (int i = 0; i < 8; i++) {  /* Small loop for potential software pipelining */
        x = x * y + sin(x);
        y = y * x - cos(y);
        result += x - y;
        
        /* Conditional that might create speculative scheduling */
        if (x > y) {
            result *= 1.1;
            (*counter)++;
        } else {
            result *= 0.9;
        }
        
        /* More arithmetic */
        x = fmod(x, 100.0) + 1.0;
        y = fmod(y, 100.0) + 1.0;
    }
    
    return result;
}

/* Function with SIMD-like operations using vector extensions */
static void vector_operations(v4si* vec1, v4si* vec2, v4si* vec3, int iterations) {
    v4si a = *vec1;
    v4si b = *vec2;
    v4si c = *vec3;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple independent vector operations */
        a = a + b * c;
        b = b - a;
        c = c * a + b;
        a = a << 2;
        b = b >> 1;
        c = c ^ a;
        
        /* Create some dependencies */
        v4si temp = a + b;
        a = temp * c;
        b = c - temp;
        c = a & b;
    }
    
    *vec1 = a;
    *vec2 = b;
    *vec3 = c;
}

/* Wide basic block with many independent computations */
static double wide_basic_block(double input) {
    double a1 = input * 1.1;
    double a2 = input * 1.2;
    double a3 = input * 1.3;
    double a4 = input * 1.4;
    double a5 = input * 1.5;
    
    /* Many independent chains */
    double b1 = sin(a1) * cos(a1);
    double b2 = sin(a2) * cos(a2);
    double b3 = sin(a3) * cos(a3);
    double b4 = sin(a4) * cos(a4);
    double b5 = sin(a5) * cos(a5);
    
    double c1 = sqrt(fabs(b1)) + log(fabs(b1) + 1.0);
    double c2 = sqrt(fabs(b2)) + log(fabs(b2) + 1.0);
    double c3 = sqrt(fabs(b3)) + log(fabs(b3) + 1.0);
    double c4 = sqrt(fabs(b4)) + log(fabs(b4) + 1.0);
    double c5 = sqrt(fabs(b5)) + log(fabs(b5) + 1.0);
    
    double d1 = c1 * c2 - c3;
    double d2 = c2 * c3 - c4;
    double d3 = c3 * c4 - c5;
    double d4 = c4 * c5 - c1;
    double d5 = c5 * c1 - c2;
    
    double e1 = d1 + d2 + d3;
    double e2 = d2 + d3 + d4;
    double e3 = d3 + d4 + d5;
    double e4 = d4 + d5 + d1;
    double e5 = d5 + d1 + d2;
    
    return e1 + e2 + e3 + e4 + e5;
}

/* Function with switch statement for complex control flow */
static int switch_based_computation(int value, int* array, int size) {
    int result = 0;
    
    switch (value % 8) {
        case 0:
            for (int i = 0; i < size; i++) {
                array[i] = i * value;
                result += array[i];
            }
            break;
        case 1:
            for (int i = 0; i < size; i++) {
                array[i] = i + value;
                result += array[i] * 2;
            }
            break;
        case 2:
            for (int i = 0; i < size; i++) {
                array[i] = i - value;
                result += array[i] / 2;
            }
            break;
        case 3:
            for (int i = 0; i < size; i++) {
                array[i] = i ^ value;
                result += array[i] << 1;
            }
            break;
        case 4:
            for (int i = 0; i < size; i++) {
                array[i] = i & value;
                result += array[i] >> 1;
            }
            break;
        case 5:
            for (int i = 0; i < size; i++) {
                array[i] = i | value;
                result += array[i] % 100;
            }
            break;
        case 6:
            for (int i = 0; i < size; i++) {
                array[i] = ~i & value;
                result += abs(array[i]);
            }
            break;
        case 7:
            for (int i = 0; i < size; i++) {
                array[i] = (i * value) % 256;
                result += array[i] * array[i];
            }
            break;
    }
    
    return result;
}

/* Main test function with multiple complex basic blocks */
void test_function_1(int iterations) {
    double accumulator = 0.0;
    int int_accumulator = 0;
    
    /* Allocate arrays for memory operations */
    int size = 64;
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* arr3 = (int*)malloc(size * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* Complex block 1: Mixed floating point and integer operations */
    for (int i = 0; i < iterations; i++) {
        double fp_result = complex_math_chain(accumulator, i * 0.1, sin(i), cos(i));
        int int_result = integer_chain(i, int_accumulator, i * 2, i * 3);
        
        accumulator += fp_result;
        int_accumulator ^= int_result;
        
        /* Memory operations with aliasing */
        memory_ops(arr1, arr2, arr3, 16);
    }
    
    /* Complex block 2: Loop with speculative scheduling opportunities */
    int counter = 0;
    for (int i = 0; i < iterations / 2; i++) {
        double mixed = mixed_operations_block(accumulator, int_accumulator * 0.01, &counter);
        accumulator = fmod(mixed, 1000.0);
    }
    
    /* Complex block 3: Wide basic block */
    accumulator += wide_basic_block(accumulator);
    
    /* Complex block 4: Switch-based computation */
    int_accumulator += switch_based_computation(int_accumulator, arr1, 32);
    
    /* Complex block 5: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    vector_operations(&vec1, &vec2, &vec3, 100);
    
    /* Use results to prevent optimization */
    printf("Test 1: accumulator = %f, int_accumulator = %d, counter = %d\n", 
           accumulator, int_accumulator, counter);
    printf("Vector result: %d %d %d %d\n", vec1[0], vec1[1], vec1[2], vec1[3]);
    
    free(arr1);
    free(arr2);
    free(arr3);
}

/* Second test function with different instruction mix */
void test_function_2(int iterations) {
    volatile double checksum = 0.0;  /* volatile to prevent optimization */
    
    /* Unrolled loop creating wide basic block */
    for (int i = 0; i < iterations; i += 8) {
        /* Create many independent computation chains */
        double a = i * 1.234;
        double b = sin(a) * cos(a);
        double c = sqrt(fabs(b)) + 1.0;
        
        double d = (i + 1) * 2.345;
        double e = sin(d) * cos(d);
        double f = sqrt(fabs(e)) + 2.0;
        
        double g = (i + 2) * 3.456;
        double h = sin(g) * cos(g);
        double j = sqrt(fabs(h)) + 3.0;
        
        double k = (i + 3) * 4.567;
        double l = sin(k) * cos(k);
        double m = sqrt(fabs(l)) + 4.0;
        
        double n = (i + 4) * 5.678;
        double o = sin(n) * cos(n);
        double p = sqrt(fabs(o)) + 5.0;
        
        double q = (i + 5) * 6.789;
        double r = sin(q) * cos(q);
        double s = sqrt(fabs(r)) + 6.0;
        
        double t = (i + 6) * 7.890;
        double u = sin(t) * cos(t);
        double v = sqrt(fabs(u)) + 7.0;
        
        double w = (i + 7) * 8.901;
        double x = sin(w) * cos(w);
        double y = sqrt(fabs(x)) + 8.0;
        
        /* Combine results with dependencies */
        checksum += (b + e + h + l + o + r + u + x) * 
                   (c + f + j + m + p + s + v + y);
    }
    
    /* Additional complex block with inline assembly barrier */
    int value = (int)checksum;
    asm volatile ("" : "+r" (value) : : "memory");
    
    /* More arithmetic */
    for (int i = 0; i < 100; i++) {
        value = value * 1103515245 + 12345;
        checksum += (value & 0x7FFFFFFF) / 2147483648.0;
    }
    
    printf("Test 2: checksum = %f, value = %d\n", checksum, value);
}

/* Third test function focusing on memory and pointer operations */
void test_function_3(int size) {
    /* Allocate and initialize matrices */
    double* matrix1 = (double*)malloc(size * size * sizeof(double));
    double* matrix2 = (double*)malloc(size * size * sizeof(double));
    double* result = (double*)malloc(size * size * sizeof(double));
    
    for (int i = 0; i < size * size; i++) {
        matrix1[i] = (i % 100) * 0.01;
        matrix2[i] = ((i + 50) % 100) * 0.01;
    }
    
    /* Matrix multiplication - creates many dependent memory operations */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0.0;
            for (int k = 0; k < size; k++) {
                /* Complex addressing calculations */
                double a = matrix1[i * size + k];
                double b = matrix2[k * size + j];
                sum += a * b * sin(a) * cos(b);
            }
            result[i * size + j] = sum;
        }
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < size * size; i++) {
        checksum += result[i] * (i % 10);
    }
    
    printf("Test 3: matrix checksum = %f\n", checksum);
    
    free(matrix1);
    free(matrix2);
    free(result);
}

/* Main driver function */
int main(int argc, char** argv) {
    int iterations = 1000;
    int matrix_size = 32;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    if (argc > 2) {
        matrix_size = atoi(argv[2]);
    }
    
    printf("Starting scheduler coverage tests...\n");
    printf("Using iterations=%d, matrix_size=%d\n\n", iterations, matrix_size);
    
    /* Run all test functions to exercise different scheduling scenarios */
    clock_t start = clock();
    
    test_function_1(iterations);
    test_function_2(iterations);
    test_function_3(matrix_size);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nAll tests completed in %.3f seconds\n", elapsed);
    
    return 0;
}
