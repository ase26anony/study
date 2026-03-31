#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with hot attribute and scheduling pressure */
__attribute__((hot, optimize("O3", "sched-pressure"))) 
static float hot_function(float* restrict a, float* restrict b, int n) {
    float sum = 0.0f;
    
    /* Mixed integer and floating point operations */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on a */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: overwriting temp */
        temp = temp + b[i];
        
        /* WAW hazard: multiple writes to sum */
        sum += temp;
        
        /* Pointer chasing pattern */
        b[i] = temp * 0.5f;
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
    }
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static int cold_function(int* restrict arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 7) {
            case 0:
                result += arr[i] * 2;
                /* Early exit point */
                if (result > 1000) return result;
                break;
            case 1:
                result -= arr[i];
                break;
            case 2:
                result ^= arr[i];
                /* Conditional move */
                result = (result < 0) ? 0 : result;
                break;
            case 3:
                /* Nested loop with data dependencies */
                for (int j = 0; j < 3; j++) {
                    result += arr[i] >> j;
                }
                break;
            case 4:
                result |= arr[i];
                break;
            case 5:
                /* Continue condition */
                if (arr[i] % 3 == 0) continue;
                result *= arr[i] % 10;
                break;
            default:
                result = result >> 1;
                break;
        }
        
        /* Assembly with register clobber */
        asm volatile("" : : : "r0", "r1", "r2", "r3");
    }
    return result;
}

/* Vectorization-friendly function */
__attribute__((optimize("O3")))
static void vectorized_loop(double* restrict a, double* restrict b, 
                           double* restrict c, int n) {
    /* SIMD-friendly loop with varying unroll factors */
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        /* Multiple data dependencies */
        double t1 = a[i] * 3.14159;
        double t2 = b[i] + t1;
        double t3 = sin(t2);
        
        /* WAW hazard on c[i] */
        c[i] = t1 * t2;
        c[i] = c[i] + t3;
        
        /* Cross-iteration dependency */
        if (i > 0) {
            c[i] += a[i-1] * 0.1;
        }
    }
}

/* Function with mixed operations and scheduling barriers */
__attribute__((optimize("O2")))
static int mixed_operations(int* restrict arr, float* restrict farr, int n) {
    int int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Integer operations */
        int val = arr[i];
        int_sum += val;
        
        /* Type conversion hazard */
        float fval = (float)val;
        
        /* Floating point operations */
        float_sum += fval * fval;
        
        /* Memory barrier splitting scheduling region */
        asm volatile("" ::: "memory");
        
        /* More operations after barrier */
        int_sum ^= (int)fval;
        
        /* Complex expression with multiple dependencies */
        farr[i] = sinf(fval) * cosf(float_sum) + tanhf(int_sum % 100);
    }
    
    return int_sum + (int)float_sum;
}

/* Function with pointer chasing and complex addressing */
__attribute__((noinline))
static double pointer_chasing(double** matrix, int rows, int cols) {
    double total = 0.0;
    
    for (int i = 0; i < rows; i++) {
        double* row = matrix[i];
        
        #pragma GCC unroll 2
        for (int j = 0; j < cols; j++) {
            /* Pointer chasing with offset */
            double* ptr = row + j;
            
            /* Load/store sequence with varying latencies */
            double val = *ptr;
            val = val * 1.5 + (double)(i * j);
            
            /* Store with dependency */
            *ptr = val;
            
            /* Accumulate with WAW hazard */
            total += val;
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            
            /* Additional computation */
            total = total * 0.99;
        }
    }
    
    return total;
}

/* Main test driver */
int main() {
    /* Initialize data */
    float fa[SIZE], fb[SIZE];
    double da[SIZE], db[SIZE], dc[SIZE];
    int ia[SIZE];
    double* matrix[16];
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX;
        fb[i] = (float)rand() / RAND_MAX;
        da[i] = (double)rand() / RAND_MAX;
        db[i] = (double)rand() / RAND_MAX;
        ia[i] = rand() % 1000;
    }
    
    for (int i = 0; i < 16; i++) {
        matrix[i] = (double*)malloc(16 * sizeof(double));
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (double)rand() / RAND_MAX;
        }
    }
    
    /* Accumulator to prevent dead code elimination */
    volatile float total_f = 0.0f;
    volatile double total_d = 0.0;
    volatile int total_i = 0;
    
    /* Execute test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function - should trigger selective scheduling */
        total_f += hot_function(fa, fb, SIZE);
        
        /* Call cold function with complex control flow */
        total_i += cold_function(ia, SIZE);
        
        /* Vectorized operations */
        vectorized_loop(da, db, dc, SIZE);
        total_d += dc[SIZE-1];
        
        /* Mixed operations */
        total_i += mixed_operations(ia, fa, SIZE);
        
        /* Pointer chasing */
        total_d += pointer_chasing(matrix, 16, 16);
        
        /* Modify inputs slightly to avoid pure loop optimization */
        for (int i = 0; i < SIZE; i++) {
            fa[i] += 0.001f;
            ia[i] ^= 1;
        }
    }
    
    /* Print results to ensure execution */
    printf("Results: f=%.2f, d=%.2f, i=%d\n", 
           (double)total_f, total_d, total_i);
    
    /* Cleanup */
    for (int i = 0; i < 16; i++) {
        free(matrix[i]);
    }
    
    return 0;
}
