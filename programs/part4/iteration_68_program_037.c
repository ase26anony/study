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

/* Complex data-dependent computation with mixed operations */
static int64_t compute_kernel(int32_t* data, int size, volatile int* vbound) {
    int64_t sum = 0;
    int64_t product = 1;
    double fp_acc = 0.0;
    
    /* Outer loop with volatile bound */
    for (int i = 0; i < *vbound; i++) {
        int idx = i % size;
        
        /* Inner loop with carried dependency */
        #pragma GCC unroll 4
        for (int j = 0; j < 8; j++) {
            /* Data-dependent computation with mixed-width operations */
            int32_t val = data[(idx + j) % size];
            int32_t prev_val = data[(idx + j - 1 + size) % size];
            
            /* Complex arithmetic with dependencies */
            int64_t temp = (int64_t)val * (int64_t)prev_val;
            sum += temp;
            
            /* Conditional operation using ternary */
            product *= (val > 0) ? val : (val < 0 ? -val : 1);
            
            /* Floating-point operations */
            fp_acc += (val % 256) * 0.01;
            
            /* Mixed-width arithmetic creating register pressure */
            sum += (sum >> 32) + (sum & 0xFFFFFFFF);
            
            /* Division with non-constant divisor (prevents optimization) */
            if (prev_val != 0) {
                sum /= (prev_val & 0xFF) + 1;
            }
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : "r"(val), "r"(prev_val) : "memory");
        }
        
        /* Control flow with multiple basic blocks */
        switch (i & 0x3) {
            case 0:
                sum += data[idx] * 2;
                fp_acc *= 1.01;
                break;
            case 1:
                sum -= data[idx] / 3;
                fp_acc /= 1.02;
                break;
            case 2:
                sum ^= data[idx];
                fp_acc = fp_acc * fp_acc;
                break;
            default:
                sum = (sum << 3) | (sum >> 61);
                fp_acc += 0.5;
                break;
        }
        
        /* Additional conditional with both branches having computation */
        if (sum & 0x1000) {
            product = (product * 3) / 2;
            fp_acc = fp_acc - 1.0;
        } else {
            product = (product + 1000) * 2;
            fp_acc = fp_acc + 2.0;
        }
    }
    
    /* Final reduction mixing integer and floating-point */
    return sum + (int64_t)fp_acc + product;
}

/* Second computation kernel with different pattern */
static double matrix_vector_kernel(double* matrix, double* vector, int n, volatile int* iter) {
    double result = 0.0;
    
    for (int k = 0; k < *iter; k++) {
        double temp_sum = 0.0;
        
        /* Nested loops for matrix-vector multiplication */
        for (int i = 0; i < n; i++) {
            double row_sum = 0.0;
            for (int j = 0; j < n; j++) {
                /* Strided access pattern */
                row_sum += matrix[i * n + j] * vector[j];
                
                /* Data-dependent floating-point operation */
                row_sum = (row_sum > 1e6) ? row_sum * 0.5 : row_sum * 1.5;
                
                /* Inline assembly barrier */
                asm volatile ("" : : "r"(row_sum) : "memory");
            }
            
            /* Complex conditional with computation in both paths */
            if (row_sum > 0) {
                temp_sum += row_sum / (i + 1);
                vector[i] = row_sum * 0.99;
            } else {
                temp_sum -= row_sum * (i + 1);
                vector[i] = row_sum * 1.01;
            }
            
            /* Mixed precision operations */
            temp_sum = (float)temp_sum + (double)((int)temp_sum % 100);
        }
        
        result += temp_sum / n;
        
        /* Pointer chasing pattern */
        double* ptr = vector;
        for (int m = 0; m < n; m++) {
            *ptr = *ptr * 0.9 + 0.1;
            ptr = &vector[(m * 13 + 7) % n];  /* Non-linear access */
        }
    }
    
    return result;
}

int main() {
    const int data_size = 1024;
    const int matrix_size = 32;
    
    /* Initialize data with pseudo-random values */
    int32_t* data = (int32_t*)malloc(data_size * sizeof(int32_t));
    double* matrix = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double* vector = (double*)malloc(matrix_size * sizeof(double));
    
    /* Use volatile seed to prevent compile-time computation */
    srand(volatile_seed);
    
    for (int i = 0; i < data_size; i++) {
        data[i] = simple_rand() % 1000 - 500;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = (double)(simple_rand() % 1000) / 100.0;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = (double)(simple_rand() % 100) / 10.0;
    }
    
    /* Execute first kernel with volatile bound */
    volatile int bound1 = volatile_bound;
    int64_t result1 = compute_kernel(data, data_size, &bound1);
    
    /* Execute second kernel with different volatile bound */
    volatile int bound2 = volatile_bound / 2;
    double result2 = matrix_vector_kernel(matrix, vector, matrix_size, &bound2);
    
    /* Final reduction to prevent dead code elimination */
    uint64_t final_result = (uint64_t)result1 ^ (uint64_t)result2;
    printf("Final result: 0x%016lx\n", final_result);
    
    /* Additional computation to increase scheduling regions */
    int64_t checksum = 0;
    for (int i = 0; i < data_size; i++) {
        checksum += data[i];
        checksum = (checksum << 5) | (checksum >> 59);  /* Rotate */
        
        /* Complex conditional with inline assembly */
        if (checksum & 1) {
            asm volatile ("" : : "r"(checksum) : "memory");
            checksum *= 3;
        } else {
            checksum /= 2;
        }
    }
    
    printf("Checksum: 0x%016lx\n", (uint64_t)checksum);
    
    free(data);
    free(matrix);
    free(vector);
    
    return (final_result & 0xFF);
}
