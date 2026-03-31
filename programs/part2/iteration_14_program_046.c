/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_USE(x) asm volatile("" : : "r"(x))
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Helper functions to force copy propagation */
static inline int32_t process_int(int32_t a, int32_t b, volatile int32_t* sink) {
    int32_t tmp = a * 3 + b / 2;
    *sink = tmp;  /* Volatile write to prevent elimination */
    return tmp - a + 7;
}

static inline float process_float(float a, float b, volatile float* sink) {
    float tmp = a * 1.5f + b * 0.25f;
    *sink = tmp;
    return tmp / (a + 1.0f);
}

static inline double process_double(double a, double b, volatile double* sink) {
    double tmp = a * 2.71828 + b * 0.367879;
    *sink = tmp;
    return tmp - a * 0.5;
}

/* Function with complex control flow to create many virtual registers */
void compute_kernel(int32_t* int_data, float* float_data, double* double_data, 
                   size_t size, volatile int32_t* int_sink, volatile float* float_sink,
                   volatile double* double_sink) {
    
    /* Multiple local variables to increase register pressure */
    int32_t acc1 = 0, acc2 = 0, acc3 = 0;
    float f_acc1 = 0.0f, f_acc2 = 0.0f;
    double d_acc1 = 0.0, d_acc2 = 0.0;
    
    /* Outer loop with multiple inner loops */
    for (size_t i = 0; i < size; i++) {
        /* First inner loop - integer operations */
        for (int j = 0; j < 8; j++) {
            /* Create dependent chain of computations */
            int32_t tmp1 = int_data[i] + j * 17;
            int32_t tmp2 = tmp1 * 3 - j;
            int32_t tmp3 = tmp2 / (abs(j) + 1);
            
            /* Force copies through helper function */
            acc1 = process_int(tmp1, tmp2, int_sink);
            acc2 = process_int(tmp2, tmp3, int_sink + 1);
            acc3 = process_int(tmp3, acc1, int_sink + 2);
            
            /* Use volatile and inline asm to prevent optimizations */
            FORCE_USE(acc1);
            FORCE_USE(acc2);
            FORCE_USE(acc3);
            
            /* Conditional to create control flow complexity */
            if (j % 3 == 0) {
                int32_t tmp4 = acc1 * 2 - acc2;
                acc3 = process_int(tmp4, acc3, int_sink + 3);
                FORCE_USE(tmp4);
            } else if (j % 3 == 1) {
                int32_t tmp5 = acc2 + acc3 * 5;
                acc1 = process_int(acc1, tmp5, int_sink + 4);
                FORCE_USE(tmp5);
            }
        }
        
        /* Second inner loop - mixed float/double operations */
        for (int k = 0; k < 4; k++) {
            /* Type conversions and mixed precision */
            float f_tmp1 = float_data[i] + k * 0.125f;
            double d_tmp1 = (double)f_tmp1 * 1.41421356;
            
            /* Force register copies with different modes */
            f_acc1 = process_float(f_tmp1, f_acc1, float_sink);
            d_acc1 = process_double(d_tmp1, d_acc1, double_sink);
            
            /* More complex expressions with type mixing */
            float f_tmp2 = (float)d_acc1 * 0.707f + f_acc1;
            double d_tmp2 = (double)f_tmp2 * 2.0 - d_acc1;
            
            f_acc2 = process_float(f_tmp2, f_acc2, float_sink + 1);
            d_acc2 = process_double(d_tmp2, d_acc2, double_sink + 1);
            
            /* Use inline asm with clobbers */
            asm volatile("" : : "r"(f_acc1), "r"(d_acc1) : "xmm0", "xmm1");
            asm volatile("" : : "r"(f_acc2), "r"(d_acc2) : "xmm2", "xmm3");
            
            /* Conditional with different computation paths */
            if (k % 2 == 0) {
                float f_tmp3 = f_acc1 * f_acc2 - float_data[i];
                f_acc1 = process_float(f_tmp3, f_acc1, float_sink + 2);
                FORCE_USE(f_tmp3);
            } else {
                double d_tmp3 = d_acc1 / (d_acc2 + 1.0);
                d_acc1 = process_double(d_tmp3, d_acc1, double_sink + 2);
                FORCE_USE(d_tmp3);
            }
        }
        
        /* Third section - more integer with different sizes */
        for (short s = 0; s < 16; s++) {
            /* Operations with different integer sizes */
            char c_val = (char)(s * 13);
            int16_t short_val = s * 11;
            int32_t int_val = int_data[i] + s;
            
            /* Promote/demote types to create mode changes */
            int32_t promoted = (int32_t)c_val * (int32_t)short_val;
            int16_t demoted = (int16_t)(int_val % 256);
            
            /* Complex expression chain */
            int32_t chain1 = promoted + demoted * 3;
            int32_t chain2 = chain1 - int_val / 2;
            int32_t chain3 = chain2 * 7 + promoted;
            
            /* Force copies and prevent optimization */
            acc1 = process_int(chain1, chain2, int_sink + 5);
            acc2 = process_int(chain2, chain3, int_sink + 6);
            acc3 = process_int(chain3, acc1, int_sink + 7);
            
            /* Use all results to prevent dead code elimination */
            asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3) : "eax", "ebx", "ecx");
            
            /* Another conditional block */
            if (s % 4 == 0) {
                int32_t tmp = acc1 + acc2 * 2;
                FORCE_USE(tmp);
                acc3 = process_int(tmp, acc3, int_sink + 8);
            }
        }
        
        /* Final mixing of all types */
        if (i % 3 == 0) {
            float mixed_f = (float)acc1 * 0.01f + f_acc1;
            double mixed_d = (double)acc2 * 0.001 + d_acc1;
            int32_t mixed_i = (int32_t)(mixed_f * 100) + (int32_t)mixed_d;
            
            f_acc1 = process_float(mixed_f, f_acc2, float_sink + 3);
            d_acc1 = process_double(mixed_d, d_acc2, double_sink + 3);
            acc1 = process_int(mixed_i, acc3, int_sink + 9);
            
            FORCE_USE(mixed_f);
            FORCE_USE(mixed_d);
            FORCE_USE(mixed_i);
        }
    }
}

int main() {
    const size_t SIZE = 1024;
    
    /* Allocate and initialize arrays with different patterns */
    int32_t* int_data = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float* float_data = (float*)malloc(SIZE * sizeof(float));
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    
    volatile int32_t int_sink[10];
    volatile float float_sink[10];
    volatile double double_sink[10];
    
    /* Initialize with varying patterns to prevent constant propagation */
    for (size_t i = 0; i < SIZE; i++) {
        int_data[i] = (i * 37) % 7919;  /* Prime modulus for variation */
        float_data[i] = (float)((i * 19) % 1024) * 0.123f;
        double_data[i] = (double)((i * 23) % 2048) * 0.456;
    }
    
    /* Run the computation kernel multiple times */
    for (int iter = 0; iter < 3; iter++) {
        compute_kernel(int_data, float_data, double_data, SIZE, 
                      int_sink, float_sink, double_sink);
    }
    
    /* Use results to prevent elimination of entire computation */
    int32_t final_check = 0;
    for (int i = 0; i < 10; i++) {
        final_check += int_sink[i];
        final_check += (int32_t)float_sink[i];
        final_check += (int32_t)double_sink[i];
    }
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return final_check % 256;
}
