/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define SIZE 1024
#define INNER_ITER 128
#define MAT_SIZE 32

/* Volatile variables to prevent optimization */
volatile int volatile_bound = INNER_ITER;
volatile int volatile_seed = 42;

/* Function with mixed operations to create scheduling complexity */
static inline int32_t mixed_ops(int32_t a, int32_t b, float c, float d) {
    /* Integer operations with dependencies */
    int32_t t1 = a * b;
    int32_t t2 = t1 / (b != 0 ? b : 1);  /* Non-constant divisor */
    int32_t t3 = t2 + (a > b ? a : b);   /* Conditional move-like */
    
    /* Floating point operations */
    float f1 = c * d;
    float f2 = (c != 0.0f) ? f1 / c : d; /* Conditional float op */
    
    /* Mixed-width operations */
    int64_t wide = (int64_t)t3 * (int64_t)((int32_t)f2);
    
    /* Inline assembly to create fixed RTL instructions */
    asm volatile ("" : "+r" (wide) : : "memory");
    
    return (int32_t)(wide & 0xFFFFFFFF) ^ (int32_t)f2;
}

/* Matrix-vector multiplication kernel */
void mat_vec_multiply(float mat[MAT_SIZE][MAT_SIZE], 
                      float vec[MAT_SIZE], 
                      float res[MAT_SIZE]) {
    #pragma GCC unroll 4
    for (int i = 0; i < MAT_SIZE; i++) {
        float sum = 0.0f;
        /* Pointer chasing pattern with stride */
        float *row = mat[i];
        for (int j = 0; j < MAT_SIZE; j++) {
            /* Complex addressing with dependency */
            sum += row[j] * vec[(j + i) % MAT_SIZE];
            
            /* Conditional operation inside loop */
            if (sum > 1000.0f) {
                sum *= 0.99f;  /* Branch with FP operation */
            } else {
                sum += 0.01f * vec[j];  /* Alternative path */
            }
        }
        res[i] = sum;
        
        /* Another inline assembly barrier */
        asm volatile ("" : : "r" (sum) : "memory");
    }
}

/* Complex loop with carried dependencies */
uint64_t data_dependent_loop(int32_t *data, int n) {
    uint64_t sum = 0;
    int volatile_counter = volatile_bound;  /* Volatile prevents unrolling */
    
    /* Outer loop */
    for (int outer = 0; outer < n / INNER_ITER; outer++) {
        int start = outer * INNER_ITER;
        
        /* Inner loop with carried dependency */
        for (int i = start + 1; i < start + volatile_counter; i++) {
            /* Data-dependent computation with cross-iteration dependency */
            int32_t val1 = data[i];
            int32_t val2 = data[i-1];  /* Dependency on previous iteration */
            
            /* Mixed operations create scheduling pressure */
            int32_t result = mixed_ops(val1, val2, 
                                      (float)val1 * 0.1f, 
                                      (float)val2 * 0.2f);
            
            /* Complex conditional with both paths having computation */
            if ((result & 0xF) == 0) {
                sum += result * 3;  /* Path A */
                /* Switch statement inside hot loop */
                switch (result % 4) {
                    case 0: sum ^= 0xAAAAAAAA; break;
                    case 1: sum |= 0x55555555; break;
                    case 2: sum = (sum << 3) | (sum >> 61); break;
                    case 3: sum = ~sum; break;
                }
            } else {
                sum += result / 2;  /* Path B */
                /* Different operations in alternative path */
                sum = (sum * 1103515245 + 12345) & 0x7FFFFFFF;
            }
            
            /* Non-trivial memory access pattern */
            data[i] = (result + (int32_t)sum) & 0xFF;
        }
        
        /* Prevent optimization with external call */
        if (outer % 8 == 0) {
            volatile_seed = rand() % 256;
        }
    }
    
    return sum;
}

/* Another computation kernel with different pattern */
float fp_intensive_kernel(float *a, float *b, int n) {
    float prod = 1.0f;
    float sum = 0.0f;
    
    /* Loop with multiple accumulators */
    for (int i = 0; i < n; i++) {
        /* Multiple FP operations with dependencies */
        float t1 = a[i] * b[i];
        float t2 = t1 + a[(i + 1) % n];
        float t3 = t2 - b[(i + 2) % n];
        float t4 = t3 * t3;
        
        /* Division with non-constant divisor */
        prod *= (t4 != 0.0f) ? t4 : 1.0f;
        sum += t4;
        
        /* Prevent vectorization with data-dependent condition */
        if (prod > 1e10f) {
            prod *= 0.5f;
            asm volatile ("" : : "r" (prod) : "memory");
        }
    }
    
    return prod + sum;
}

int main() {
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    int32_t *data = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float *fdata1 = (float*)malloc(SIZE * sizeof(float));
    float *fdata2 = (float*)malloc(SIZE * sizeof(float));
    float matrix[MAT_SIZE][MAT_SIZE];
    float vec[MAT_SIZE], res[MAT_SIZE];
    
    /* Initialize with varied data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (rand() % 1000) - 500;
        fdata1[i] = (rand() % 1000) / 100.0f;
        fdata2[i] = (rand() % 1000) / 100.0f;
    }
    
    for (int i = 0; i < MAT_SIZE; i++) {
        vec[i] = (rand() % 100) / 10.0f;
        for (int j = 0; j < MAT_SIZE; j++) {
            matrix[i][j] = (rand() % 100) / 10.0f;
        }
    }
    
    /* Perform multiple computation kernels */
    uint64_t result1 = 0;
    float result2 = 0.0f;
    
    /* Run multiple iterations to increase scheduling opportunities */
    for (int iter = 0; iter < 3; iter++) {
        result1 ^= data_dependent_loop(data, SIZE);
        result2 += fp_intensive_kernel(fdata1, fdata2, SIZE);
        mat_vec_multiply(matrix, vec, res);
        
        /* Modify data between iterations to prevent dead code elimination */
        for (int i = 0; i < SIZE; i++) {
            data[i] = (data[i] * 13 + 7) & 0xFFF;
        }
    }
    
    /* Final reduction to ensure side effects */
    uint64_t final_result = result1 ^ (uint64_t)result2;
    for (int i = 0; i < MAT_SIZE; i++) {
        final_result ^= (uint64_t)res[i];
    }
    
    printf("Result: 0x%016llX\n", (unsigned long long)final_result);
    
    /* Cleanup */
    free(data);
    free(fdata1);
    free(fdata2);
    
    return 0;
}
