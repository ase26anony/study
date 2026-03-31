/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Simple PRNG to avoid libc rand() overhead in analysis */
static uint32_t prng_state = 123456789;
static inline uint32_t simple_rand() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function with mixed-width operations to create register pressure */
static inline int64_t mixed_width_ops(int32_t a, int64_t b, int32_t c) {
    int64_t wide = (int64_t)a * b;      /* 32-bit * 64-bit */
    int32_t narrow = (int32_t)(b >> 32); /* 64-bit to 32-bit */
    int64_t result = wide / (c + 1);     /* Division with variable divisor */
    
    /* Conditional move via ternary */
    result = (result > 0) ? result : -result;
    
    /* Inline assembly to create fixed RTL instruction */
    asm volatile ("" : "+r" (result) : : "memory");
    
    return result;
}

/* Matrix-vector multiplication kernel */
void matvec_multiply(float* restrict result, 
                     const float* restrict matrix,
                     const float* restrict vector,
                     int rows, int cols) {
    volatile int vol_rows = rows; /* Prevent loop unrolling before scheduling */
    
    #pragma GCC unroll 4
    for (int i = 0; i < vol_rows; i++) {
        float sum = 0.0f;
        const float* row_ptr = &matrix[i * cols];
        
        /* Inner loop with data dependency */
        for (int j = 0; j < cols; j++) {
            /* Complex addressing with stride */
            float val = row_ptr[j] * vector[j];
            
            /* Branch with substantial computation in both paths */
            if (j > 0 && (simple_rand() & 1)) {
                /* Path 1: More floating point ops */
                sum += val * 1.5f - (float)(j % 3);
                asm volatile ("" ::: "memory"); /* Fixed instruction */
            } else {
                /* Path 2: Different computation */
                sum += val / ((j % 5) + 1.0f) + 0.25f;
                
                /* Switch statement inside loop */
                switch (j % 4) {
                    case 0: sum += 0.1f; break;
                    case 1: sum -= 0.1f; break;
                    case 2: sum *= 1.01f; break;
                    case 3: sum /= 1.01f; break;
                }
            }
            
            /* Pointer chasing pattern */
            if (j > 1) {
                val += row_ptr[j-1] * 0.3f + row_ptr[j-2] * 0.1f;
            }
        }
        
        /* Non-trivial store with conversion */
        result[i] = sum + (float)(i % 7);
    }
}

/* Integer computation with carried dependencies */
uint64_t integer_kernel(const int32_t* data, int size) {
    uint64_t acc1 = 0, acc2 = 0;
    volatile int vol_size = size;
    
    /* Nested loops with carried dependency */
    for (int i = 1; i < vol_size; i++) {
        int32_t prev = data[i-1];
        int32_t curr = data[i];
        
        /* Data-dependent computation */
        acc1 += (uint64_t)(prev * curr);
        
        /* Inner loop with variable bound */
        int inner_bound = (simple_rand() % 8) + 1;
        for (int j = 0; j < inner_bound; j++) {
            /* Mixed operations creating scheduling complexity */
            int32_t temp = prev + (curr >> (j % 5));
            temp = (temp > 0) ? temp : -temp;
            
            /* Division with non-constant divisor */
            if (temp != 0) {
                acc2 += (uint64_t)(curr / (temp % 7 + 1));
            }
            
            /* Memory barrier via inline assembly */
            asm volatile ("" ::: "memory");
        }
        
        /* Conditional update with side effect */
        if ((acc1 & 0xFF) > 128) {
            acc2 ^= acc1;
            asm volatile ("" ::: "memory");
        }
    }
    
    return acc1 ^ acc2;
}

int main() {
    const int N = 1024;
    const int M = 64;
    
    /* Initialize with pseudo-random data */
    int32_t* int_data = (int32_t*)malloc(N * sizeof(int32_t));
    float* matrix = (float*)malloc(M * M * sizeof(float));
    float* vector = (float*)malloc(M * sizeof(float));
    float* result = (float*)malloc(M * sizeof(float));
    
    /* Use volatile seed to prevent compile-time computation */
    prng_state = volatile_seed;
    
    for (int i = 0; i < N; i++) {
        int_data[i] = (int32_t)(simple_rand() % 1000) - 500;
    }
    
    for (int i = 0; i < M * M; i++) {
        matrix[i] = (float)(simple_rand() % 100) / 10.0f;
    }
    
    for (int i = 0; i < M; i++) {
        vector[i] = (float)(simple_rand() % 100) / 10.0f;
    }
    
    /* Run both kernels multiple times */
    uint64_t int_result = 0;
    for (int iter = 0; iter < volatile_bound % 10; iter++) {
        /* Integer kernel */
        int_result ^= integer_kernel(int_data, N);
        
        /* Floating-point kernel */
        matvec_multiply(result, matrix, vector, M, M);
        
        /* Modify data slightly between iterations */
        for (int i = 0; i < N; i += 16) {
            int_data[i] += (iter % 3);
        }
        
        /* External function call to prevent optimization */
        if (iter % 100 == 0) {
            time(NULL);
        }
    }
    
    /* Final reduction to ensure side effects */
    float float_sum = 0.0f;
    for (int i = 0; i < M; i++) {
        float_sum += result[i];
    }
    
    uint64_t final_result = int_result ^ (uint64_t)(float_sum * 1000.0f);
    
    printf("Result: %lu\n", final_result);
    
    free(int_data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
