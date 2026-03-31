/* sel_sched_trigger.c
 * Designed to trigger selective scheduler debug dumping in GCC's sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel_sched_trigger.c -o sel_sched_trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 100;
volatile int volatile_seed = 42;

/* External dependency to prevent constant propagation */
extern int get_external_value(void);

/* Simple PRNG to create data dependencies */
static inline uint32_t prng(uint32_t *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Complex computation with data dependencies */
static int64_t compute_kernel(int *data, int size, volatile int bound) {
    int64_t sum = 0;
    int64_t product = 1;
    uint32_t rng_state = time(NULL);
    
    /* Nested loops with carried dependencies */
    for (int i = 1; i < bound; i++) {
        int inner_bound = bound % 32 + 8; /* Non-constant bound */
        
        #pragma GCC unroll 4
        for (int j = 0; j < inner_bound; j++) {
            /* Data-dependent computation with mixed-width operations */
            int idx = (i * 17 + j * 13) % size;
            int prev_idx = (idx - 1 + size) % size;
            
            /* Complex addressing mode */
            int32_t val1 = data[idx];
            int32_t val2 = data[prev_idx];
            
            /* Mixed integer operations */
            int64_t temp = (int64_t)val1 * (int64_t)val2;
            
            /* Conditional operations */
            int64_t cond_result = (val1 > val2) ? 
                (temp >> 3) : (temp << 2);
            
            /* Floating-point operations to create FPU pressure */
            double fp_temp = (double)val1 * 1.5;
            int fp_int = (int)(fp_temp * 0.75);
            
            /* Division with non-constant divisor */
            if (val2 != 0) {
                cond_result /= (val2 & 0xFF) + 1;
            }
            
            /* Running sum with dependency chain */
            sum += cond_result + fp_int;
            
            /* Pointer chasing pattern */
            product *= (data[(product & 0xFF) % size] & 0x3F) + 1;
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile("" : "+r"(sum), "+r"(product) : : "memory");
            
            /* External dependency */
            if ((j & 0xF) == 0) {
                volatile_seed = get_external_value();
            }
        }
        
        /* Switch-like control flow */
        switch (i & 0x7) {
            case 0:
                sum += product * 2;
                break;
            case 1:
                sum -= product / 3;
                break;
            case 2:
                sum ^= product;
                break;
            case 3:
                sum |= (product & 0xFFFF);
                break;
            case 4:
                sum = (sum << 1) | (product & 1);
                break;
            case 5:
                sum = (sum >> 2) + product;
                break;
            case 6:
                sum = sum * 3 - product;
                break;
            case 7:
                sum = (sum + product) * 5;
                break;
        }
        
        /* Volatile access to prevent reordering */
        asm volatile("" : : "r"(volatile_seed) : "memory");
    }
    
    return sum ^ product;
}

/* Second computation kernel - matrix-vector like */
static double matrix_vector_kernel(double *matrix, double *vector, int n, volatile int iter) {
    double result = 0.0;
    
    for (int k = 0; k < iter; k++) {
        double temp_sum = 0.0;
        
        /* Nested loops with stride access */
        for (int i = 0; i < n; i++) {
            double row_sum = 0.0;
            
            #pragma GCC unroll 2
            for (int j = 0; j < n; j++) {
                /* Non-unit stride access */
                int idx = (i * n + (j * 3) % n) % (n * n);
                
                /* Floating-point operations */
                double val = matrix[idx] * vector[j];
                
                /* Conditional floating-point operation */
                row_sum += (val > 0.0) ? val : -val * 0.5;
                
                /* Dependency chain */
                vector[j] = vector[j] * 0.99 + matrix[idx] * 0.01;
            }
            
            /* Reduction with mixed operations */
            temp_sum += row_sum / (i + 1);
            
            /* Inline assembly barrier */
            asm volatile("" : "+r"(temp_sum) : : "memory");
        }
        
        result += temp_sum;
        
        /* Volatile bound check */
        if (k < volatile_bound) {
            result *= 1.0001;
        }
    }
    
    return result;
}

/* External dependency simulation */
int get_external_value(void) {
    static int counter = 0;
    return counter++ & 0xFF;
}

int main(void) {
    const int data_size = 1024;
    const int matrix_size = 64;
    
    /* Initialize with pseudo-random data */
    int *data = (int*)malloc(data_size * sizeof(int));
    double *matrix = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double *vector = (double*)malloc(matrix_size * sizeof(double));
    
    if (!data || !matrix || !vector) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    uint32_t rng = 123456789;
    
    /* Fill arrays with PRNG data */
    for (int i = 0; i < data_size; i++) {
        data[i] = (int)(prng(&rng) % 1000) - 500;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = (double)(prng(&rng) % 1000) / 100.0 - 5.0;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = (double)(prng(&rng) % 1000) / 200.0 - 2.5;
    }
    
    /* Volatile iteration bounds */
    volatile int iter1 = volatile_bound + 50;
    volatile int iter2 = volatile_bound / 2 + 25;
    
    /* First computation kernel */
    int64_t result1 = compute_kernel(data, data_size, iter1);
    
    /* Second computation kernel */
    double result2 = matrix_vector_kernel(matrix, vector, matrix_size, iter2);
    
    /* Final reduction to prevent optimization */
    int64_t final_result = result1 ^ (int64_t)(result2 * 1000.0);
    
    /* Use result to ensure side effects */
    printf("Result: %ld\n", (long)final_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
