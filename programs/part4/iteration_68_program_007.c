/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* External function to create dependencies */
extern int rand(void);

/* Complex computation with data dependencies */
static inline int64_t complex_op(int64_t a, int64_t b, int64_t c) {
    /* Mixed-width operations */
    int32_t a32 = (int32_t)a;
    int64_t b64 = b;
    
    /* Conditional operations */
    int64_t result = (a > 0) ? (a32 * b64) : (b64 / (a32 ? a32 : 1));
    
    /* Floating point to create FPU pressure */
    double dbl = (double)result;
    dbl = dbl * 1.234567 - 0.987654;
    
    /* More integer ops */
    result = (int64_t)dbl + c * 3;
    
    return result;
}

/* Pointer chasing pattern */
static int64_t chase_pointer(int64_t *data, int size, int start) {
    int64_t sum = 0;
    int idx = start;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent access with stride */
        idx = (idx * 13 + 7) % size;
        
        /* Complex addressing mode */
        sum += data[idx] * data[(idx + 1) % size] - 
               data[(idx + 2) % size] / (data[idx] ? data[idx] : 1);
        
        /* Inline assembly to create fixed RTL */
        asm volatile ("" : : "r"(sum) : "memory");
    }
    
    return sum;
}

/* Matrix-vector like computation */
static void matrix_compute(double *matrix, double *vector, double *result, 
                          int rows, int cols) {
    #pragma GCC unroll 4
    for (int i = 0; i < rows; i++) {
        double acc = 0.0;
        
        /* Inner loop with complex addressing */
        for (int j = 0; j < cols; j++) {
            /* Non-trivial addressing */
            double val = matrix[i * cols + j] * vector[j];
            
            /* Conditional operation */
            if (val > 0.0) {
                acc += val * 1.5;
            } else {
                acc -= val * 0.5;
            }
            
            /* Mixed precision */
            acc = acc + (float)(val * 0.1);
        }
        
        /* Store with potential aliasing */
        result[i] = acc;
        
        /* Another inline asm barrier */
        asm volatile ("" : : "r"(acc) : "memory");
    }
}

/* Main computation kernel */
static int64_t compute_kernel(int64_t *data, int size) {
    int64_t total = 0;
    volatile int v_counter = 0;
    
    /* Outer loop with volatile bound */
    for (int outer = 0; outer < volatile_bound % 100; outer++) {
        v_counter++;
        
        /* Inner loop with carried dependency */
        int64_t running_sum = data[0];
        
        #pragma GCC unroll 2
        for (int i = 1; i < size; i++) {
            /* Complex data-dependent computation */
            int64_t prev = running_sum;
            
            /* Multiple operations to create ILP opportunities */
            int64_t a = data[i];
            int64_t b = data[i - 1];
            int64_t c = data[(i * 7) % size];
            
            /* Branch with computation in both paths */
            if ((a ^ b) & 0x1) {
                running_sum += complex_op(a, b, c);
                running_sum *= (prev % 7) + 1;
            } else {
                running_sum -= complex_op(b, a, c);
                running_sum /= ((prev % 5) + 1);
            }
            
            /* Memory access with stride */
            if (i % 3 == 0) {
                running_sum += data[(i * 2) % size];
            }
            
            /* Switch statement to create control flow */
            switch (i % 4) {
                case 0:
                    running_sum |= 0xFF;
                    break;
                case 1:
                    running_sum &= 0xFFFF;
                    break;
                case 2:
                    running_sum ^= running_sum >> 4;
                    break;
                case 3:
                    running_sum = ~running_sum;
                    break;
            }
        }
        
        total ^= running_sum;
        
        /* Prevent loop optimization */
        if (rand() % 1000 == 0) {
            volatile_bound++;
        }
    }
    
    return total;
}

int main(void) {
    const int data_size = 1024;
    const int matrix_size = 64;
    
    /* Initialize with pseudo-random data */
    int64_t *data = (int64_t*)malloc(data_size * sizeof(int64_t));
    double *matrix = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    double *vector = (double*)malloc(matrix_size * sizeof(double));
    double *result = (double*)malloc(matrix_size * sizeof(double));
    
    /* Simple PRNG for initialization */
    uint32_t seed = time(NULL) ^ volatile_seed;
    for (int i = 0; i < data_size; i++) {
        seed = seed * 1103515245 + 12345;
        data[i] = (int64_t)(seed % 1000) - 500;
    }
    
    for (int i = 0; i < matrix_size * matrix_size; i++) {
        seed = seed * 1103515245 + 12345;
        matrix[i] = (double)(seed % 100) / 10.0;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        seed = seed * 1103515245 + 12345;
        vector[i] = (double)(seed % 50) / 5.0;
    }
    
    /* Run multiple computation patterns */
    int64_t sum1 = 0, sum2 = 0;
    
    /* First: Pointer chasing computation */
    sum1 = chase_pointer(data, data_size, rand() % data_size);
    
    /* Second: Main kernel with complex loops */
    sum2 = compute_kernel(data, data_size);
    
    /* Third: Matrix computation */
    matrix_compute(matrix, vector, result, matrix_size, matrix_size);
    
    /* Final reduction with side effect */
    int64_t final_result = sum1 ^ sum2;
    for (int i = 0; i < matrix_size; i++) {
        final_result += (int64_t)result[i];
    }
    
    /* Ensure side effects are visible */
    printf("Result: %ld\n", final_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
