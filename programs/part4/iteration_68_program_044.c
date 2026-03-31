/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumping in GCC.
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-test
 * For more verbose output: add -fdump-rtl-all
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* External function to create dependencies */
extern int rand(void);

/* Complex data-dependent computation with carried dependencies */
static int64_t compute_kernel(int32_t* data, int size, volatile int* vbound) {
    int64_t sum = 0;
    int64_t product = 1;
    int32_t prev = data[0];
    
    /* Use volatile bound to prevent loop unrolling before scheduling */
    int bound = *vbound;
    
    /* First loop: data-dependent computations with mixed operations */
    for (int i = 1; i < bound && i < size; i++) {
        /* Data dependency across iterations */
        int32_t current = data[i];
        
        /* Mixed-width arithmetic to create register pressure */
        sum += (int64_t)current * (int64_t)prev;
        
        /* Conditional operation using ternary */
        product *= (current > prev) ? (int64_t)current : (int64_t)(prev + 1);
        
        /* Complex addressing mode */
        int32_t neighbor = data[(i * 13 + 7) % size];
        
        /* Floating-point operations mixed with integer */
        double temp = (double)current / (double)(neighbor + 1);
        sum += (int64_t)(temp * 100.0);
        
        /* Inline assembly to create fixed RTL instructions */
        asm volatile ("" : : "r"(current), "r"(prev) : "memory");
        
        prev = current;
        
        /* Branch with substantial computation in both paths */
        if (current % 3 == 0) {
            /* Path 1: More complex arithmetic */
            sum -= (int64_t)data[(i * 17) % size] << 2;
            product /= (neighbor != 0) ? neighbor : 1;
        } else {
            /* Path 2: Different operations */
            sum ^= (int64_t)data[(i * 19) % size];
            product += (int64_t)(current % 7);
        }
    }
    
    return sum ^ product;
}

/* Matrix-vector multiplication kernel for additional scheduling regions */
static void matrix_vector_multiply(float* matrix, float* vector, float* result, 
                                   int rows, int cols, volatile int* viter) {
    int iterations = *viter;
    
    for (int iter = 0; iter < iterations; iter++) {
        #pragma GCC unroll 4
        for (int i = 0; i < rows; i++) {
            float acc = 0.0f;
            
            /* Inner loop with stride access */
            for (int j = 0; j < cols; j++) {
                /* Non-contiguous memory access pattern */
                acc += matrix[i * cols + j] * vector[(j * 7) % cols];
                
                /* Conditional with floating point ops */
                if (acc > 1000.0f) {
                    acc *= 0.99f;
                } else {
                    acc += matrix[(i * cols + j) % (rows * cols)] * 0.01f;
                }
                
                /* Another inline assembly barrier */
                asm volatile ("" : : "r"(acc) : "memory");
            }
            
            /* Complex reduction with dependency */
            result[i] = acc + (result[(i + 1) % rows] * 0.1f);
        }
        
        /* Modify vector for next iteration */
        for (int k = 0; k < cols; k++) {
            vector[k] = vector[k] * 0.9f + result[k % rows] * 0.1f;
        }
    }
}

/* Switch statement with multiple computation paths */
static int64_t switch_computation(int32_t* data, int index, int size) {
    int64_t result = 0;
    
    switch (data[index] % 5) {
        case 0:
            /* Complex case 0 */
            for (int i = 0; i < 8 && (index + i) < size; i++) {
                result += (int64_t)data[index + i] << (i % 4);
            }
            result = (result * 3) / 2;
            break;
            
        case 1:
            /* Complex case 1 */
            result = (int64_t)data[index] * data[(index * 2) % size];
            for (int i = 0; i < 4; i++) {
                result ^= (int64_t)data[(index + i * 3) % size] << 8;
            }
            break;
            
        case 2:
            /* Complex case 2 with division */
            result = (int64_t)data[index];
            for (int i = 1; i < 6; i++) {
                int divisor = data[(index + i) % size];
                result /= (divisor != 0) ? divisor : 1;
                result += 0xABCD;
            }
            break;
            
        case 3:
            /* Complex case 3 with mixed operations */
            result = 1;
            for (int i = 0; i < 7 && (index + i) < size; i++) {
                result *= (int64_t)data[index + i] + (i * 2);
                result = (result << 3) | (result >> 61); /* 64-bit rotate */
            }
            break;
            
        default:
            /* Default case with pointer chasing */
            int pos = index;
            for (int i = 0; i < 10; i++) {
                result += data[pos];
                pos = (pos * 13 + 7) % size;
                asm volatile ("" : : "r"(pos) : "memory");
            }
            break;
    }
    
    return result;
}

int main(void) {
    const int data_size = 10000;
    const int matrix_rows = 64;
    const int matrix_cols = 64;
    
    /* Initialize with pseudo-random data */
    int32_t* data = (int32_t*)malloc(data_size * sizeof(int32_t));
    float* matrix = (float*)malloc(matrix_rows * matrix_cols * sizeof(float));
    float* vector = (float*)malloc(matrix_cols * sizeof(float));
    float* result = (float*)malloc(matrix_rows * sizeof(float));
    
    /* Simple PRNG for reproducibility */
    unsigned int seed = time(NULL) ^ volatile_seed;
    for (int i = 0; i < data_size; i++) {
        seed = seed * 1103515245 + 12345;
        data[i] = (int32_t)(seed % 1000);
    }
    
    for (int i = 0; i < matrix_rows * matrix_cols; i++) {
        seed = seed * 1103515245 + 12345;
        matrix[i] = (float)(seed % 100) / 10.0f;
    }
    
    for (int i = 0; i < matrix_cols; i++) {
        seed = seed * 1103515245 + 12345;
        vector[i] = (float)(seed % 50) / 5.0f;
    }
    
    /* Run multiple computation kernels to create multiple scheduling regions */
    int64_t total_result = 0;
    
    /* Kernel 1: Data-dependent computation with carried dependencies */
    total_result ^= compute_kernel(data, data_size, &volatile_bound);
    
    /* Kernel 2: Matrix operations */
    volatile int matrix_iterations = 5;
    matrix_vector_multiply(matrix, vector, result, matrix_rows, matrix_cols, &matrix_iterations);
    
    /* Kernel 3: Switch-based computations */
    for (int i = 0; i < 1000; i += 37) {
        total_result += switch_computation(data, i % data_size, data_size);
    }
    
    /* Kernel 4: Nested loops with complex addressing */
    int64_t reduction = 0;
    for (int i = 0; i < volatile_bound && i < data_size - 100; i += 3) {
        int64_t block_sum = 0;
        for (int j = 0; j < 50; j++) {
            /* Non-linear addressing with mixed operations */
            int idx = (i * j + 7) % data_size;
            block_sum += (int64_t)data[idx] * (int64_t)data[(idx + 1) % data_size];
            
            /* Floating point in inner loop */
            double ratio = (data[idx] != 0) ? (double)data[(idx + 2) % data_size] / data[idx] : 1.0;
            block_sum += (int64_t)(ratio * 1000.0);
            
            /* Conditional with both paths having computation */
            if ((i + j) % 11 == 0) {
                block_sum ^= (int64_t)data[(idx * 3) % data_size] << 4;
            } else {
                block_sum |= (int64_t)data[(idx * 5) % data_size];
            }
        }
        reduction ^= block_sum;
    }
    
    total_result ^= reduction;
    
    /* Final result to prevent optimization */
    printf("Result: %ld\n", (long)total_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
