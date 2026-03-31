/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* External function to create dependencies */
extern int rand_r(unsigned int *seed);

/* Complex data-dependent computation with carried dependencies */
static long long complex_kernel(int *data, int size, volatile int *bound) {
    long long sum = 0;
    long long prod = 1;
    int i, j;
    
    /* Outer loop with volatile bound */
    for (i = 0; i < *bound; i++) {
        int idx = i % size;
        
        /* Inner loop with data dependencies */
        for (j = 1; j < 8; j++) {
            int prev_idx = (idx + j - 1) % size;
            int curr_idx = (idx + j) % size;
            
            /* Data-dependent computation with mixed operations */
            int val1 = data[prev_idx];
            int val2 = data[curr_idx];
            
            /* Mixed-width arithmetic to create register pressure */
            long long temp = (long long)val1 * val2;
            
            /* Conditional operation using ternary */
            temp = (val1 > val2) ? temp + (val1 - val2) : temp - (val2 - val1);
            
            /* Floating-point operation in integer context */
            double fp_temp = (double)temp / (val2 != 0 ? val2 : 1);
            
            /* Integer division with non-constant divisor */
            int int_result = (int)fp_temp;
            int_result /= (val1 % 7 + 1);  /* Non-constant divisor */
            
            /* Update running sums with dependencies */
            sum += int_result;
            prod *= (int_result & 0xFF);
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : "r"(sum), "r"(prod) : "memory");
        }
        
        /* Pointer chasing pattern */
        int next_idx = (data[idx] % size + size) % size;
        int chase_val = data[next_idx];
        
        /* Complex addressing mode */
        sum += data[(chase_val + i) % size] * data[(chase_val * 2) % size];
    }
    
    return sum ^ (prod & 0xFFFFFFFF);
}

/* Matrix-vector multiplication kernel */
static void matrix_vector_multiply(float *matrix, float *vector, float *result, 
                                   int rows, int cols, volatile int iter) {
    int i, j, k;
    
    #pragma GCC unroll 4
    for (k = 0; k < iter; k++) {
        for (i = 0; i < rows; i++) {
            float acc = 0.0f;
            
            /* Unrolled inner loop */
            for (j = 0; j < cols; j += 4) {
                /* Non-trivial addressing with stride */
                float m1 = matrix[i * cols + j];
                float m2 = (j + 1 < cols) ? matrix[i * cols + j + 1] : 0.0f;
                float m3 = (j + 2 < cols) ? matrix[i * cols + j + 2] : 0.0f;
                float m4 = (j + 3 < cols) ? matrix[i * cols + j + 3] : 0.0f;
                
                float v1 = vector[j];
                float v2 = (j + 1 < cols) ? vector[j + 1] : 0.0f;
                float v3 = (j + 2 < cols) ? vector[j + 2] : 0.0f;
                float v4 = (j + 3 < cols) ? vector[j + 3] : 0.0f;
                
                /* Fused multiply-add pattern */
                acc += m1 * v1 + m2 * v2 + m3 * v3 + m4 * v4;
                
                /* Conditional branch with computation */
                if (acc > 1000.0f) {
                    acc *= 0.99f;
                } else {
                    acc += 0.01f * (float)(i + j);
                }
            }
            
            result[i] += acc;
            
            /* Another inline assembly barrier */
            asm volatile ("" : : "r"(acc), "r"(result[i]) : "memory");
        }
    }
}

/* Switch-based computation with multiple basic blocks */
static int switch_computation(int value, int *data, int size) {
    int result = 0;
    
    switch (value % 5) {
        case 0:
            /* Basic block 0: Integer operations */
            for (int i = 0; i < size; i++) {
                result += data[i] * data[(i + 1) % size];
                result ^= data[i] << (i % 16);
            }
            break;
            
        case 1:
            /* Basic block 1: Floating point */
            for (int i = 0; i < size; i++) {
                double temp = (double)data[i] / (data[(i + 2) % size] + 1);
                result += (int)(temp * 1000.0);
            }
            break;
            
        case 2:
            /* Basic block 2: Mixed operations */
            for (int i = 0; i < size; i++) {
                int idx1 = data[i] % size;
                int idx2 = data[(i + 3) % size] % size;
                result += (data[idx1] * data[idx2]) / (i + 1);
            }
            break;
            
        case 3:
            /* Basic block 3: Pointer arithmetic */
            int *ptr = data;
            for (int i = 0; i < size; i++) {
                result += *ptr;
                ptr += (data[i] % 3) + 1;
                if (ptr >= data + size) ptr = data;
            }
            break;
            
        case 4:
            /* Basic block 4: Complex dependencies */
            for (int i = 1; i < size; i++) {
                int dep = result % 256;
                result = (result * data[i]) + (data[i - 1] ^ dep);
                result = (result >> 4) | (result << 28);  /* Rotate */
            }
            break;
    }
    
    return result;
}

int main(void) {
    const int DATA_SIZE = 1024;
    const int MATRIX_SIZE = 64;
    
    /* Initialize with pseudo-random data */
    int *data = (int*)malloc(DATA_SIZE * sizeof(int));
    float *matrix = (float*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float));
    float *vector = (float*)malloc(MATRIX_SIZE * sizeof(float));
    float *result = (float*)malloc(MATRIX_SIZE * sizeof(float));
    
    unsigned int seed = g_volatile_seed;
    
    /* Fill arrays with random data */
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = rand_r(&seed) % 1000;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (float)(rand_r(&seed) % 100) / 10.0f;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        vector[i] = (float)(rand_r(&seed) % 100) / 10.0f;
        result[i] = 0.0f;
    }
    
    /* Run multiple computation kernels */
    long long kernel1_result = 0;
    int kernel2_result = 0;
    
    /* Use volatile iteration count */
    volatile int iterations = g_volatile_bound % 100;
    
    /* First kernel: complex data-dependent computation */
    kernel1_result = complex_kernel(data, DATA_SIZE, &iterations);
    
    /* Second kernel: matrix-vector multiplication */
    matrix_vector_multiply(matrix, vector, result, MATRIX_SIZE, MATRIX_SIZE, iterations);
    
    /* Third kernel: switch-based computation */
    for (int i = 0; i < iterations; i++) {
        kernel2_result ^= switch_computation(data[i % DATA_SIZE], data, DATA_SIZE);
    }
    
    /* Final reduction to prevent optimization */
    long long final_result = kernel1_result ^ kernel2_result;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        final_result += (long long)result[i];
    }
    
    printf("Final result: %lld\n", final_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
