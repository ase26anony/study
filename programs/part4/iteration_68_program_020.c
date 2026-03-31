/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Simple PRNG to avoid library function calls in loops */
static inline uint32_t prng(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 100;
volatile int volatile_seed = 42;

/* Complex data-dependent computation with carried dependencies */
static int64_t compute_kernel1(int32_t *data, int size, int stride) {
    int64_t sum = 0;
    volatile int v_counter = 0;
    
    /* Outer loop with volatile bound */
    for (int i = 0; i < volatile_bound; i++) {
        /* Inner loop with data dependency across iterations */
        for (int j = 1; j < size - 1; j++) {
            /* Complex addressing with stride */
            int idx = (j * stride) % size;
            int prev_idx = ((j - 1) * stride) % size;
            
            /* Data-dependent computation with mixed operations */
            int32_t val1 = data[idx];
            int32_t val2 = data[prev_idx];
            
            /* Multiplication with dependency on previous iteration */
            int64_t prod = (int64_t)val1 * (int64_t)val2;
            
            /* Conditional operation creates control flow */
            if (val1 > val2) {
                /* Floating point operation in integer context */
                double ratio = (val1 != 0) ? (double)val2 / (double)val1 : 1.0;
                prod += (int64_t)(ratio * 1000.0);
            } else {
                /* Different computation path */
                prod -= (val1 + val2) * 3;
            }
            
            /* Running sum with dependency chain */
            sum += prod;
            
            /* Inline assembly creates fixed RTL instruction */
            asm volatile ("" : : "r"(prod) : "memory");
            
            /* Volatile update prevents reordering */
            v_counter++;
        }
        
        /* Switch statement creates multiple basic blocks */
        switch (i % 4) {
            case 0:
                sum += data[i % size] * 2;
                break;
            case 1:
                sum -= data[i % size] / 3;
                break;
            case 2:
                sum ^= data[i % size];
                break;
            case 3:
                sum |= data[(i * 3) % size];
                break;
        }
    }
    
    return sum;
}

/* Second computation kernel with different pattern */
static double compute_kernel2(double *matrix, double *vector, int n) {
    double result = 0.0;
    volatile int v_iter = 0;
    
    /* Nested loops for matrix-vector multiplication */
    for (int i = 0; i < n; i++) {
        double row_sum = 0.0;
        
        #pragma GCC unroll 4
        for (int j = 0; j < n; j++) {
            /* Non-contiguous memory access pattern */
            int idx = i * n + j;
            
            /* Mixed precision computation */
            float elem = (float)matrix[idx];
            float vec_elem = (float)vector[j];
            
            /* Complex floating point operation */
            double product = (double)elem * (double)vec_elem;
            
            /* Conditional based on random-like value */
            if ((idx ^ (idx >> 3)) & 1) {
                product *= 1.01;
            } else {
                product /= 1.01;
            }
            
            row_sum += product;
            
            /* Another inline assembly barrier */
            asm volatile ("" : : "r"(product) : "memory");
        }
        
        /* Division with non-constant divisor */
        if (row_sum != 0.0) {
            result += row_sum / (1.0 + (double)(i % 8));
        }
        
        v_iter++;
    }
    
    return result;
}

/* Pointer chasing pattern to create complex dependencies */
static uint64_t pointer_chase(uint64_t *arena, int arena_size, int steps) {
    uint64_t *current = &arena[0];
    uint64_t hash = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Chase pointer through arena */
        uint64_t value = *current;
        hash ^= value;
        
        /* Complex address calculation */
        uint64_t next_idx = (value ^ (value >> 32)) % arena_size;
        current = &arena[next_idx];
        
        /* Mixed-width operations */
        hash += (hash << 32) | (hash >> 32);
        hash *= 0x9e3779b97f4a7c15ULL;
        
        /* Conditional move via ternary */
        hash = (hash & 1) ? (hash ^ 0xdeadbeef) : (hash + 0xcafebabe);
    }
    
    return hash;
}

int main(void) {
    const int data_size = 1024;
    const int matrix_size = 64;
    
    /* Initialize with pseudo-random data */
    int32_t *data = malloc(data_size * sizeof(int32_t));
    double *matrix = malloc(matrix_size * matrix_size * sizeof(double));
    double *vector = malloc(matrix_size * sizeof(double));
    uint64_t *arena = malloc(data_size * sizeof(uint64_t));
    
    uint32_t seed = time(NULL) ^ volatile_seed;
    
    /* Fill arrays with pseudo-random values */
    for (int i = 0; i < data_size; i++) {
        data[i] = (int32_t)prng(&seed);
        arena[i] = prng(&seed);
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = (double)prng(&seed) / 1000.0;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = (double)prng(&seed) / 1000.0;
    }
    
    /* Execute multiple computation kernels */
    int64_t result1 = compute_kernel1(data, data_size, 7);
    double result2 = compute_kernel2(matrix, vector, matrix_size);
    uint64_t result3 = pointer_chase(arena, data_size, 1000);
    
    /* Final reduction to prevent optimization */
    uint64_t final_hash = (uint64_t)result1 ^ (uint64_t)result2 ^ result3;
    
    /* Use results to ensure side effects */
    printf("Results: %ld, %.6f, %lu\n", (long)result1, result2, (unsigned long)result3);
    printf("Final hash: %lu\n", (unsigned long)final_hash);
    
    free(data);
    free(matrix);
    free(vector);
    free(arena);
    
    return 0;
}
