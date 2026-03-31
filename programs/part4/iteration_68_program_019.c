/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumping in GCC.
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
 * Or more aggressively: gcc -O3 -fsel-sched-pipelining -funroll-loops -march=native -fdump-rtl-all sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Function with mixed-width operations to create register pressure */
static inline long long mixed_width_compute(int a, long long b, float c) {
    /* Mixed 32-bit and 64-bit operations */
    long long t1 = (long long)a * b;      /* 32x64 -> 64 */
    int t2 = (int)(c * 100.0f);           /* float -> 32 */
    long long t3 = t1 / (t2 + 1);         /* 64/32 -> 64 with non-constant divisor */
    return t3 + (t2 > 50 ? t1 : -t1);     /* Conditional move pattern */
}

/* Matrix-vector multiplication kernel */
void matvec_multiply(float* result, float** matrix, float* vector, 
                     int rows, int cols, volatile int* volatile_counter) {
    for (int i = 0; i < rows; i++) {
        float sum = 0.0f;
        /* Inner loop with data dependency */
        for (int j = 0; j < cols; j++) {
            /* Complex addressing with stride */
            float val = matrix[i][j] * vector[j];
            
            /* Data-dependent computation with cross-iteration dependency */
            if (j > 0) {
                val += matrix[i][j-1] * 0.5f;  /* Carried dependency */
            }
            
            /* Mixed operations */
            sum += val;
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : : "memory");
            
            /* Conditional operation based on value */
            sum = (val > 0.0f) ? sum * 1.01f : sum * 0.99f;
        }
        
        /* Pointer chasing pattern */
        float* ptr = &result[i];
        for (int k = 0; k < 3; k++) {
            *ptr = *ptr * 1.1f + sum;
            ptr = (float*)((char*)ptr + sizeof(float));
        }
        
        /* Volatile access prevents optimization */
        (*volatile_counter)++;
    }
}

/* Complex nested loop with control flow */
long long complex_loop(int* data, int size, volatile int bound) {
    long long total = 0;
    int local_sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < size; i++) {
        /* Nested loop with carried dependency */
        for (int j = 1; j < bound; j++) {
            /* Data-dependent computation */
            int idx = (i * j) % size;
            
            /* Complex conditional with both branches having computation */
            if (data[idx] > 100) {
                /* Branch 1: Integer operations */
                int temp = data[idx] * 3;
                temp = temp / (data[(idx + 1) % size] + 1);  /* Non-constant divisor */
                local_sum += temp;
                
                /* Floating point in integer loop */
                float ftemp = (float)temp * 1.5f;
                local_sum += (int)ftemp;
            } else {
                /* Branch 2: Different computation */
                int temp = data[idx] + data[(idx + size - 1) % size];  /* Another dependency */
                temp = temp * temp;
                local_sum -= temp % 256;
            }
            
            /* Switch statement with multiple cases */
            switch (data[idx] % 4) {
                case 0:
                    total += local_sum * 2LL;
                    break;
                case 1:
                    total += local_sum * 3LL;
                    break;
                case 2:
                    total += (long long)local_sum * local_sum;
                    break;
                case 3:
                    total -= local_sum;
                    break;
            }
            
            /* Mixed-width operation */
            total = mixed_width_compute(local_sum, total, (float)data[idx]);
            
            /* Another inline assembly barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Loop unrolling hint */
        #pragma GCC unroll 4
        for (int k = 0; k < 4; k++) {
            /* Additional computation in unrolled loop */
            total ^= (long long)data[(i + k) % size] << (k * 8);
        }
    }
    
    return total;
}

/* Initialize array with pseudo-random data */
void init_array(int* arr, int size) {
    unsigned int seed = volatile_seed;
    for (int i = 0; i < size; i++) {
        /* Simple PRNG to avoid rand() calls in tight loops */
        seed = seed * 1103515245 + 12345;
        arr[i] = (int)(seed % 1000);
    }
}

/* Initialize matrix */
void init_matrix(float** matrix, int rows, int cols) {
    unsigned int seed = 123456789;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            seed = seed * 1664525 + 1013904223;
            matrix[i][j] = (float)(seed % 100) / 10.0f;
        }
    }
}

int main() {
    const int data_size = 1024;
    const int matrix_rows = 64;
    const int matrix_cols = 64;
    
    /* Allocate and initialize data */
    int* data = (int*)malloc(data_size * sizeof(int));
    float* vector = (float*)malloc(matrix_cols * sizeof(float));
    float* result = (float*)malloc(matrix_rows * sizeof(float));
    float** matrix = (float**)malloc(matrix_rows * sizeof(float*));
    
    for (int i = 0; i < matrix_rows; i++) {
        matrix[i] = (float*)malloc(matrix_cols * sizeof(float));
    }
    
    /* Initialize with pseudo-random data */
    init_array(data, data_size);
    init_matrix(matrix, matrix_rows, matrix_cols);
    
    /* Initialize vector with volatile-influenced values */
    for (int i = 0; i < matrix_cols; i++) {
        vector[i] = (float)((data[i % data_size] + volatile_seed) % 100) / 20.0f;
    }
    
    volatile int counter = 0;
    
    /* First computation kernel: complex nested loops */
    printf("Starting complex loop computation...\n");
    long long result1 = complex_loop(data, data_size, volatile_bound);
    printf("Complex loop result: %lld\n", result1);
    
    /* Second computation kernel: matrix-vector multiplication */
    printf("Starting matrix-vector multiplication...\n");
    matvec_multiply(result, matrix, vector, matrix_rows, matrix_cols, &counter);
    
    /* Final reduction to prevent optimization */
    float final_sum = 0.0f;
    for (int i = 0; i < matrix_rows; i++) {
        final_sum += result[i];
    }
    
    /* XOR all results together */
    long long final_result = result1 ^ (long long)final_sum ^ counter;
    printf("Final combined result: %lld\n", final_result);
    printf("Volatile counter: %d\n", counter);
    
    /* Cleanup */
    for (int i = 0; i < matrix_rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(data);
    free(vector);
    free(result);
    
    return 0;
}
