/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* External function to create dependencies */
extern int rand(void);

/* Complex data-dependent computation with carried dependencies */
static long long complex_loop(int* data, int size, volatile int* bound) {
    long long sum = 0;
    int local_bound = *bound; /* Volatile read */
    
    /* Outer loop with volatile control */
    for (int i = 1; i < local_bound; i += (rand() % 3 + 1)) {
        int inner_limit = size - 1;
        
        /* Inner loop with multiple dependency chains */
        #pragma GCC unroll 4
        for (int j = 1; j < inner_limit; j++) {
            /* Data-dependent computation with cross-iteration dependencies */
            int val1 = data[j];
            int val2 = data[j - 1];
            
            /* Mixed-width operations */
            long long product = (long long)val1 * val2;
            long long accum = product + (sum & 0xFFFFFFFF);
            
            /* Conditional operations */
            int cond = val1 > val2 ? val1 : val2;
            accum += (cond % 7) * 3;
            
            /* Floating-point operations to create FPU pressure */
            double fp_val = (double)val1 / (val2 != 0 ? val2 : 1);
            accum += (long long)(fp_val * 100.0);
            
            /* Pointer chasing pattern */
            int* ptr = &data[j];
            accum += *ptr;
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("# Fixed instruction point %0" : : "r"(accum) : "memory");
            
            /* Update sum with dependency */
            sum = (sum * 3 + accum) % 0x7FFFFFFF;
            
            /* Branch with computation in both paths */
            if (val1 & 0x1) {
                /* Complex branch path */
                sum += (val1 << 3) | (val2 & 0xF);
                sum ^= (product >> 16);
            } else {
                /* Alternative path with different operations */
                sum -= (val1 * 7) / (val2 != 0 ? val2 : 1);
                sum |= (val2 << 8);
            }
            
            /* Switch statement creating multiple basic blocks */
            switch (val1 % 4) {
                case 0:
                    sum += val1 * val1;
                    break;
                case 1:
                    sum ^= val2 * 3;
                    break;
                case 2:
                    sum = (sum << 1) | (sum >> 31);
                    break;
                case 3:
                    sum = sum / (val1 != 0 ? val1 : 1);
                    break;
            }
        }
        
        /* Update data array with stride pattern */
        for (int k = 0; k < size; k += 8) {
            data[k] = (data[k] + i) & 0xFF;
        }
    }
    
    return sum;
}

/* Second computation kernel: matrix-vector like operation */
static long long matrix_vector_kernel(int* matrix, int* vector, int n, volatile int iter) {
    long long result = 0;
    
    for (int it = 0; it < iter; it += (rand() % 2 + 1)) {
        for (int i = 0; i < n; i++) {
            int row_sum = 0;
            
            #pragma GCC unroll 2
            for (int j = 0; j < n; j++) {
                /* Non-contiguous memory access pattern */
                int idx = (i * n + j) % (n * n);
                row_sum += matrix[idx] * vector[j];
                
                /* Division with non-constant divisor */
                if (row_sum != 0) {
                    row_sum /= (abs(vector[j]) + 1);
                }
            }
            
            /* Complex accumulation with mixed operations */
            result += (long long)row_sum * (it + 1);
            result ^= (result << 13) | (result >> 51);
            
            /* Another inline assembly point */
            asm volatile ("# Matrix computation %0" : : "r"(row_sum) : "memory");
        }
        
        /* Modify vector occasionally */
        if (it % 7 == 0) {
            for (int v = 0; v < n; v++) {
                vector[v] = (vector[v] + matrix[it % (n * n)]) & 0x3FF;
            }
        }
    }
    
    return result;
}

int main(void) {
    const int data_size = 512;
    const int matrix_size = 32;
    
    /* Initialize with pseudo-random data */
    int* data = (int*)malloc(data_size * sizeof(int));
    int* matrix = (int*)malloc(matrix_size * matrix_size * sizeof(int));
    int* vector = (int*)malloc(matrix_size * sizeof(int));
    
    srand(g_volatile_seed);
    
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 256;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = rand() % 128;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = rand() % 64;
    }
    
    /* Volatile iteration counter */
    volatile int iterations = g_volatile_bound;
    
    /* First complex computation */
    long long result1 = complex_loop(data, data_size, &iterations);
    
    /* Adjust bound for second kernel */
    iterations = g_volatile_bound / 4;
    
    /* Second computation kernel */
    long long result2 = matrix_vector_kernel(matrix, vector, matrix_size, iterations);
    
    /* Final reduction with side effects */
    long long final_result = result1 ^ result2;
    final_result += (long long)data[0] * vector[0];
    
    /* Ensure side effect is observable */
    printf("Result: %lld\n", final_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    
    return (final_result != 0) ? 0 : 1;
}
