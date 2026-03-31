/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o trigger
 * Or for more aggressive scheduling: gcc -O3 -fsel-sched-pipelining -funroll-loops -march=native -fdump-rtl-all sel-sched-trigger.c -o trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* External function to create dependencies */
extern int rand(void);

/* Complex data-dependent computation with carried dependencies */
static long long complex_kernel(int *data, int size, volatile int *vbound) {
    long long sum = 0;
    long long product = 1;
    int i, j;
    
    /* Outer loop with volatile bound */
    for (i = 0; i < *vbound; i++) {
        /* Inner loop with software pipelining potential */
        #pragma GCC unroll 4
        for (j = 1; j < size - 1; j++) {
            /* Data-dependent computation with cross-iteration dependencies */
            int prev = data[j - 1];
            int curr = data[j];
            int next = data[j + 1];
            
            /* Mixed-width operations to create register pressure */
            long long temp64 = (long long)prev * (long long)curr;
            int temp32 = (int)(temp64 & 0xFFFFFFFF);
            
            /* Conditional operations using ternary operator (potential for cmov) */
            int select = (temp32 > 1000) ? temp32 : -temp32;
            
            /* Complex arithmetic with division (non-constant divisor) */
            if (select != 0) {
                temp64 /= (select | 1);  /* Avoid division by zero */
            }
            
            /* Floating-point operations to stress FP scheduler */
            double fp_temp = (double)temp64 * 0.5;
            int int_result = (int)fp_temp;
            
            /* Pointer chasing pattern */
            int *ptr = &data[j];
            int deref = *ptr;
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : "r"(deref) : "memory");
            
            /* Update running sums with dependencies */
            sum += int_result + deref;
            product *= (temp32 % 256) | 1;  /* Avoid zero product */
            
            /* Branch with substantial computation in both paths */
            if (deref % 3 == 0) {
                /* Branch 1: More complex computation */
                sum += (next * prev) / ((curr | 1) * 2);
                product ^= (temp64 & 0xFF);
            } else {
                /* Branch 2: Different complex computation */
                sum -= (prev ^ next) * ((curr + 1) / 2);
                product |= (temp64 >> 8) & 0xFF;
            }
        }
        
        /* Switch statement creating multiple basic blocks */
        switch (i % 4) {
            case 0:
                sum += product * 2;
                /* Memory access with non-trivial addressing */
                data[i % size] = (int)(sum & 0x7FFFFFFF);
                break;
            case 1:
                sum -= product / 3;
                data[(i + 1) % size] = (int)(product & 0x7FFF);
                break;
            case 2:
                sum ^= product;
                /* Another inline assembly barrier */
                asm volatile ("" : : : "memory");
                break;
            case 3:
                sum = (sum << 1) | (product & 1);
                data[(i + 2) % size] = (int)sum;
                break;
        }
    }
    
    return sum ^ product;
}

/* Second computation kernel for additional scheduling regions */
static double matrix_vector_kernel(double *matrix, double *vector, int n, volatile int *iter) {
    double result = 0.0;
    int i, j, k;
    
    for (k = 0; k < *iter; k++) {
        /* Nested loops ideal for software pipelining */
        for (i = 0; i < n; i++) {
            double row_sum = 0.0;
            #pragma GCC unroll 2
            for (j = 0; j < n; j++) {
                /* Strided memory access pattern */
                double elem = matrix[i * n + j];
                double vec_elem = vector[j];
                
                /* Floating-point operations */
                double prod = elem * vec_elem;
                
                /* Conditional based on random-like value */
                if ((i + j + k) % 7 == 0) {
                    row_sum += prod * 1.5;
                } else {
                    row_sum += prod * 0.75;
                }
                
                /* Prevent optimization with volatile-like behavior */
                asm volatile ("" : "+m" (matrix[i * n + j]));
            }
            result += row_sum / (n + 1);
        }
        
        /* Modify vector for next iteration */
        for (i = 0; i < n; i++) {
            vector[i] = vector[i] * 0.99 + result * 0.01;
        }
    }
    
    return result;
}

int main(void) {
    const int data_size = 1024;
    const int matrix_size = 32;
    int i;
    
    /* Initialize with pseudo-random data */
    int *data = (int*)malloc(data_size * sizeof(int));
    double *matrix = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double *vector = (double*)malloc(matrix_size * sizeof(double));
    
    /* Simple PRNG for initialization */
    unsigned int seed = time(NULL) ^ volatile_seed;
    for (i = 0; i < data_size; i++) {
        seed = seed * 1103515245 + 12345;
        data[i] = (int)(seed % 10000);
    }
    
    for (i = 0; i < matrix_size * matrix_size; i++) {
        seed = seed * 1103515245 + 12345;
        matrix[i] = (double)(seed % 1000) / 1000.0;
    }
    
    for (i = 0; i < matrix_size; i++) {
        seed = seed * 1103515245 + 12345;
        vector[i] = (double)(seed % 1000) / 1000.0;
    }
    
    /* Call computation kernels with volatile bounds */
    volatile int bound1 = volatile_bound;
    volatile int bound2 = volatile_bound / 10;
    
    long long result1 = complex_kernel(data, data_size, &bound1);
    double result2 = matrix_vector_kernel(matrix, vector, matrix_size, &bound2);
    
    /* Final reduction to ensure side effects */
    unsigned long long final_result = (unsigned long long)result1 ^ 
                                     *(unsigned long long*)&result2;
    
    printf("Result: 0x%016llx\n", final_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
