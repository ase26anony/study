/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Simple PRNG using volatile seed */
static inline int pseudo_rand(void) {
    volatile_seed = (volatile_seed * 1103515245 + 12345) & 0x7fffffff;
    return volatile_seed;
}

/* Complex computation with data dependencies */
static int64_t compute_kernel1(int32_t* data, int size, int stride) {
    int64_t sum = 0;
    volatile int vol_counter = 0;
    
    /* Outer loop with volatile bound */
    for (int i = 1; i < volatile_bound && i < size; i++) {
        int32_t temp = data[i];
        
        /* Inner loop with carried dependency */
        #pragma GCC unroll 4
        for (int j = 0; j < 8; j++) {
            /* Data-dependent computation with mixed-width operations */
            int32_t prev = data[i-1];
            int64_t wide_mul = (int64_t)temp * (int64_t)prev;
            
            /* Conditional operations */
            int32_t cond_val = (prev > 0) ? (temp / (prev | 1)) : (temp * 2);
            
            /* Floating point operations to create FPU pressure */
            float fp_temp = (float)temp * 0.5f;
            double dp_temp = (double)cond_val * 1.5;
            
            /* Mixed operations */
            sum += (int64_t)(wide_mul + (int64_t)cond_val + 
                   (int64_t)(fp_temp * 100.0f) + 
                   (int64_t)(dp_temp * 50.0));
            
            /* Pointer chasing pattern */
            temp = data[(i + j) % size];
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : "r"(temp) : "memory");
        }
        
        /* Branch with substantial computation in both paths */
        if (temp & 0x1) {
            /* Complex path 1 */
            sum += (int64_t)data[i] * data[(i * stride) % size];
            sum ^= (sum >> 32) | (sum << 32);
        } else {
            /* Complex path 2 */
            sum -= (int64_t)data[i] / (data[(i * 3) % size] | 1);
            sum = (sum * 6364136223846793005LL) ^ 1442695040888963407LL;
        }
        
        vol_counter++;
    }
    
    return sum;
}

/* Second computation kernel with different pattern */
static int64_t compute_kernel2(int32_t* mat, int32_t* vec, int n) {
    int64_t result = 0;
    volatile int vol_n = n;
    
    /* Matrix-vector multiplication pattern */
    for (int i = 0; i < vol_n && i < 64; i++) {
        int32_t row_sum = 0;
        
        /* Switch statement creating multiple basic blocks */
        switch (i % 5) {
            case 0:
                for (int j = 0; j < n; j++) {
                    row_sum += mat[i * n + j] * vec[j];
                    /* Memory access with non-trivial addressing */
                    row_sum ^= mat[(i * n + (j ^ 0x3)) % (n * n)];
                }
                break;
            case 1:
                for (int j = n-1; j >= 0; j--) {
                    row_sum += mat[i * n + j] / (vec[j] | 1);
                    /* Another inline assembly barrier */
                    asm volatile ("" : : "r"(row_sum) : "memory");
                }
                break;
            case 2:
                for (int j = 0; j < n; j += 2) {
                    int32_t a = mat[i * n + j];
                    int32_t b = (j+1 < n) ? mat[i * n + j+1] : 1;
                    row_sum += (a > b) ? (a - b) : (b - a);
                }
                break;
            case 3:
                for (int j = 0; j < n; j++) {
                    float fval = (float)mat[i * n + j] * 0.25f;
                    row_sum += (int32_t)(fval * 100.0f);
                }
                break;
            default:
                for (int j = 0; j < n; j++) {
                    row_sum = (row_sum << 3) | (row_sum >> 29);
                    row_sum ^= mat[i * n + j] * vec[j];
                }
        }
        
        result += (int64_t)row_sum * (i + 1);
        
        /* Additional computation with external dependency */
        if (rand() % 100 < 10) {  /* External function call */
            result = (result >> 16) | (result << 48);
        }
    }
    
    return result;
}

/* Third kernel with nested loops and complex addressing */
static int64_t compute_kernel3(int32_t* data, int size) {
    int64_t acc = 0;
    volatile int vol_size = size;
    
    for (int i = 0; i < vol_size; i++) {
        int32_t* ptr = data + i;
        
        /* Loop with pointer arithmetic */
        for (int k = 0; k < 16 && (ptr - data) < size; k++) {
            /* Complex addressing mode simulation */
            int idx = (i * 17 + k * 13) % size;
            int idx2 = (i * 23 + k * 7) % size;
            
            /* Mixed operations creating register pressure */
            int64_t val1 = (int64_t)data[idx] * 0x9e3779b97f4a7c15LL;
            int64_t val2 = (int64_t)data[idx2] * 0xbf58476d1ce4e5b9LL;
            
            /* 32-bit and 64-bit mixed arithmetic */
            acc += (val1 & 0xFFFFFFFF) + (val2 >> 32);
            acc ^= (val1 >> 32) | (val2 & 0xFFFFFFFF);
            
            /* Conditional move simulation via ternary */
            acc = (acc < 0) ? -acc : acc;
            
            ptr++;
        }
        
        /* Periodic external call */
        if (i % 100 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return acc;
}

int main(void) {
    const int DATA_SIZE = 1024;
    const int MAT_SIZE = 32;
    
    /* Initialize with pseudo-random data */
    int32_t* data = (int32_t*)malloc(DATA_SIZE * sizeof(int32_t));
    int32_t* matrix = (int32_t*)malloc(MAT_SIZE * MAT_SIZE * sizeof(int32_t));
    int32_t* vector = (int32_t*)malloc(MAT_SIZE * sizeof(int32_t));
    
    if (!data || !matrix || !vector) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    srand(time(NULL));
    
    /* Fill arrays with varied data */
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = pseudo_rand() % 1000 - 500;
    }
    
    for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
        matrix[i] = rand() % 256 - 128;
    }
    
    for (int i = 0; i < MAT_SIZE; i++) {
        vector[i] = rand() % 128 - 64;
    }
    
    /* Execute computation kernels */
    int64_t result1 = compute_kernel1(data, DATA_SIZE, 7);
    int64_t result2 = compute_kernel2(matrix, vector, MAT_SIZE);
    int64_t result3 = compute_kernel3(data, DATA_SIZE);
    
    /* Final reduction to prevent optimization */
    int64_t final_result = result1 ^ result2 ^ result3;
    
    /* Use result to ensure side effects */
    printf("Final result: %ld\n", (long)final_result);
    
    /* Additional volatile operations */
    volatile int check = (final_result != 0);
    if (check) {
        printf("Computation completed successfully\n");
    }
    
    free(data);
    free(matrix);
    free(vector);
    
    return 0;
}
