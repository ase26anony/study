/* sel-sched-trigger.c
 * Designed to trigger debug_insn_rtx in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization of critical dependencies */
static volatile int global_seed = 42;

/* Function with potential aliasing to create memory dependencies */
static inline void compute_loop(int *restrict dest, const int *src, 
                               float *farr, double *darr, int n) {
    int i;
    int temp_acc = 0;
    float f_acc = 1.0f;
    double d_acc = 0.5;
    
    /* Complex loop with multiple dependencies and operations */
    for (i = 0; i < n; i++) {
        /* Carried dependency chain for integers */
        temp_acc = temp_acc * 6364136223846793005ULL + src[i];
        
        /* Independent floating-point operations */
        f_acc = f_acc * 1.0001f + farr[i];
        d_acc = d_acc / 1.00001 + darr[i];
        
        /* Memory operations with potential aliasing concerns */
        dest[i] = temp_acc + (int)(f_acc * 100.0f);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Additional operations in taken branch */
            f_acc = f_acc - 0.5f;
            d_acc = d_acc * 0.999;
            
            /* Volatile access to prevent reordering */
            global_seed = global_seed + 1;
        } else if (i % 13 == 0) {
            /* Another basic block */
            d_acc = d_acc + 2.0;
            temp_acc = temp_acc ^ 0x55555555;
        }
        
        /* More arithmetic diversity */
        if (i % 3 == 0) {
            farr[i] = f_acc * 2.0f;
        }
        
        /* Integer division - expensive operation for scheduler */
        if (src[i] != 0) {
            dest[i] = dest[i] / (src[i] | 1);
        }
        
        /* Mix of operations in single expression */
        darr[i] = d_acc + (double)temp_acc * 0.001 + (double)(i % 1024);
    }
    
    /* Store final accumulators to prevent dead code elimination */
    if (n > 0) {
        dest[0] = temp_acc;
        farr[0] = f_acc;
        darr[0] = d_acc;
    }
}

/* Another hot function to encourage inlining and create more scheduling regions */
static inline int process_data(int *data, float *fdata, double *ddata, int size) {
    int result = 0;
    int local_buf[256];
    float local_fbuf[256];
    double local_dbuf[256];
    
    /* Initialize local buffers */
    for (int i = 0; i < 256 && i < size; i++) {
        local_buf[i] = data[i] ^ 0xAAAAAAAA;
        local_fbuf[i] = fdata[i] * 3.14159f;
        local_dbuf[i] = ddata[i] / 2.71828;
    }
    
    /* Call compute_loop multiple times with different parameters */
    compute_loop(data, local_buf, fdata, ddata, size > 256 ? 256 : size);
    compute_loop(local_buf, data, local_fbuf, local_dbuf, size > 128 ? 128 : size);
    
    /* Compute checksum */
    for (int i = 0; i < size && i < 256; i++) {
        result ^= data[i];
        result += (int)(fdata[i] * 1000.0f);
        result ^= (int)ddata[i];
    }
    
    return result;
}

int main(void) {
    const int DATA_SIZE = 1024;
    int *int_data;
    float *float_data;
    double *double_data;
    int final_result = 0;
    
    /* Allocate and initialize data with non-zero values */
    int_data = (int*)malloc(DATA_SIZE * sizeof(int));
    float_data = (float*)malloc(DATA_SIZE * sizeof(float));
    double_data = (double*)malloc(DATA_SIZE * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic data */
    for (int i = 0; i < DATA_SIZE; i++) {
        int_data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_data[i] = (float)i * 0.12345f;
        double_data[i] = (double)i * 0.67891;
    }
    
    /* Create multiple scheduling regions by calling hot functions */
    for (int iter = 0; iter < 100; iter++) {
        /* Modify global seed to affect computation */
        global_seed = iter;
        
        /* This is the performance-critical section */
        int iter_result = process_data(int_data, float_data, double_data, DATA_SIZE);
        
        /* Accumulate results to prevent optimization */
        final_result ^= iter_result;
        
        /* Slight modification to input data for next iteration */
        if (iter % 3 == 0) {
            for (int i = 0; i < DATA_SIZE; i += 7) {
                int_data[i] += iter;
                float_data[i] += (float)iter * 0.01f;
            }
        }
    }
    
    /* Additional complex loop in main to create another scheduling region */
    {
        int local_acc = final_result;
        double local_dacc = 0.0;
        
        for (int i = 0; i < DATA_SIZE; i++) {
            /* Complex expression with mixed operations */
            local_acc = (local_acc * 31 + int_data[i]) / (1 + (i & 0xF));
            local_dacc = local_dacc + (double)local_acc * 0.001 - double_data[i];
            
            /* Memory store with potential dependency */
            if (i % 5 == 0) {
                int_data[i] = local_acc;
                double_data[i] = local_dacc;
            }
            
            /* Inline assembly with memory clobber to create scheduling barriers */
            asm volatile("" : : : "memory");
        }
        
        final_result = local_acc ^ (int)local_dacc;
    }
    
    /* Print result to ensure computation isn't optimized away */
    printf("Result: %d (seed: %d)\n", final_result, global_seed);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
