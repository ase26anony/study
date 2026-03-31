/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Simple PRNG to avoid libc rand() inlining issues */
static uint32_t prng_state = 123456789;
static inline uint32_t simple_rand(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function with mixed operations to create scheduling complexity */
static int64_t complex_kernel(int32_t *data, int32_t *weights, int n, int stride) {
    int64_t sum = 0;
    int32_t prev = 0;
    volatile int vol_counter = 0;
    
    /* Outer loop with data-dependent computation */
    for (int i = 0; i < n; i++) {
        int idx = i * stride;
        vol_counter++; /* Volatile access prevents loop optimizations */
        
        /* Inner loop with carried dependency */
        #pragma GCC unroll 4
        for (int j = 0; j < 8; j++) {
            int32_t current = data[idx + j];
            
            /* Complex data-dependent computation with mixed operations */
            int64_t temp = (int64_t)current * (int64_t)prev;
            
            /* Conditional operations create control flow */
            if (current > 0) {
                /* Mixed-width arithmetic */
                temp = temp / (simple_rand() % 256 + 1); /* Non-constant divisor */
                temp += weights[idx + j] * 3;
            } else {
                temp = temp >> (current & 0x7); /* Variable shift */
                temp -= weights[idx + j] / 2;
            }
            
            /* Floating-point operations to create FPU pressure */
            double fp_temp = (double)temp * 1.5;
            temp = (int64_t)(fp_temp * 0.6667);
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : "+r" (temp) : : "memory");
            
            /* Running sum with dependency chain */
            sum += temp;
            prev = current;
            
            /* Additional conditional with ternary operator */
            sum = (sum > 1000000) ? sum % 1000000 : sum;
        }
        
        /* Switch statement for additional control flow */
        switch (i % 4) {
            case 0:
                sum += data[idx] * 2;
                break;
            case 1:
                sum -= data[idx + 1];
                break;
            case 2:
                sum ^= data[idx + 2];
                break;
            case 3:
                sum = sum * 3 / 2;
                break;
        }
    }
    
    return sum;
}

/* Second computation kernel with different pattern */
static double matrix_vector_kernel(double *matrix, double *vector, int size) {
    double result = 0.0;
    volatile int vol_idx = 0;
    
    for (int i = 0; i < size; i++) {
        double row_sum = 0.0;
        vol_idx = i; /* Volatile write */
        
        for (int j = 0; j < size; j++) {
            /* Non-unit stride access */
            double elem = matrix[i * size + j];
            
            /* Complex FP operation chain */
            elem *= vector[j] * (1.0 + (simple_rand() % 100) / 1000.0);
            
            /* Branch with substantial computation on both sides */
            if (elem > 0.5) {
                row_sum += elem * elem;
                /* Additional computation in taken branch */
                row_sum = row_sum / (1.0 + j * 0.01);
            } else {
                row_sum -= elem * 0.5;
                /* Different computation in not-taken branch */
                row_sum = row_sum * (1.0 - j * 0.005);
            }
            
            /* Inline assembly barrier */
            asm volatile ("" : "+r" (row_sum) : : "memory");
        }
        
        result += row_sum;
        
        /* Prevent excessive unrolling */
        if (i % 16 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return result;
}

int main(void) {
    const int data_size = 1024;
    const int matrix_size = 64;
    
    /* Allocate and initialize data with pseudo-random values */
    int32_t *data = (int32_t*)malloc(data_size * sizeof(int32_t));
    int32_t *weights = (int32_t*)malloc(data_size * sizeof(int32_t));
    double *matrix = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double *vector = (double*)malloc(matrix_size * sizeof(double));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < data_size; i++) {
        data[i] = (int32_t)simple_rand() % 1000 - 500;
        weights[i] = (int32_t)simple_rand() % 100;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = (simple_rand() % 1000) / 1000.0;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = (simple_rand() % 1000) / 500.0 - 1.0;
    }
    
    /* Use volatile bound to prevent compile-time optimization */
    int bound = volatile_bound;
    if (bound > data_size / 8) bound = data_size / 8;
    
    /* First computation kernel */
    int64_t result1 = 0;
    for (int iter = 0; iter < 10; iter++) {
        /* Varying stride creates different access patterns */
        int stride = (iter % 3) + 1;
        result1 ^= complex_kernel(data, weights, bound, stride);
        
        /* Volatile function call prevents reordering */
        volatile_seed = simple_rand() % 256;
    }
    
    /* Second computation kernel */
    double result2 = 0.0;
    for (int iter = 0; iter < 5; iter++) {
        result2 += matrix_vector_kernel(matrix, vector, matrix_size);
        
        /* Modify vector slightly each iteration */
        for (int i = 0; i < matrix_size; i++) {
            vector[i] *= 0.95 + (simple_rand() % 100) / 1000.0;
        }
    }
    
    /* Final reduction to ensure side effects */
    int64_t final_result = result1 ^ (int64_t)result2;
    printf("Result: %ld (checksum: %f)\n", 
           (long)final_result, result2);
    
    /* Cleanup */
    free(data);
    free(weights);
    free(matrix);
    free(vector);
    
    return 0;
}
