/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force copies and prevent optimization */
static inline int32_t copy_and_add(int32_t a, int32_t b, volatile int32_t* sink) {
    int32_t result = a + b;
    *sink = result;  /* volatile write to prevent elimination */
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

static inline float promote_and_multiply(int16_t a, float b, volatile float* sink) {
    float promoted = (float)a;
    float result = promoted * b;
    *sink = result;
    asm volatile("" : : "f"(result) : "memory");
    return result;
}

static inline double mixed_calc(int8_t a, int32_t b, double c, volatile double* sink) {
    double da = (double)a;
    double db = (double)b;
    double result = da * db + c;
    *sink = result;
    asm volatile("" : : "f"(result) : "memory");
    return result;
}

/* Function with complex control flow to create many basic blocks */
static void compute_kernel(int32_t* int_data, float* float_data, 
                          double* double_data, size_t size) {
    volatile int32_t sink_int __attribute__((unused));
    volatile float sink_float __attribute__((unused));
    volatile double sink_double __attribute__((unused));
    
    /* Outer loop creates register pressure */
    for (size_t i = 0; i < size; i++) {
        int32_t base = int_data[i];
        float fbase = float_data[i];
        double dbase = double_data[i];
        
        /* First inner loop with integer operations */
        for (int j = 0; j < 8; j++) {
            /* Create dependent chain of recomputable values */
            int32_t v1 = base + j * 3;
            int32_t v2 = v1 * 2 - 5;
            int32_t v3 = v2 + (j << 2);
            
            /* Force copy propagation context */
            int32_t copied = copy_and_add(v3, base, &sink_int);
            
            /* Use in conditional to create control flow */
            if (copied > 100) {
                /* Different mode calculations */
                float f1 = promote_and_multiply((int16_t)v1, fbase, &sink_float);
                float f2 = f1 * 2.5f + (float)j;
                
                /* More copies */
                float f3 = f2;
                asm volatile("" : : "f"(f3) : "memory");
            }
        }
        
        /* Second inner loop with mixed precision */
        for (int k = 0; k < 4; k++) {
            /* Create values that can be rematerialized */
            int8_t small = (int8_t)(base & 0xFF);
            int32_t medium = base + k * 17;
            
            /* Mixed precision calculation */
            double d1 = mixed_calc(small, medium, dbase, &sink_double);
            double d2 = d1 * 1.5 - (double)k;
            
            /* Conditional with register pressure */
            if (k % 2 == 0) {
                /* More operations to increase pressure */
                float temp = (float)d2;
                int32_t itemp = (int32_t)temp;
                itemp = copy_and_add(itemp, k, &sink_int);
                
                /* Use inline asm to clobber registers */
                asm volatile("" : : "r"(itemp), "f"(temp) : "r0", "r1", "r2", "r3");
            }
        }
        
        /* Third loop with nested conditionals */
        for (int m = 0; m < 6; m++) {
            /* Values that are cheap to recompute */
            int32_t a = base + m * 7;
            int32_t b = a ^ 0x55AA55AA;
            int32_t c = b >> 3;
            
            /* Complex conditional structure */
            if (m < 3) {
                float f = (float)c * 0.25f;
                if (f > 0.0f) {
                    double d = (double)f + dbase;
                    asm volatile("" : : "f"(d) : "memory");
                }
            } else {
                int32_t d = c * 2 + m;
                d = copy_and_add(d, base, &sink_int);
                
                /* Force spill/reload context */
                for (int n = 0; n < 2; n++) {
                    int32_t e = d + n * 11;
                    volatile int32_t* ptr = &sink_int;
                    *ptr = e;
                }
            }
        }
    }
}

/* Initialize with patterns that create varied computations */
static void init_data(int32_t* int_data, float* float_data, 
                     double* double_data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        int_data[i] = (int32_t)(i * 12345 + 6789);
        float_data[i] = (float)(i * 1.2345f - 0.6789f);
        double_data[i] = (double)(i * 0.98765 + 0.54321);
    }
}

int main(void) {
    const size_t data_size = 128;
    
    /* Allocate aligned data for better code generation */
    int32_t* int_data = (int32_t*)aligned_alloc(16, data_size * sizeof(int32_t));
    float* float_data = (float*)aligned_alloc(16, data_size * sizeof(float));
    double* double_data = (double*)aligned_alloc(16, data_size * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        return 1;
    }
    
    init_data(int_data, float_data, double_data, data_size);
    
    /* Multiple iterations to increase optimization opportunities */
    for (int iter = 0; iter < 3; iter++) {
        compute_kernel(int_data, float_data, double_data, data_size);
        
        /* Modify data slightly each iteration */
        for (size_t i = 0; i < data_size; i++) {
            int_data[i] += iter * 17;
            float_data[i] += (float)iter * 0.1f;
        }
    }
    
    /* Use results to prevent dead code elimination */
    volatile int32_t final_sink = 0;
    for (size_t i = 0; i < data_size; i++) {
        final_sink += int_data[i];
        asm volatile("" : : "r"(final_sink) : "memory");
    }
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return (int)final_sink != 0;
}
