/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug dumping in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* External function to create dependencies */
extern int rand_r(unsigned int *seed);

/* Complex data-dependent computation with carried dependencies */
static long long complex_kernel(int *data, int size, volatile int *vbound) {
    long long sum = 0;
    int local_seed = volatile_seed;
    
    /* Outer loop with volatile bound */
    for (int i = 0; i < *vbound; i++) {
        int idx = rand_r((unsigned int*)&local_seed) % size;
        
        /* Inner loop with data dependency across iterations */
        #pragma GCC unroll 4
        for (int j = 0; j < 64; j++) {
            /* Complex addressing with mixed-width operations */
            int idx1 = (idx + j) % size;
            int idx2 = (idx + j * 3) % size;
            int idx3 = (idx + j * 7) % size;
            
            /* Data-dependent computation with multiple operation types */
            int val1 = data[idx1];
            int val2 = data[idx2];
            int val3 = data[idx3];
            
            /* Integer operations with dependencies */
            int temp = val1 * val2;
            temp += val3 * (j & 0xF);
            
            /* Conditional operations creating control flow */
            if (temp > 1000) {
                /* Floating point operations to create FPU pressure */
                float ftemp = (float)temp / (val2 != 0 ? val2 : 1);
                sum += (long long)(ftemp * 100.0f);
                
                /* Inline assembly to create fixed RTL patterns */
                asm volatile ("" : : "r"(ftemp) : "memory");
            } else {
                /* Different computation path */
                temp = (temp << 3) | (temp >> 5);  /* Rotate */
                sum += temp * (val1 % 16);
            }
            
            /* Pointer chasing pattern */
            idx = (idx * 13 + 7) % size;
            
            /* Mixed 32/64-bit operations */
            sum += (long long)val1 * (long long)val2;
            
            /* Division with non-constant divisor (prevents optimization) */
            if (val3 != 0) {
                sum /= (val3 & 0x3F) + 1;
            }
        }
        
        /* Additional control flow with switch statement */
        switch (i % 4) {
            case 0:
                sum += data[i % size] * 2;
                break;
            case 1:
                sum -= data[(i + 1) % size] / 3;
                break;
            case 2:
                sum ^= data[(i + 2) % size];
                break;
            case 3:
                sum |= data[(i + 3) % size];
                break;
        }
    }
    
    return sum;
}

/* Second computation kernel - matrix-vector like operation */
static long long matrix_like_kernel(int *matrix, int *vector, int n, volatile int iter) {
    long long result = 0;
    
    for (int k = 0; k < iter; k++) {
        #pragma GCC unroll 2
        for (int i = 0; i < n; i++) {
            int row_sum = 0;
            
            /* Strided memory access pattern */
            for (int j = 0; j < n; j++) {
                int idx = (i * n + j * 3) % (n * n);
                row_sum += matrix[idx] * vector[j % n];
                
                /* Complex conditional with ternary operator */
                row_sum = (row_sum > 1000000) ? row_sum / 2 : row_sum * 3;
            }
            
            /* Floating point conversion and back */
            float fsum = (float)row_sum;
            fsum = fsum * 1.5f - 0.5f;
            result += (long long)fsum;
            
            /* Another inline assembly barrier */
            asm volatile ("" : : "r"(fsum), "r"(row_sum) : "memory");
        }
        
        /* Modify vector for next iteration */
        for (int i = 0; i < n; i++) {
            vector[i] = (vector[i] * 13 + matrix[i]) % 1000;
        }
    }
    
    return result;
}

int main(void) {
    const int DATA_SIZE = 1024;
    const int MATRIX_SIZE = 32;
    
    /* Initialize with pseudo-random data */
    int *data = (int*)malloc(DATA_SIZE * sizeof(int));
    int *matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int *vector = (int*)malloc(MATRIX_SIZE * sizeof(int));
    
    if (!data || !matrix || !vector) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Simple PRNG for initialization */
    unsigned int seed = time(NULL);
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = rand_r(&seed) % 10000;
    }
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = rand_r(&seed) % 1000;
    }
    for (int i = 0; i < MATRIX_SIZE; i++) {
        vector[i] = rand_r(&seed) % 500;
    }
    
    /* Volatile iteration counters */
    volatile int iter1 = volatile_bound / 2;
    volatile int iter2 = volatile_bound / 4;
    
    /* First complex kernel */
    long long result1 = complex_kernel(data, DATA_SIZE, &iter1);
    
    /* Second matrix-like kernel */
    long long result2 = matrix_like_kernel(matrix, vector, MATRIX_SIZE, iter2);
    
    /* Final reduction with XOR to prevent optimization */
    long long final_result = result1 ^ result2;
    
    /* Use the result to ensure side effects */
    printf("Result: %lld\n", final_result);
    
    /* Additional volatile operations to keep everything alive */
    volatile long long vol_result = final_result;
    if (vol_result > 1000000000LL) {
        printf("Large result detected\n");
    }
    
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
