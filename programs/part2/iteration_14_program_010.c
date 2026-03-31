/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5")

/* Helper functions to force copy propagation contexts */
static inline int32_t copy_and_transform(int32_t a, int32_t b, int mode) {
    volatile int32_t temp = a + b;
    FORCE_COPY(temp);
    switch(mode & 3) {
        case 0: return temp * 2;
        case 1: return temp / 2;
        case 2: return temp ^ 0x5555;
        default: return temp + 1;
    }
}

static inline float float_copy_propagate(float a, float b, int iter) {
    volatile float v1 = a;
    volatile float v2 = b;
    FORCE_COPY(v1);
    FORCE_COPY(v2);
    return (iter & 1) ? v1 * v2 : v1 + v2;
}

static inline int16_t narrow_copy(int32_t val) {
    volatile int16_t narrow = (int16_t)(val & 0xFFFF);
    FORCE_COPY(narrow);
    return narrow + 1;
}

/* Main computation kernel */
void compute_kernel(int32_t* int_data, float* float_data, 
                    int8_t* char_data, double* double_data,
                    int outer_iters, int inner_iters) {
    
    for (int outer = 0; outer < outer_iters; ++outer) {
        /* Create register pressure with many live values */
        volatile int32_t base = int_data[outer % 1024];
        volatile float fbase = float_data[outer % 1024];
        volatile double dbase = double_data[outer % 1024];
        
        FORCE_COPY(base);
        FORCE_COPY(fbase);
        FORCE_COPY(dbase);
        
        /* First inner loop - integer computations */
        for (int i = 0; i < inner_iters; ++i) {
            /* Chain of dependent computations creating virtual registers */
            int32_t v1 = base + i * 3;
            int32_t v2 = v1 ^ 0x12345678;
            int32_t v3 = v2 * 2 - 1;
            int32_t v4 = copy_and_transform(v3, i, outer);
            int32_t v5 = v4 + (v3 >> 4);
            
            /* Force copies between different scopes */
            {
                volatile int32_t scope_copy = v5;
                v1 = scope_copy + 1;
            }
            
            /* Mixed-type computations */
            float f1 = (float)v1 * 0.5f;
            float f2 = float_copy_propagate(f1, fbase, i);
            int32_t v6 = (int32_t)f2 + v5;
            
            /* Narrow/widen conversions */
            int16_t s1 = narrow_copy(v6);
            int32_t v7 = (int32_t)s1 * 2;
            
            /* Conditional block creating control flow complexity */
            if (i & 1) {
                volatile int32_t cond_val = v7 + outer;
                v6 = cond_val * 3;
                FORCE_COPY(v6);
            } else {
                volatile float cond_float = f2 * 2.0f;
                f1 = cond_float + 1.0f;
                FORCE_COPY(f1);
            }
            
            /* More register pressure */
            int32_t v8 = v6 + (v7 & 0xFF);
            float f3 = f1 * 1.5f;
            double d1 = (double)f3 + dbase;
            
            /* Use results to prevent elimination */
            char_data[(outer * inner_iters + i) % 1024] = (int8_t)(v8 & 0xFF);
            VOLATILE_SINK(f3);
            VOLATILE_SINK(d1);
            
            /* Clobber registers to force spills/reloads */
            if (i % 16 == 0) {
                CLASH_REGISTERS;
            }
        }
        
        /* Second inner loop - floating-point intensive */
        for (int j = 0; j < inner_iters / 2; ++j) {
            volatile float accum = fbase;
            volatile double daccum = dbase;
            
            for (int k = 0; k < 4; ++k) {
                float f1 = accum * (float)(j + k + 1);
                float f2 = float_copy_propagate(f1, accum, k);
                double d1 = (double)f2 * 1.25;
                double d2 = daccum + d1;
                
                /* Force copies through inline asm */
                asm volatile("" : "+r"(f1), "+r"(f2));
                asm volatile("" : "+r"(d1), "+r"(d2));
                
                accum = f2;
                daccum = d2;
                
                /* Integer side computation */
                int32_t side = base + j * k;
                side = copy_and_transform(side, k, j);
                VOLATILE_SINK(side);
            }
            
            VOLATILE_SINK(accum);
            VOLATILE_SINK(daccum);
        }
        
        /* Third loop with complex control flow */
        for (int m = 0; m < inner_iters; m += 3) {
            int32_t val = base + m;
            
            switch (m % 5) {
                case 0:
                    val = copy_and_transform(val, outer, 0);
                    break;
                case 1:
                    val = val * 2 - 1;
                    break;
                case 2: {
                    volatile int32_t switch_val = val + 1000;
                    val = switch_val / 2;
                    break;
                }
                case 3:
                    val = narrow_copy(val) * 3;
                    break;
                default:
                    val = val ^ 0xAAAAAAAA;
            }
            
            /* Nested conditional */
            if (val > 0) {
                float fval = (float)val * 0.25f;
                if (fval < 1000.0f) {
                    volatile float temp = fval * 2.0f;
                    fval = temp;
                }
                VOLATILE_SINK(fval);
            } else {
                double dval = (double)val * (-0.5);
                VOLATILE_SINK(dval);
            }
            
            VOLATILE_SINK(val);
        }
    }
}

/* Initialize with varied data patterns */
void init_data(int32_t* int_data, float* float_data,
               int8_t* char_data, double* double_data, int size) {
    for (int i = 0; i < size; ++i) {
        int_data[i] = (i * 37) ^ 0x12345678;
        float_data[i] = (float)(i * 0.12345f);
        char_data[i] = (int8_t)(i * 3);
        double_data[i] = (double)(i * 0.6789);
    }
}

int main() {
    const int data_size = 1024;
    const int outer_iters = 100;
    const int inner_iters = 50;
    
    /* Allocate and initialize data */
    int32_t* int_data = (int32_t*)malloc(data_size * sizeof(int32_t));
    float* float_data = (float*)malloc(data_size * sizeof(float));
    int8_t* char_data = (int8_t*)malloc(data_size * sizeof(int8_t));
    double* double_data = (double*)malloc(data_size * sizeof(double));
    
    init_data(int_data, float_data, char_data, double_data, data_size);
    
    /* Run computation kernel multiple times */
    for (int run = 0; run < 3; ++run) {
        compute_kernel(int_data, float_data, char_data, double_data,
                      outer_iters, inner_iters);
    }
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(char_data);
    free(double_data);
    
    return 0;
}
