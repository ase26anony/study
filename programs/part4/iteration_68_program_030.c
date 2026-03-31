/* sel_sched_trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* External function to create dependencies */
extern int rand_r(unsigned int *seed);

/* Complex computation with data dependencies */
static inline int64_t complex_op(int64_t a, int64_t b, int64_t c) {
    /* Mixed-width operations */
    int32_t a32 = (int32_t)a;
    int64_t b64 = b;
    
    /* Conditional move via ternary */
    int64_t cond = (a > b) ? (a * 2) : (b / 3);
    
    /* Floating point to create FPU pressure */
    double fa = (double)a;
    double fb = (double)b;
    double fc = (double)c;
    
    /* Complex floating point chain */
    fa = fa * 1.234567 + fb / 3.14159;
    fb = fb * 2.71828 - fc / 2.0;
    
    /* Integer result with dependency on FP */
    int64_t result = (int64_t)(fa + fb) + cond;
    
    /* More mixed operations */
    result = result * 7 + (a32 * b64) / 5;
    
    /* Division with non-constant divisor */
    if (c != 0) {
        result = result / (c & 0xFF + 1);
    }
    
    return result;
}

/* Pointer chasing pattern */
static int64_t chase_pointer(int64_t *data, int size, int start) {
    int64_t sum = 0;
    int idx = start;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent access pattern */
        idx = (idx * 13 + 7) % size;
        
        /* Complex computation on chased data */
        int64_t prev = (i > 0) ? data[(idx * 17) % size] : 1;
        int64_t curr = data[idx];
        
        /* Nested conditional with substantial computation */
        if (curr > prev) {
            sum += complex_op(curr, prev, sum);
        } else {
            sum -= complex_op(prev, curr, sum) / 2;
        }
        
        /* Inline assembly to create fixed RTL */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* Matrix-vector multiplication kernel */
static void matvec_multiply(double *result, 
                           double **matrix, 
                           double *vector, 
                           int rows, 
                           int cols) {
    volatile int row_bound = rows; /* Prevent optimization */
    
    #pragma GCC unroll 4
    for (int i = 0; i < row_bound; i++) {
        double sum = 0.0;
        
        /* Complex addressing mode */
        double *row = matrix[i];
        
        for (int j = 0; j < cols; j++) {
            /* Strided access with dependency */
            double val = row[j];
            double vec_val = vector[j];
            
            /* Floating point with dependency chain */
            sum = sum * 0.99 + val * vec_val;
            
            /* Conditional with computation in both branches */
            if (j % 3 == 0) {
                sum = sum / (vec_val + 1.0);
            } else {
                sum = sum * (vec_val * 0.5 + 0.5);
            }
        }
        
        /* Store with potential aliasing */
        result[i] = sum;
        
        /* Another inline asm barrier */
        asm volatile("" ::: "memory");
    }
}

/* Main computation kernel */
static int64_t compute_kernel(int64_t *data, int size) {
    int64_t total = 0;
    unsigned int seed = g_volatile_seed;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < g_volatile_bound / 100; outer++) {
        /* Inner loop with carried dependency */
        int64_t running_sum = 0;
        
        #pragma GCC unroll 2
        for (int i = 1; i < size; i++) {
            /* Data-dependent computation with cross-iteration dependency */
            int64_t prev = data[i-1];
            int64_t curr = data[i];
            
            /* Complex arithmetic chain */
            int64_t diff = curr - prev;
            int64_t prod = curr * prev;
            
            /* Division with runtime divisor */
            if (diff != 0) {
                prod = prod / (diff & 0x3F + 1);
            }
            
            /* Update running sum with dependency */
            running_sum = running_sum * 3 + prod;
            
            /* Conditional based on random value */
            if (rand_r(&seed) & 0x1) {
                running_sum = running_sum >> 2;
            } else {
                running_sum = running_sum << 1;
            }
            
            /* Memory store with potential aliasing */
            data[i] = running_sum & 0xFFFF;
        }
        
        /* Reduction with complex operation */
        total ^= complex_op(running_sum, total, seed);
        
        /* Switch-like control flow */
        switch (outer % 4) {
            case 0:
                total += chase_pointer(data, size, outer % size);
                break;
            case 1:
                total -= running_sum * 7;
                break;
            case 2:
                total = total / (running_sum & 0xFF + 1);
                break;
            default:
                total = total * 3 - running_sum;
                break;
        }
    }
    
    return total;
}

int main(void) {
    const int data_size = 1024;
    const int matrix_size = 64;
    
    /* Initialize with pseudo-random data */
    int64_t *data = (int64_t*)malloc(data_size * sizeof(int64_t));
    double *vector = (double*)malloc(matrix_size * sizeof(double));
    double **matrix = (double**)malloc(matrix_size * sizeof(double*));
    double *result = (double*)malloc(matrix_size * sizeof(double));
    
    /* Simple PRNG for initialization */
    unsigned int seed = time(NULL);
    for (int i = 0; i < data_size; i++) {
        data[i] = (int64_t)(rand_r(&seed) & 0xFFFF);
    }
    
    for (int i = 0; i < matrix_size; i++) {
        matrix[i] = (double*)malloc(matrix_size * sizeof(double));
        vector[i] = (double)(rand_r(&seed) & 0xFF) / 256.0;
        
        for (int j = 0; j < matrix_size; j++) {
            matrix[i][j] = (double)(rand_r(&seed) & 0xFF) / 256.0;
        }
    }
    
    /* First computation kernel */
    printf("Starting selective scheduling test...\n");
    int64_t result1 = compute_kernel(data, data_size);
    
    /* Second distinct kernel */
    matvec_multiply(result, matrix, vector, matrix_size, matrix_size);
    
    /* Final reduction to prevent optimization */
    double result2 = 0.0;
    for (int i = 0; i < matrix_size; i++) {
        result2 += result[i];
    }
    
    /* Mix results */
    int64_t final_result = result1 ^ (int64_t)(result2 * 1000.0);
    
    printf("Result: %ld\n", (long)final_result);
    
    /* Cleanup */
    for (int i = 0; i < matrix_size; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(vector);
    free(result);
    free(data);
    
    return 0;
}
