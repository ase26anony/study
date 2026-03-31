/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INNER_ITER 128
#define UNROLL_FACTOR 4

/* Volatile variables to prevent optimization */
volatile int volatile_bound = INNER_ITER;
volatile int volatile_seed = 42;

/* Complex data-dependent computation with carried dependency */
static long long data_dependent_sum(int *data, int n, int stride) {
    long long sum = 0;
    volatile int vol_counter = 0;
    
    /* Outer loop with volatile bound */
    for (int i = 1; i < n && vol_counter < volatile_bound; i++) {
        int local_sum = 0;
        
        /* Inner loop with complex addressing and dependencies */
        #pragma GCC unroll UNROLL_FACTOR
        for (int j = 0; j < INNER_ITER; j++) {
            /* Data-dependent computation with carried dependency */
            int idx = (i * stride + j) % n;
            int prev_idx = (idx - 1 + n) % n;
            
            /* Mixed-width operations */
            long temp = (long)data[idx] * (long)data[prev_idx];
            
            /* Conditional operation using ternary */
            int sign = (temp > 0) ? 1 : ((temp < 0) ? -1 : 0);
            
            /* Complex arithmetic with division (non-constant divisor) */
            if (data[idx] != 0) {
                /* Division creates long latency operation */
                local_sum += sign * (temp / (data[idx] + 1));
            }
            
            /* Pointer chasing pattern */
            int *ptr = &data[idx];
            local_sum += *ptr;
            
            /* Inline assembly to create fixed RTL instruction */
            asm volatile ("" : : "r"(ptr) : "memory");
            
            /* Floating point operation to create FP unit pressure */
            float fp_temp = (float)local_sum * 0.5f;
            local_sum += (int)fp_temp;
        }
        
        /* Control flow with multiple basic blocks */
        if (local_sum > 1000) {
            /* Branch 1: Complex computation */
            sum += local_sum * 2LL;
            
            /* Switch statement for additional control flow */
            switch (local_sum % 4) {
                case 0:
                    sum += data[i] * 3LL;
                    break;
                case 1:
                    sum += data[i-1] * 5LL;
                    break;
                case 2:
                    sum += (long long)data[i] * data[(i+1)%n];
                    break;
                default:
                    sum -= local_sum / 2;
            }
        } else {
            /* Branch 2: Different computation pattern */
            sum += (long long)local_sum * local_sum;
            
            /* More mixed operations */
            double dbl_val = (double)local_sum * 1.5;
            sum += (long long)dbl_val;
        }
        
        vol_counter++;
    }
    
    return sum;
}

/* Second computation kernel: matrix-vector like operation */
static long long matrix_vector_kernel(int *matrix, int *vector, int n) {
    long long result = 0;
    volatile int vol_iter = n / 2;
    
    for (int i = 0; i < n && i < vol_iter; i++) {
        int row_sum = 0;
        
        /* Strided memory access pattern */
        for (int j = 0; j < n; j += 2) {
            /* Non-trivial addressing mode */
            int elem = matrix[i * n + j] * vector[j];
            
            /* Complex dependency chain */
            row_sum += elem;
            
            /* Another inline assembly barrier */
            asm volatile ("" : : "r"(&elem) : "memory");
            
            /* Additional computation to increase instruction mix */
            if (j % 3 == 0) {
                row_sum -= elem / 3;
            } else {
                row_sum += elem * 2;
            }
        }
        
        /* Reduction with dependency */
        result += row_sum;
        
        /* Prevent optimization with external call */
        if (i % 100 == 0) {
            result += rand() % 10;
        }
    }
    
    return result;
}

/* Initialize with pseudo-random data */
static void init_data(int *data, int n) {
    unsigned int seed = volatile_seed;
    for (int i = 0; i < n; i++) {
        /* Simple PRNG to avoid libc call in tight loops */
        seed = seed * 1103515245 + 12345;
        data[i] = (int)(seed % 1000) - 500;
    }
}

int main(void) {
    /* Allocate and initialize data */
    int *data = (int*)malloc(SIZE * sizeof(int));
    int *matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    int *vector = (int*)malloc(SIZE * sizeof(int));
    
    if (!data || !matrix || !vector) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns */
    init_data(data, SIZE);
    init_data(matrix, SIZE * SIZE);
    init_data(vector, SIZE);
    
    /* Seed rand for volatile calls */
    srand(time(NULL));
    
    long long total = 0;
    
    /* First computation: data-dependent sum with complex loops */
    total += data_dependent_sum(data, SIZE, 3);
    
    /* Second computation: matrix-vector kernel */
    total += matrix_vector_kernel(matrix, vector, 64);  /* Use smaller size for matrix */
    
    /* Third computation: different stride pattern */
    volatile_bound = INNER_ITER / 2;
    total += data_dependent_sum(data, SIZE, 7);
    
    /* Final reduction with XOR to ensure side effect */
    long long final_result = total ^ (total >> 32);
    
    printf("Result: %lld\n", final_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
