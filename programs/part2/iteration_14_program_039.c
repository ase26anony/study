/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force copies and prevent optimization */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    volatile int32_t temp = a + b;
    asm volatile("" : "+r"(temp) : : "memory");
    return temp + c;
}

static inline float promote_and_multiply(int16_t s, float f) {
    volatile int32_t promoted = (int32_t)s;
    asm volatile("" : "+r"(promoted) : : "memory");
    return promoted * f;
}

static inline double mixed_calc(uint8_t b, double d, int64_t l) {
    volatile double temp = d + (double)b;
    asm volatile("" : "+f"(temp) : : "memory");
    return temp * (double)l;
}

/* Function to create register pressure with mixed types */
static void process_block(int32_t *int_arr, float *float_arr, 
                         double *double_arr, size_t size) {
    volatile int32_t sink_int = 0;
    volatile float sink_float = 0.0f;
    volatile double sink_double = 0.0;
    
    /* Multiple local variables to increase register pressure */
    int32_t local1, local2, local3, local4;
    float flocal1, flocal2, flocal3;
    double dlocal1, dlocal2;
    
    /* Outer loop with complex control flow */
    for (size_t i = 0; i < size; i++) {
        /* First inner loop - integer operations */
        for (int j = 0; j < 8; j++) {
            /* Chain of dependent arithmetic operations */
            local1 = int_arr[i] + j * 3;
            local2 = local1 ^ 0x55AA55AA;
            local3 = copy_and_add(local1, local2, j);
            
            /* Force copy propagation context */
            local4 = local3;
            asm volatile("" : "+r"(local4) : : "memory");
            
            /* Mixed precision calculation */
            flocal1 = promote_and_multiply((int16_t)local4, float_arr[i]);
            
            /* Conditional block to split control flow */
            if (local3 & 1) {
                flocal2 = flocal1 * 2.5f;
                sink_float = flocal2;
            } else {
                flocal2 = flocal1 / 2.0f;
                asm volatile("" : "+f"(flocal2) : : "memory");
            }
            
            /* Another level of nesting */
            for (int k = 0; k < 4; k++) {
                /* More register pressure with different types */
                dlocal1 = mixed_calc((uint8_t)local2, double_arr[i], 
                                   (int64_t)local3 + k);
                
                /* Complex expression with multiple intermediate values */
                dlocal2 = dlocal1 * (k + 1) * 0.25;
                flocal3 = (float)dlocal2 + flocal2;
                
                /* Volatile sink to prevent elimination */
                sink_double = dlocal2;
                sink_float = flocal3;
                
                /* Inline asm that clobbers registers */
                asm volatile("" : : "r"(local1), "r"(local2), 
                           "f"(flocal3), "f"(dlocal2) : "memory");
            }
            
            /* Final sink */
            sink_int = local4;
        }
        
        /* Second inner loop with different operations */
        for (int j = 0; j < 4; j++) {
            /* Create more virtual registers */
            int32_t tmp1 = int_arr[i] - j * 7;
            int32_t tmp2 = tmp1 << (j % 4);
            
            /* Force register moves with inline asm */
            asm volatile("" : "+r"(tmp1), "+r"(tmp2) : : "memory");
            
            /* Floating point conversion chain */
            float ftmp1 = (float)tmp1 * 1.1f;
            float ftmp2 = (float)tmp2 * 0.9f;
            
            /* Another copy propagation opportunity */
            float ftmp3 = ftmp1;
            asm volatile("" : "+f"(ftmp3) : : "memory");
            
            /* Mixed operation */
            double dtmp = (double)ftmp2 + (double)ftmp3;
            sink_double = dtmp;
            
            /* More clobbering to force spills */
            asm volatile("" : : "r"(tmp1), "r"(tmp2), 
                       "f"(ftmp1), "f"(ftmp2) : "memory");
        }
    }
}

/* Additional helper to create more complex call graph */
static inline int64_t recursive_chain(int32_t base, int depth) {
    if (depth <= 0) return base;
    
    volatile int32_t intermediate = base * 2;
    asm volatile("" : "+r"(intermediate) : : "memory");
    
    /* Recursive call creates more register pressure */
    int64_t result = recursive_chain(intermediate, depth - 1);
    
    /* Mixed type operation */
    return result + (int64_t)((float)intermediate * 1.5f);
}

int main(void) {
    const size_t ARRAY_SIZE = 1024;
    
    /* Allocate and initialize arrays with different patterns */
    int32_t *int_array = (int32_t*)malloc(ARRAY_SIZE * sizeof(int32_t));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!int_array || !float_array || !double_array) {
        return 1;
    }
    
    /* Initialize with varying patterns to prevent optimization */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (int32_t)(i * 3 + 1);
        float_array[i] = (float)(i * 0.5f + 0.1f);
        double_array[i] = (double)(i * 0.25 + 0.01);
    }
    
    /* Multiple processing blocks to increase optimization opportunities */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Call with different parameters each iteration */
        process_block(int_array + iteration, 
                     float_array + iteration,
                     double_array + iteration,
                     ARRAY_SIZE - iteration);
        
        /* Additional computations between calls */
        volatile int64_t chain_result = recursive_chain(iteration, 3);
        asm volatile("" : : "r"(chain_result) : "memory");
        
        /* Modify arrays slightly to prevent loop invariant removal */
        int_array[iteration] ^= 0x12345678;
        float_array[iteration] *= 1.01f;
    }
    
    /* Final volatile store to ensure all computations are used */
    volatile int32_t final_sink = int_array[0];
    asm volatile("" : : "r"(final_sink) : "memory");
    
    free(int_array);
    free(float_array);
    free(double_array);
    
    return 0;
}
