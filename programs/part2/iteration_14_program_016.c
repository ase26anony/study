/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -S -o test.s test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5")

/* Helper functions to force copy propagation */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    int32_t t1 = a + b;
    int32_t t2 = t1 ^ c;
    FORCE_COPY(t2);
    return t2 - a;
}

static inline float copy_and_mul(float a, float b, float c) {
    float t1 = a * b;
    float t2 = t1 + c;
    FORCE_COPY(t2);
    return t2 / a;
}

static inline double mixed_compute(int32_t a, float b, double c) {
    double t1 = (double)a + (double)b;
    double t2 = t1 * c;
    FORCE_COPY(t2);
    return t2 - (double)(a * (int32_t)b);
}

/* Complex kernel with nested loops and mixed operations */
void __attribute__((noinline))
compute_kernel(int32_t* int_data, float* float_data, double* double_data, 
               int size, int iterations) {
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Outer loop creates register pressure */
        int32_t accum_int = iter;
        float accum_float = iter * 0.5f;
        double accum_double = iter * 0.25;
        
        /* First inner loop - integer heavy */
        for (int i = 0; i < size; i++) {
            /* Create many short-lived recomputable values */
            int32_t tmp1 = int_data[i] + 7;
            int32_t tmp2 = tmp1 * 3;
            int32_t tmp3 = tmp2 ^ 0x55AA55AA;
            
            /* Force copies between different scopes */
            if (i & 1) {
                tmp3 = copy_and_add(tmp3, accum_int, i);
                accum_int += tmp3;
            } else {
                accum_int -= tmp3;
            }
            
            /* Mixed precision calculations */
            float ftmp = (float)tmp3 * 0.123f;
            accum_float += copy_and_mul(ftmp, accum_float, float_data[i]);
            
            /* Volatile sink to prevent elimination */
            VOLATILE_SINK(accum_int);
            CLASH_REGISTERS;
        }
        
        /* Second inner loop - floating point heavy */
        for (int j = 0; j < size; j += 2) {
            /* Create dependent chain of FP operations */
            float f1 = float_data[j] * 1.5f;
            float f2 = f1 + float_data[j + 1];
            float f3 = f2 / 3.14159f;
            
            /* Promote to double and back */
            double d1 = (double)f3 * 2.0;
            double d2 = d1 + accum_double;
            
            /* Mixed precision computation forcing copies */
            accum_double = mixed_compute(accum_int, f3, d2);
            
            /* Integer operations in FP loop */
            int32_t itmp = (int32_t)d2;
            accum_int ^= copy_and_add(itmp, j, accum_int);
            
            /* More register clobbering */
            asm volatile("" : : "r"(accum_int), "r"(accum_float), "r"(accum_double));
        }
        
        /* Conditional block with different register usage */
        if (iter % 3 == 0) {
            /* Use different types to create different register modes */
            int16_t short_val = accum_int & 0xFFFF;
            int64_t long_val = (int64_t)accum_int * (int64_t)accum_float;
            
            /* Force conversions and copies */
            float from_short = (float)short_val;
            double from_long = (double)long_val;
            
            accum_float = copy_and_mul(from_short, accum_float, 1.0f);
            accum_double += from_long;
            
            /* Complex expression with many temporaries */
            for (int k = 0; k < 4; k++) {
                double complex_tmp = (double)k * accum_double;
                float float_tmp = (float)complex_tmp;
                int32_t int_tmp = (int32_t)float_tmp;
                
                accum_int += copy_and_add(int_tmp, k, short_val);
                accum_float *= float_tmp;
                
                VOLATILE_SINK(complex_tmp);
            }
        } else if (iter % 3 == 1) {
            /* Alternative path with different operations */
            for (int m = 0; m < 8; m++) {
                /* Create many virtual registers */
                int32_t v1 = accum_int + m;
                int32_t v2 = v1 * 2;
                int32_t v3 = v2 - accum_int;
                int32_t v4 = v3 ^ v1;
                int32_t v5 = v4 + m;
                
                /* Chain them together */
                accum_int = copy_and_add(v5, v2, v3);
                
                /* Floating point side chain */
                float fv1 = (float)v5 * 0.01f;
                float fv2 = fv1 + accum_float;
                accum_float = copy_and_mul(fv2, 0.99f, accum_float);
            }
        }
        
        /* Final store with volatile to prevent optimization */
        VOLATILE_SINK(accum_int);
        VOLATILE_SINK(accum_float);
        VOLATILE_SINK(accum_double);
    }
}

/* Initialize with patterns that create varied computation */
void init_data(int32_t* int_data, float* float_data, double* double_data, int size) {
    for (int i = 0; i < size; i++) {
        int_data[i] = (i * 37) ^ 0x12345678;
        float_data[i] = (float)i * 0.73f - 12.5f;
        double_data[i] = (double)i * 1.23456789 + 45.678;
    }
}

int main() {
    const int SIZE = 256;
    const int ITERS = 100;
    
    /* Allocate aligned to avoid unnecessary checks */
    int32_t* int_data = __builtin_aligned_alloc(64, SIZE * sizeof(int32_t));
    float* float_data = __builtin_aligned_alloc(64, SIZE * sizeof(float));
    double* double_data = __builtin_aligned_alloc(64, SIZE * sizeof(double));
    
    if (!int_data || !float_data || !double_data) return 1;
    
    init_data(int_data, float_data, double_data, SIZE);
    
    /* Multiple calls to increase optimization opportunities */
    for (int run = 0; run < 3; run++) {
        compute_kernel(int_data, float_data, double_data, SIZE, ITERS);
    }
    
    /* Use results to prevent dead code elimination */
    volatile int32_t final_int = int_data[0];
    volatile float final_float = float_data[0];
    volatile double final_double = double_data[0];
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return (final_int > 0) ? 0 : 1;
}
