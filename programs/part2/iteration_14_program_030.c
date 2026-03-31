/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force copies and prevent optimization */
static inline int32_t copy_and_add(int32_t a, int32_t b, volatile int32_t* sink) {
    int32_t result = a + b;
    *sink = result;  /* volatile write to prevent elimination */
    return result;
}

static inline float promote_and_multiply(int16_t a, float b, volatile float* sink) {
    float promoted = (float)a;
    float result = promoted * b;
    *sink = result;
    return result;
}

static inline double mixed_calc(int32_t a, float b, double c, volatile double* sink) {
    double da = (double)a;
    double db = (double)b;
    double result = da * db + c;
    *sink = result;
    return result;
}

/* Function with complex control flow to create many basic blocks */
static void process_block(int32_t* int_data, float* float_data, 
                         double* double_data, size_t size,
                         volatile int32_t* v_int, volatile float* v_float,
                         volatile double* v_double) {
    
    /* Multiple local variables to create register pressure */
    int32_t t1, t2, t3, t4, t5;
    float f1, f2, f3, f4;
    double d1, d2, d3;
    
    /* Outer loop with multiple induction variables */
    for (size_t i = 0; i < size; i += 2) {
        /* First inner loop - integer operations */
        for (int j = 0; j < 8; j++) {
            /* Chain of dependent arithmetic operations */
            t1 = int_data[i] + j * 3;
            t2 = t1 - int_data[i + 1];
            t3 = t2 * 7;
            t4 = t3 >> 2;
            t5 = t4 ^ 0x55;
            
            /* Force copy propagation context */
            t1 = copy_and_add(t5, j, v_int);
            
            /* Use inline asm to clobber registers */
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3) : "memory");
            
            /* Conditional block creating control flow merge */
            if (j & 1) {
                t2 = t1 * 2;
                /* Another volatile sink */
                *v_int = t2;
            } else {
                t2 = t1 / 2;
                asm volatile("" : : "r"(t2));
            }
            
            /* Mixed precision calculations */
            f1 = promote_and_multiply((int16_t)t2, float_data[i], v_float);
            f2 = f1 * 1.5f;
            
            /* More register pressure with floating point */
            d1 = mixed_calc(t2, f2, double_data[i], v_double);
            d2 = d1 * 0.75;
            
            /* Use results to prevent elimination */
            asm volatile("" : : "r"(d2), "f"(f2));
        }
        
        /* Second inner loop with different operations */
        for (int k = 0; k < 4; k++) {
            /* Create more virtual registers with different modes */
            int32_t tmp1 = int_data[i] * k;
            int32_t tmp2 = tmp1 + 0x1000;
            
            /* Type conversions creating different RTL modes */
            float ftmp1 = (float)tmp2;
            float ftmp2 = ftmp1 / (k + 1.0f);
            
            double dtmp1 = (double)ftmp2;
            double dtmp2 = dtmp1 * 2.0;
            
            /* Force copies between different precision types */
            int32_t tmp3 = (int32_t)dtmp2;
            tmp3 = copy_and_add(tmp3, k, v_int);
            
            /* More inline asm to prevent optimization */
            asm volatile("" : : "r"(tmp3), "f"(ftmp2), "d"(dtmp2));
        }
        
        /* Conditional block with more complex operations */
        if (i % 3 == 0) {
            /* Create register pressure with many live values */
            int32_t a1 = int_data[i] * 3;
            int32_t a2 = a1 + 17;
            int32_t a3 = a2 ^ 0xFF;
            float b1 = float_data[i] * 2.0f;
            float b2 = b1 + 1.0f;
            double c1 = double_data[i] * 1.5;
            double c2 = c1 - 0.5;
            
            /* Use all values to keep them live */
            asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), 
                        "f"(b1), "f"(b2), "d"(c1), "d"(c2));
            
            /* Force another round of copy propagation */
            a1 = copy_and_add(a3, a2, v_int);
            b1 = promote_and_multiply((int16_t)a1, b2, v_float);
            c1 = mixed_calc(a1, b1, c2, v_double);
            
            /* Final volatile sinks */
            *v_int = a1;
            *v_float = b1;
            *v_double = c1;
        }
    }
}

/* Main function with initialization and multiple calls */
int main(void) {
    const size_t SIZE = 256;
    
    /* Allocate and initialize arrays with different patterns */
    int32_t* int_data = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float* float_data = (float*)malloc(SIZE * sizeof(float));
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize with patterns that create varied values */
    for (size_t i = 0; i < SIZE; i++) {
        int_data[i] = (int32_t)(i * 3 + 1);
        float_data[i] = (float)(i * 0.5f + 1.0f);
        double_data[i] = (double)(i * 0.25 + 0.5);
    }
    
    /* Volatile sinks to prevent optimization */
    volatile int32_t v_int_sink = 0;
    volatile float v_float_sink = 0.0f;
    volatile double v_double_sink = 0.0;
    
    /* Call processing function multiple times to increase pressure */
    for (int iter = 0; iter < 10; iter++) {
        process_block(int_data, float_data, double_data, SIZE,
                     &v_int_sink, &v_float_sink, &v_double_sink);
        
        /* Modify data slightly each iteration */
        for (size_t i = 0; i < SIZE; i++) {
            int_data[i] += iter;
            float_data[i] += (float)iter * 0.1f;
        }
    }
    
    /* Use results to prevent dead code elimination */
    asm volatile("" : : "r"(int_data[0]), "f"(float_data[0]), 
                 "d"(double_data[0]), "m"(v_int_sink), 
                 "m"(v_float_sink), "m"(v_double_sink));
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return (int)v_int_sink;
}
