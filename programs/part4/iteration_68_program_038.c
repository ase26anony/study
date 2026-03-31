/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 -o trigger sel-sched-trigger.c
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
static uint64_t compute_kernel1(int *data, int size, int stride) {
    uint64_t sum = 0;
    volatile int v_counter = 0;
    
    /* Nested loops with data dependencies */
    for (int i = 1; i < size; i += stride) {
        /* Outer loop with volatile bound */
        for (int j = 0; j < volatile_bound % 8; j++) {
            v_counter++;
            
            /* Complex data-dependent computation with mixed operations */
            int idx = (i * 17 + j * 13) % size;
            int prev_idx = ((i-1) * 17 + j * 13) % size;
            
            /* Carried dependency across iterations */
            int64_t temp = (int64_t)data[idx] * (int64_t)data[prev_idx];
            
            /* Mixed-width operations creating register pressure */
            sum += (uint64_t)(temp >> 3);
            sum ^= (uint64_t)(temp & 0x7F);
            
            /* Conditional operations with both branches having computation */
            if (data[idx] > data[prev_idx]) {
                /* Branch 1: Floating point and integer mix */
                float f1 = (float)data[idx] / (float)(data[prev_idx] + 1);
                sum += (uint64_t)(f1 * 1000.0f);
                
                /* Inline assembly to create fixed RTL patterns */
                asm volatile ("" : : "r"(f1) : "memory");
            } else {
                /* Branch 2: Different computation pattern */
                double d1 = (double)data[prev_idx] * 1.5;
                sum += (uint64_t)d1;
                
                /* More complex integer arithmetic */
                sum = (sum * 1103515245 + 12345) & 0x7FFFFFFF;
            }
            
            /* Switch statement creating multiple basic blocks */
            switch (data[idx] & 0x3) {
                case 0:
                    sum += data[idx] << 2;
                    /* Memory access with non-trivial addressing */
                    sum += data[(idx + 3) % size] * 3;
                    break;
                case 1:
                    sum ^= data[prev_idx] | 0xFF00;
                    /* Division with non-constant divisor */
                    sum /= (data[prev_idx] & 0x1F) + 1;
                    break;
                case 2:
                    /* Pointer chasing pattern */
                    sum += data[data[idx] % size];
                    /* Ternary operator creating conditional move */
                    sum += (data[idx] > 0) ? sum >> 4 : sum << 4;
                    break;
                default:
                    /* Complex floating point */
                    float f2 = (float)sum / 256.0f;
                    sum += (uint64_t)(f2 * f2);
                    break;
            }
        }
        
        /* Partial unrolling hint */
        #pragma GCC unroll 4
        for (int k = 0; k < 4; k++) {
            /* Additional computation in unrolled loop */
            int offset = (i + k) % size;
            sum ^= (uint64_t)data[offset] << (k * 8);
            
            /* More inline assembly */
            asm volatile ("# Fixed instruction" : : : "memory");
        }
    }
    
    return sum;
}

/* Second computation kernel with different pattern */
static uint64_t compute_kernel2(int *matrix, int *vector, int n) {
    uint64_t result = 0;
    volatile int v_iter = volatile_seed % 4;
    
    /* Matrix-vector multiplication pattern */
    for (int i = 0; i < n; i++) {
        int row_sum = 0;
        
        /* Inner loop with stride access */
        for (int j = 0; j < n; j += 2) {
            /* Data-dependent addressing */
            int idx = i * n + j;
            int next_idx = idx + 1;
            
            /* Mixed computations */
            int prod1 = matrix[idx] * vector[j];
            int prod2 = (next_idx < n * n) ? matrix[next_idx] * vector[j + 1] : 0;
            
            row_sum += prod1 + prod2;
            
            /* Complex conditional */
            if ((prod1 ^ prod2) & 0x1) {
                row_sum = (row_sum * 3) / 2;
            } else {
                row_sum = (row_sum << 1) | 0x1;
            }
            
            /* Volatile check every 8 iterations */
            if ((j & 0x7) == 0) {
                v_iter++;
            }
        }
        
        /* Reduction with dependency */
        result = (result * 31) + row_sum;
        
        /* Additional floating point stress */
        if (i % 3 == 0) {
            double d = (double)row_sum / (double)(i + 1);
            result += (uint64_t)(d * d * 100.0);
        }
    }
    
    return result;
}

/* Initialize data with pseudo-random values */
static void init_data(int *data, int size) {
    unsigned int seed = volatile_seed;
    for (int i = 0; i < size; i++) {
        /* Simple PRNG to avoid libc calls in init */
        seed = seed * 1103515245 + 12345;
        data[i] = (int)(seed >> 16) & 0x7FFF;
    }
}

int main(void) {
    const int data_size = 1024;
    const int matrix_size = 32;  // 32x32 matrix
    
    /* Allocate and initialize data */
    int *data = (int*)malloc(data_size * sizeof(int));
    int *matrix = (int*)malloc(matrix_size * matrix_size * sizeof(int));
    int *vector = (int*)malloc(matrix_size * sizeof(int));
    
    if (!data || !matrix || !vector) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_data(data, data_size);
    init_data(matrix, matrix_size * matrix_size);
    init_data(vector, matrix_size);
    
    uint64_t result1 = 0, result2 = 0;
    
    /* Run first kernel multiple times with different strides */
    for (int stride = 1; stride <= 4; stride++) {
        volatile_seed = stride * 100;
        result1 ^= compute_kernel1(data, data_size, stride);
    }
    
    /* Run second kernel */
    volatile_seed = result1 & 0xFF;
    result2 = compute_kernel2(matrix, vector, matrix_size);
    
    /* Final reduction to prevent optimization */
    uint64_t final_result = result1 ^ result2;
    
    /* Use result to ensure side effects */
    printf("Result: 0x%016llx\n", (unsigned long long)final_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
