/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* External function to create dependencies */
extern int rand_r(unsigned int *seed);

/* Complex computation with data dependencies */
static long long complex_kernel(int *data, int size, volatile int *bound) {
    long long sum = 0;
    long long prod = 1;
    int local_seed = g_volatile_seed;
    
    /* Outer loop with volatile bound */
    for (int i = 1; i < *bound; i++) {
        int inner_bound = (rand_r(&local_seed) % 16) + 8; /* Non-constant bound */
        
        /* Inner loop with carried dependency */
        #pragma GCC unroll 4
        for (int j = 0; j < inner_bound; j++) {
            /* Data-dependent computation with mixed operations */
            int idx = (i * j) % size;
            int prev_idx = ((i-1) * j) % size;
            
            /* Complex addressing with multiple operations */
            int val1 = data[idx];
            int val2 = data[prev_idx];
            
            /* Mixed-width arithmetic */
            long long temp = (long long)val1 * val2;
            
            /* Conditional operations */
            if (temp > 1000000) {
                sum += temp / (val2 ? val2 : 1); /* Division with non-constant divisor */
                prod *= (val1 % 7) + 1;
            } else {
                sum += temp * 3;
                prod /= (val1 % 5) ? (val1 % 5) : 2;
            }
            
            /* Floating-point operations to create FPU pressure */
            double fp_temp = (double)val1 / (val2 + 1.0);
            sum += (long long)(fp_temp * 100.0);
            
            /* Inline assembly to create fixed RTL patterns */
            asm volatile ("" : : "r"(val1), "r"(val2) : "memory");
        }
        
        /* Switch-like control flow within loop */
        switch (i % 4) {
            case 0:
                sum += prod >> 2;
                break;
            case 1:
                sum -= prod * 2;
                break;
            case 2:
                sum ^= (int)prod;
                break;
            case 3:
                sum = (sum << 1) | (prod & 1);
                break;
        }
        
        /* Pointer chasing pattern */
        int *ptr = &data[i % size];
        for (int k = 0; k < 3; k++) {
            sum += *ptr;
            ptr = &data[(*ptr) % size];
        }
    }
    
    return sum ^ (int)prod;
}

/* Second computation kernel - matrix-vector style */
static long long matrix_vector_kernel(int *matrix, int *vector, int n, volatile int iter) {
    long long result = 0;
    
    for (int it = 0; it < iter; it++) {
        for (int i = 0; i < n; i++) {
            int row_sum = 0;
            
            /* Unrolled inner product */
            #pragma GCC unroll 2
            for (int j = 0; j < n; j++) {
                int idx = i * n + j;
                row_sum += matrix[idx] * vector[j];
                
                /* Complex addressing with stride */
                if (j % 3 == 0) {
                    row_sum -= matrix[(idx + n) % (n * n)];
                }
            }
            
            /* Branch with computation in both paths */
            if (row_sum > 0) {
                result += (long long)row_sum * (i + 1);
                /* More operations in taken branch */
                result ^= matrix[i * n + (i % n)];
            } else {
                result -= (long long)(-row_sum) / ((i % 7) + 1);
                /* Different operations in not-taken branch */
                result |= vector[i] & 0xFF;
            }
            
            /* Memory barrier via inline asm */
            asm volatile ("" : : : "memory");
        }
        
        /* Modify vector for next iteration */
        for (int i = 0; i < n; i++) {
            vector[i] = (vector[i] * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    return result;
}

int main(void) {
    const int data_size = 1024;
    const int matrix_size = 32;
    
    /* Initialize with pseudo-random data */
    int *data = (int*)malloc(data_size * sizeof(int));
    int *matrix = (int*)malloc(matrix_size * matrix_size * sizeof(int));
    int *vector = (int*)malloc(matrix_size * sizeof(int));
    
    if (!data || !matrix || !vector) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pseudo-random values */
    unsigned int seed = time(NULL);
    for (int i = 0; i < data_size; i++) {
        data[i] = rand_r(&seed) % 1000;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = rand_r(&seed) % 100 - 50;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = rand_r(&seed) % 100;
    }
    
    /* Run first kernel */
    volatile int bound1 = g_volatile_bound;
    long long result1 = complex_kernel(data, data_size, &bound1);
    
    /* Run second kernel */
    volatile int bound2 = g_volatile_bound / 2;
    long long result2 = matrix_vector_kernel(matrix, vector, matrix_size, bound2);
    
    /* Final reduction to prevent optimization */
    long long final_result = result1 ^ result2;
    printf("Result: %lld\n", final_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
