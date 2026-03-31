/* sel_sched_trigger.c
 * Designed to trigger selective scheduler debug dumping in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel_sched_trigger.c -o sel_sched_trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Simple PRNG to avoid libc rand() overhead in analysis */
static unsigned int prng_state = 123456789;
static inline unsigned int fast_rand(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function with mixed operations to create scheduling complexity */
static inline int complex_op(int a, int b, int c) {
    /* Mixed-width operations */
    long long wide = (long long)a * b;
    int narrow = (int)(wide >> 16);
    
    /* Conditional move via ternary */
    int cond = (c & 1) ? narrow : -narrow;
    
    /* Division with non-constant divisor (prevoves constant propagation) */
    if (b != 0) {
        cond /= (b & 0xFF) + 1;  /* Non-zero divisor */
    }
    
    return cond;
}

/* Matrix-vector multiplication kernel */
void matvec_multiply(int *restrict result, 
                     const int *restrict matrix,
                     const int *restrict vector,
                     int rows, int cols) {
    volatile int volatile_i = 0;  /* Volatile loop counter */
    
    for (int i = 0; i < rows; i++) {
        volatile_i = i;  /* Force memory access */
        int sum = 0;
        
        /* Inner loop with carried dependency */
        for (int j = 0; j < cols; j++) {
            /* Data-dependent computation with stride access */
            int idx = i * cols + j;
            int prev_idx = (i * cols + ((j + cols - 1) % cols));
            
            /* Complex addressing mode */
            int val = matrix[idx] * vector[j];
            
            /* Dependency chain: sum depends on previous iteration */
            sum += val + (matrix[prev_idx] & 0xF);
            
            /* Conditional operation inside loop */
            if (val > 1000) {
                sum -= matrix[idx] / ((vector[j] & 0x7F) + 1);
            } else {
                sum += complex_op(matrix[idx], vector[j], sum);
            }
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : : "memory");
        }
        
        /* Store with potential aliasing */
        result[i] = sum;
    }
}

/* Pointer chasing pattern */
int pointer_chase(int *data, int size, int steps) {
    int index = 0;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        /* True pointer chasing with data dependency */
        index = data[index % size];
        sum += index;
        
        /* Mixed floating-point operations */
        float fval = (float)sum * 0.5f;
        sum += (int)fval;
        
        /* Switch statement for control flow complexity */
        switch (i & 0x3) {
            case 0:
                sum += data[(index + 1) % size] * 2;
                break;
            case 1:
                sum -= data[(index + 2) % size] / 3;
                break;
            case 2:
                sum ^= data[(index + 3) % size];
                break;
            default:
                sum |= data[(index + 4) % size];
                break;
        }
    }
    
    return sum;
}

/* Main computation kernel */
int main_kernel(int *data, int size) {
    int result = 0;
    volatile int volatile_size = size;  /* Prevent optimization */
    
    /* Outer loop */
    for (int i = 0; i < volatile_size - 1; i++) {
        int local_sum = 0;
        
        /* Inner loop with software pipelining potential */
        #pragma GCC unroll 4
        for (int j = 0; j < 64; j++) {
            /* Strong data dependency across iterations */
            int idx = (i + j) % size;
            int prev_idx = (i + j - 1 + size) % size;
            
            /* Complex computation chain */
            int a = data[idx];
            int b = data[prev_idx];
            
            /* Multiple dependent operations */
            int prod = a * b;
            int diff = a - b;
            
            /* Division with variable divisor */
            if (diff != 0) {
                prod /= (abs(diff) & 0x3F) + 1;
            }
            
            /* Floating-point operation to create FPU pressure */
            float fprod = (float)prod * 1.5f;
            prod = (int)fprod;
            
            /* Conditional update */
            local_sum += (prod > 0) ? prod : -prod;
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Reduction with dependency */
        result ^= local_sum;
        
        /* External function call to prevent optimization */
        if (i % 100 == 0) {
            result += rand() & 0xFF;  /* Use libc rand for volatility */
        }
    }
    
    return result;
}

int main(void) {
    const int data_size = 1024;
    const int matrix_rows = 64;
    const int matrix_cols = 64;
    
    /* Allocate and initialize data with pseudo-random values */
    int *data = (int*)malloc(data_size * sizeof(int));
    int *matrix = (int*)malloc(matrix_rows * matrix_cols * sizeof(int));
    int *vector = (int*)malloc(matrix_cols * sizeof(int));
    int *result = (int*)malloc(matrix_rows * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    srand(volatile_seed);
    for (int i = 0; i < data_size; i++) {
        data[i] = fast_rand() % 1000;
    }
    
    for (int i = 0; i < matrix_rows * matrix_cols; i++) {
        matrix[i] = fast_rand() % 500;
    }
    
    for (int i = 0; i < matrix_cols; i++) {
        vector[i] = fast_rand() % 300;
    }
    
    /* Run multiple kernels to increase scheduling regions */
    int total_result = 0;
    
    /* Kernel 1: Main computation */
    total_result ^= main_kernel(data, data_size);
    
    /* Kernel 2: Matrix-vector multiplication */
    matvec_multiply(result, matrix, vector, matrix_rows, matrix_cols);
    for (int i = 0; i < matrix_rows; i++) {
        total_result += result[i];
    }
    
    /* Kernel 3: Pointer chasing */
    total_result ^= pointer_chase(data, data_size, volatile_bound % 500);
    
    /* Final reduction to ensure side effects */
    total_result = (total_result << 1) | (total_result >> 31);  /* Rotate */
    
    printf("Result: %d\n", total_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
