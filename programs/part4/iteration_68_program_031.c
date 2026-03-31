/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumping in GCC.
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
static int64_t complex_kernel(int32_t *data, int32_t *weights, int n, volatile int *v_bound) {
    int64_t sum = 0;
    int32_t prev = data[0];
    float fp_acc = 0.0f;
    double dp_acc = 0.0;
    
    /* Outer loop with volatile bound */
    for (int i = 1; i < *v_bound; i++) {
        int idx = i % n;
        
        /* Data-dependent computation with carried dependency */
        int32_t current = data[idx];
        int32_t product = prev * current;
        prev = current;
        
        /* Mixed-width operations */
        int64_t extended = (int64_t)product * weights[idx];
        
        /* Conditional operations */
        int32_t conditional = (product > 0) ? extended & 0xFF : extended >> 8;
        
        /* Floating point operations */
        fp_acc += (float)conditional * 0.5f;
        dp_acc += (double)conditional * 0.25;
        
        /* Integer division with non-constant divisor */
        if (weights[idx] != 0) {
            sum += extended / (weights[idx] | 1);
        }
        
        /* Inline assembly to create fixed RTL instructions */
        asm volatile ("" : : "r"(sum) : "memory");
        
        /* Inner loop with partial unrolling */
        #pragma GCC unroll 4
        for (int j = 0; j < 4; j++) {
            /* Memory access with non-trivial addressing */
            int offset = (idx + j) % n;
            int32_t temp = data[offset] * weights[offset];
            
            /* Branch with computation in both paths */
            if (temp & 1) {
                sum += temp * 3;
                fp_acc -= 1.0f;
            } else {
                sum -= temp / 2;
                dp_acc += 0.5;
            }
        }
        
        /* Switch statement for control flow complexity */
        switch (i & 3) {
            case 0:
                sum += (int64_t)(fp_acc * 10.0f);
                break;
            case 1:
                sum += (int64_t)(dp_acc * 5.0);
                break;
            case 2:
                /* Pointer chasing pattern */
                int32_t *ptr = &data[idx];
                for (int k = 0; k < 2; k++) {
                    sum += *ptr;
                    ptr = &data[(*ptr) % n];
                }
                break;
            case 3:
                /* More complex arithmetic */
                sum += (sum << 3) | (sum >> 5);
                break;
        }
    }
    
    /* Final reduction */
    return sum + (int64_t)fp_acc + (int64_t)dp_acc;
}

/* Second computation kernel for additional scheduling regions */
static double matrix_vector_kernel(double *matrix, double *vector, int size, volatile int iter) {
    double result = 0.0;
    
    for (int it = 0; it < iter; it++) {
        double temp = 0.0;
        
        /* Strided memory access pattern */
        for (int i = 0; i < size; i++) {
            double dot = 0.0;
            for (int j = 0; j < size; j++) {
                /* Non-unit stride access */
                int idx = (i * size + j) % (size * size);
                dot += matrix[idx] * vector[j];
                
                /* Conditional floating point operation */
                if (dot > 100.0) {
                    dot *= 0.9;
                }
            }
            temp += dot * vector[i];
            
            /* Inline assembly barrier */
            asm volatile ("" : : "r"(temp) : "memory");
        }
        
        /* Data-dependent update */
        if (it & 1) {
            result += temp / (it + 1);
        } else {
            result -= temp * 0.5;
        }
    }
    
    return result;
}

int main(void) {
    const int data_size = 1024;
    const int matrix_size = 32;
    
    /* Allocate and initialize data with pseudo-random values */
    int32_t *data = (int32_t*)malloc(data_size * sizeof(int32_t));
    int32_t *weights = (int32_t*)malloc(data_size * sizeof(int32_t));
    double *matrix = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double *vector = (double*)malloc(matrix_size * sizeof(double));
    
    /* Initialize with simple PRNG */
    for (int i = 0; i < data_size; i++) {
        data[i] = (int32_t)simple_rand() % 256;
        weights[i] = (int32_t)simple_rand() % 128 + 1;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = (double)(simple_rand() % 1000) / 100.0;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = (double)(simple_rand() % 1000) / 200.0;
    }
    
    /* Volatile iteration counters */
    volatile int iter1 = volatile_bound;
    volatile int iter2 = volatile_seed % 50 + 10;
    
    /* First complex kernel */
    int64_t result1 = complex_kernel(data, weights, data_size, &iter1);
    
    /* Second kernel */
    double result2 = matrix_vector_kernel(matrix, vector, matrix_size, iter2);
    
    /* Combine results to prevent optimization */
    double final_result = (double)result1 + result2;
    
    /* Use result to ensure side effects */
    printf("Result: %f\n", final_result);
    
    /* Cleanup */
    free(data);
    free(weights);
    free(matrix);
    free(vector);
    
    return 0;
}
