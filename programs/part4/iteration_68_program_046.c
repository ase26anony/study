/* sel_sched_trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* Function with complex control flow */
static inline int complex_branch(int x, int y) {
    if (x > y) {
        return (x * y) | (x ^ y);
    } else if (x < y) {
        return (x + y) * (x - y);
    } else {
        return (x << 4) | (y >> 2);
    }
}

/* Pointer chasing pattern */
static int chase_pointer(int *data, int start, int steps) {
    int idx = start;
    int sum = 0;
    for (int i = 0; i < steps; i++) {
        sum += data[idx];
        idx = data[idx] % steps;  /* Non-linear access pattern */
        asm volatile("" ::: "memory");  /* Force specific RTL pattern */
    }
    return sum;
}

/* Mixed-width operations */
static uint64_t mixed_width_compute(uint32_t *arr32, uint64_t *arr64, int len) {
    uint64_t acc64 = 0;
    uint32_t acc32 = 0;
    
    for (int i = 0; i < len; i++) {
        /* Mixed 32-bit and 64-bit operations */
        uint64_t temp64 = (uint64_t)arr32[i] * arr64[i % 16];
        uint32_t temp32 = arr32[i] ^ (temp64 >> 32);
        
        acc64 += temp64;
        acc32 += temp32;
        
        /* Conditional move via ternary */
        acc64 = (temp64 > acc64) ? temp64 : (acc64 - 1);
    }
    
    return acc64 + acc32;
}

/* Nested loop with carried dependency */
static double nested_loop_dep(double *data, int n, volatile int bound) {
    double sum = 0.0;
    double prod = 1.0;
    
    /* Outer loop with volatile bound */
    for (int i = 1; i < bound; i++) {
        float fsum = 0.0f;
        
        /* Inner loop with data dependency */
        #pragma GCC unroll 4
        for (int j = 1; j < n; j++) {
            /* Cross-iteration dependency */
            double val = data[j] * data[j-1];
            
            /* Complex conditional */
            if (j % 3 == 0) {
                val /= (data[j] + 1.0);
                fsum += (float)val;
            } else if (j % 3 == 1) {
                val *= 1.5;
                fsum -= (float)val;
            } else {
                val = (val > 0) ? val : -val;
                fsum *= (float)val;
            }
            
            /* Non-constant divisor */
            sum += val / (i + j + 1);
        }
        
        prod *= fsum;
        asm volatile("" ::: "memory");  /* Another fixed RTL instruction */
    }
    
    return sum + prod;
}

/* Matrix-vector multiplication kernel */
static void matvec_multiply(float *result, float **matrix, float *vector, 
                           int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        float sum = 0.0f;
        float *row = matrix[i];
        
        /* Unrolled inner loop */
        #pragma GCC unroll 2
        for (int j = 0; j < cols; j++) {
            /* Strided access pattern */
            sum += row[j] * vector[j];
            
            /* Branch with computation in both paths */
            if (j % 2 == 0) {
                sum = sum * 0.99f - 0.01f;
            } else {
                sum = sum * 1.01f + 0.01f;
            }
        }
        
        result[i] = sum;
        
        /* Switch statement creating multiple basic blocks */
        switch (i % 4) {
            case 0:
                result[i] += 1.0f;
                break;
            case 1:
                result[i] *= 2.0f;
                break;
            case 2:
                result[i] = result[i] > 0 ? result[i] : -result[i];
                break;
            case 3:
                result[i] = 1.0f / (result[i] + 1.0f);
                break;
        }
    }
}

/* Main computation */
int main(void) {
    const int SIZE = 1024;
    const int MAT_SIZE = 64;
    
    /* Initialize with pseudo-random data */
    srand(g_volatile_seed);
    
    /* Allocate and initialize arrays */
    double *data = (double*)malloc(SIZE * sizeof(double));
    uint32_t *arr32 = (uint32_t*)malloc(SIZE * sizeof(uint32_t));
    uint64_t *arr64 = (uint64_t*)malloc(16 * sizeof(uint64_t));
    int *int_data = (int*)malloc(SIZE * sizeof(int));
    
    float **matrix = (float**)malloc(MAT_SIZE * sizeof(float*));
    float *vector = (float*)malloc(MAT_SIZE * sizeof(float));
    float *result = (float*)malloc(MAT_SIZE * sizeof(float));
    
    for (int i = 0; i < SIZE; i++) {
        data[i] = (rand() % 1000) / 100.0;
        arr32[i] = rand();
        int_data[i] = rand() % SIZE;
    }
    
    for (int i = 0; i < 16; i++) {
        arr64[i] = ((uint64_t)rand() << 32) | rand();
    }
    
    for (int i = 0; i < MAT_SIZE; i++) {
        matrix[i] = (float*)malloc(MAT_SIZE * sizeof(float));
        for (int j = 0; j < MAT_SIZE; j++) {
            matrix[i][j] = (rand() % 1000) / 100.0f;
        }
        vector[i] = (rand() % 1000) / 100.0f;
    }
    
    /* Volatile loop counter */
    volatile int v_bound = g_volatile_bound % 500 + 100;
    
    /* First computation: nested loops with dependencies */
    double res1 = nested_loop_dep(data, SIZE, v_bound);
    
    /* Second: pointer chasing */
    int res2 = chase_pointer(int_data, 0, v_bound);
    
    /* Third: mixed-width operations */
    uint64_t res3 = mixed_width_compute(arr32, arr64, SIZE);
    
    /* Fourth: matrix-vector with control flow */
    matvec_multiply(result, matrix, vector, MAT_SIZE, MAT_SIZE);
    float res4 = 0.0f;
    for (int i = 0; i < MAT_SIZE; i++) {
        res4 += result[i];
    }
    
    /* Final reduction with XOR to prevent optimization */
    uint64_t final_result = 0;
    final_result ^= *(uint64_t*)&res1;
    final_result ^= res2;
    final_result ^= res3;
    final_result ^= *(uint32_t*)&res4;
    
    printf("Result: 0x%016lx\n", final_result);
    
    /* Cleanup */
    free(data);
    free(arr32);
    free(arr64);
    free(int_data);
    
    for (int i = 0; i < MAT_SIZE; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(vector);
    free(result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
