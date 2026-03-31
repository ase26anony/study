/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
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
static long long complex_loop(int *data, int size, volatile int *vbound) {
    long long sum = 0;
    long long product = 1;
    int i, j;
    
    /* Outer loop with volatile bound */
    for (i = 0; i < *vbound; i++) {
        /* Inner loop with data dependency across iterations */
        for (j = 1; j < size - 1; j++) {
            /* Data-dependent computation with cross-iteration dependency */
            int prev = data[j - 1];
            int curr = data[j];
            int next = data[j + 1];
            
            /* Mixed-width operations creating register pressure */
            long long temp = (long long)prev * curr;
            temp += (long long)curr * next;
            
            /* Conditional move via ternary operator */
            int cond = (prev & 0x1) ? 1 : -1;
            temp *= cond;
            
            /* Floating-point operations mixed with integer */
            double fp_temp = (double)temp / (curr != 0 ? curr : 1);
            sum += (long long)fp_temp;
            
            /* Complex addressing mode */
            product *= data[(i * 17 + j * 13) % size];
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : "r"(temp), "r"(product) : "memory");
        }
        
        /* Branch with substantial computation in both paths */
        if (data[i % size] > 100) {
            /* Branch 1: floating-point intensive */
            double div_result = (double)sum / (i + 1);
            sum += (long long)(div_result * 1000.0);
            
            /* Switch statement creating multiple basic blocks */
            switch (data[i % size] % 4) {
                case 0:
                    product ^= (data[(i + 1) % size] << 2);
                    break;
                case 1:
                    product |= (data[(i + 2) % size] * 3);
                    break;
                case 2:
                    product &= ~(data[(i + 3) % size]);
                    break;
                case 3:
                    product += (data[(i + 4) % size] / 2);
                    break;
            }
        } else {
            /* Branch 2: integer intensive with division */
            int divisor = (i % 7) + 1;
            sum /= (divisor != 0 ? divisor : 1);
            product = (product * 31) % 0x7FFFFFFF;
            
            /* Pointer chasing pattern */
            int idx = i;
            for (int k = 0; k < 3; k++) {
                idx = data[idx % size] % size;
                sum += data[idx];
            }
        }
        
        /* Prevent loop unrolling */
        asm volatile ("" : : : "memory");
    }
    
    return sum ^ (product & 0xFFFFFFFF);
}

/* Second computation kernel: matrix-vector multiplication pattern */
static long long matrix_vector_kernel(int *matrix, int *vector, int n, volatile int iter) {
    long long result = 0;
    int i, j, k;
    
    #pragma GCC unroll 4
    for (i = 0; i < iter; i++) {
        for (j = 0; j < n; j++) {
            int dot = 0;
            /* Inner loop with dependency chain */
            for (k = 0; k < n; k++) {
                /* Strided memory access */
                dot += matrix[j * n + k] * vector[k];
                
                /* Mixed operations */
                if (k % 3 == 0) {
                    dot = (dot * 7) / 5;
                } else if (k % 3 == 1) {
                    dot = (dot << 2) | (dot >> 30);
                } else {
                    dot = dot ^ (matrix[(j * n + k) % (n * n)]);
                }
            }
            
            /* Conditional accumulation */
            result += (dot > 0) ? dot : -dot;
            
            /* Non-trivial addressing with modulo */
            vector[j] = (vector[j] + matrix[(i * n + j) % (n * n)]) % 1000;
        }
        
        /* External dependency */
        if (i % 100 == 0) {
            vector[0] ^= rand() % 256;
        }
    }
    
    return result;
}

/* Initialize array with pseudo-random data */
static void init_array(int *arr, int size, int seed) {
    /* Simple PRNG to avoid library call overhead in loops */
    unsigned int state = seed;
    for (int i = 0; i < size; i++) {
        state = state * 1103515245 + 12345;
        arr[i] = (state >> 16) & 0x7FFF;
    }
}

int main(void) {
    const int array_size = 1024;
    const int matrix_size = 32;
    int *data = malloc(array_size * sizeof(int));
    int *matrix = malloc(matrix_size * matrix_size * sizeof(int));
    int *vector = malloc(matrix_size * sizeof(int));
    
    if (!data || !matrix || !vector) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    init_array(data, array_size, volatile_seed);
    init_array(matrix, matrix_size * matrix_size, rand());
    init_array(vector, matrix_size, rand());
    
    /* First computation kernel */
    printf("Starting complex loop...\n");
    long long result1 = complex_loop(data, array_size, &volatile_bound);
    
    /* Second computation kernel */
    printf("Starting matrix-vector kernel...\n");
    volatile int matrix_iter = 50;
    long long result2 = matrix_vector_kernel(matrix, vector, matrix_size, matrix_iter);
    
    /* Final reduction to prevent optimization */
    long long final_result = result1 ^ result2;
    printf("Final result: %lld\n", final_result);
    
    /* Additional stress test with different data patterns */
    {
        /* Create aliasing pointers to confuse the optimizer */
        int *alias1 = data;
        int *alias2 = data + array_size/2;
        
        long long alias_sum = 0;
        for (int i = 0; i < array_size/2; i++) {
            /* Potential aliasing creates memory dependencies */
            *alias1 = *alias2 + i;
            alias_sum += *alias1 * *alias2;
            alias1++;
            alias2++;
            
            /* Periodic inline assembly barrier */
            if (i % 8 == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        final_result ^= alias_sum;
    }
    
    printf("Final XOR result: %lld\n", final_result);
    
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
