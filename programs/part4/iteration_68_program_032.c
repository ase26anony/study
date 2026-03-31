/* sel-sched-trigger.c
 * Program designed to trigger selective scheduling debug dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* Simple PRNG to avoid libc rand() overhead in analysis */
static unsigned int prng_state = 123456789;
static inline unsigned int fast_rand(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Complex data-dependent computation with mixed operations */
static long long compute_kernel(int *data, int size, volatile int *bound_ptr) {
    long long sum = 0;
    double fp_acc = 0.0;
    int i, j;
    
    /* Outer loop with volatile bound */
    for (i = 0; i < *bound_ptr; i++) {
        /* Inner loop with carried dependency */
        for (j = 1; j < size - 1; j++) {
            /* Data-dependent computation with mixed-width operations */
            int val1 = data[j];
            int val2 = data[j - 1];
            int val3 = data[j + 1];
            
            /* Complex arithmetic with dependencies */
            int temp = val1 * val2;
            temp += val3 * (j & 0xFF);
            
            /* Conditional operation using ternary */
            temp = (temp > 1000) ? temp / ((val1 % 7) + 1) : temp * 2;
            
            /* Mixed 32/64-bit operations */
            long long ltemp = (long long)temp * (i + 1);
            
            /* Floating-point operation to create FPU pressure */
            fp_acc += (double)ltemp * 0.001;
            
            /* Update with dependency on previous iteration */
            sum += (long long)(fp_acc * 100.0) + ltemp;
            
            /* Inline assembly to create fixed RTL instruction */
            asm volatile ("" : : "r"(temp) : "memory");
        }
        
        /* Branch with substantial computation in both paths */
        if (i & 1) {
            /* Path A: More complex computation */
            for (j = 0; j < 4; j++) {
                int idx = (i * 17 + j * 13) % size;
                sum += data[idx] * (j + 1);
                fp_acc -= data[idx] * 0.01;
            }
        } else {
            /* Path B: Different computation pattern */
            int k = i % 8;
            switch (k) {
                case 0: sum += data[i % size] << 2; break;
                case 1: sum -= data[i % size] * 3; break;
                case 2: sum ^= data[i % size]; break;
                case 3: sum |= data[i % size]; break;
                case 4: sum &= ~data[i % size]; break;
                case 5: sum += (sum >> 3); break;
                case 6: sum *= 97; break;
                case 7: sum = (sum << 1) | (sum >> 63); break;
            }
        }
        
        /* Prevent loop unrolling beyond scheduler's control */
        asm volatile ("" : : : "memory");
    }
    
    return sum + (long long)fp_acc;
}

/* Second computation kernel with different pattern */
static long long matrix_vector_kernel(int *matrix, int *vector, int n, volatile int iter) {
    long long result = 0;
    int i, j, k;
    
    #pragma GCC unroll 4
    for (i = 0; i < iter; i++) {
        /* Matrix-vector like computation */
        for (j = 0; j < n; j++) {
            int dot = 0;
            for (k = 0; k < n; k++) {
                /* Strided access pattern */
                dot += matrix[j * n + k] * vector[k];
                
                /* Complex addressing mode */
                int idx = (j * 17 + k * 13) % (n * n);
                dot ^= matrix[idx];
            }
            
            /* Division with non-constant divisor */
            if (dot != 0) {
                result += (long long)dot / ((vector[j] & 0xF) + 1);
            }
            
            /* Conditional move simulation */
            result = (dot > 1000) ? result + 1 : result - 1;
        }
        
        /* Pointer chasing pattern */
        int *ptr = vector;
        for (j = 0; j < n; j++) {
            result += *ptr;
            ptr = &vector[*ptr % n];
        }
    }
    
    return result;
}

int main(void) {
    const int data_size = 1024;
    const int matrix_size = 32;
    int *data = malloc(data_size * sizeof(int));
    int *matrix = malloc(matrix_size * matrix_size * sizeof(int));
    int *vector = malloc(matrix_size * sizeof(int));
    
    if (!data || !matrix || !vector) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        matrix[i] = rand() % 100;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        vector[i] = rand() % 50;
    }
    
    /* Volatile iteration bounds */
    volatile int iter1 = g_volatile_bound;
    volatile int iter2 = g_volatile_bound / 2;
    
    /* First computation kernel */
    long long result1 = compute_kernel(data, data_size, &iter1);
    
    /* Second computation kernel */
    long long result2 = matrix_vector_kernel(matrix, vector, matrix_size, iter2);
    
    /* Final reduction to prevent optimization */
    long long final_result = result1 ^ result2;
    
    /* Use result to ensure side effects */
    printf("Result: %lld\n", final_result);
    
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
