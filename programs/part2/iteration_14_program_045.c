/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -finline-small-functions -fno-tree-pre -fdump-rtl-expand */

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
    *sink = tmp;  /* Volatile write */
    return tmp / (a + 1.0f);
}

static inline double process_double(double a, double b, volatile double* sink) {
    double tmp = a * 2.71828 + b * 0.367879;
    *sink = tmp;  /* Volatile write */
    return tmp - a * 0.5;
}

/* Function with mixed operations to create different register modes */
static inline int16_t mixed_ops(uint8_t a, int16_t b, int32_t c, float d, volatile int* sink) {
    /* Promote uint8_t to int32_t */
    int32_t t1 = (int32_t)a * 3;
    
    /* Convert float to int */
    int32_t t2 = (int32_t)(d * 100.0f);
    
    /* Mixed-width arithmetic */
    int32_t t3 = t1 + c - t2;
    
    /* Narrow to int16_t with saturation */
    int16_t result = (int16_t)CLAMP(t3, -32768, 32767);
    
    *sink = t3;  /* Force use of intermediate */
    FORCE_USE(b);  /* Force register use */
    
    return result + b;
}

/* Main computation kernel */
void compute_kernel(int32_t* int_data, float* float_data, double* double_data, 
                   size_t size, volatile int* global_sink) {
    volatile int32_t vsink_int __attribute__((unused));
    volatile float vsink_float __attribute__((unused));
    volatile double vsink_double __attribute__((unused)));
    
    /* Multiple nested loops to create complex CFG */
    for (size_t i = 0; i < size; i++) {
        /* Outer loop creates register pressure */
        int32_t base_int = int_data[i];
        float base_float = float_data[i];
        double base_double = double_data[i];
        
        FORCE_USE(base_int);
        
        /* First inner loop with integer focus */
        for (int j = 0; j < 8; j++) {
            /* Chain of dependent integer operations */
            int32_t v1 = base_int + j * 7;
            int32_t v2 = v1 - (j << 3);
            int32_t v3 = v2 * 3 + 11;
            
            /* Force copy through function call */
            int32_t v4 = process_int(v3, base_int, &vsink_int);
            
            /* More operations creating short-lived values */
            int32_t v5 = v4 ^ 0x55AA55AA;
            int32_t v6 = v5 + (v3 >> 2);
            
            /* Use inline asm to clobber registers */
            asm volatile("" : : "r"(v5), "r"(v6));
            
            /* Conditional block to split control flow */
            if (j & 1) {
                int32_t v7 = v6 * 2 - 5;
                vsink_int = v7;
                base_int = v7 & 0xFF;
            } else {
                int32_t v7 = v6 / 2 + 1;
                FORCE_USE(v7);
                base_int = v7 | 0x80;
            }
        }
        
        /* Second inner loop with floating-point focus */
        for (int k = 0; k < 4; k++) {
            /* Mixed precision calculations */
            float f1 = base_float + k * 0.125f;
            float f2 = f1 * 2.0f - 1.0f;
            
            /* Force float copy propagation */
            float f3 = process_float(f2, base_float, &vsink_float);
            
            /* Convert to different type */
            int32_t i1 = (int32_t)(f3 * 1000.0f);
            
            /* More operations */
            float f4 = f3 / (k + 2.0f);
            double d1 = (double)f4 + base_double;
            
            /* Force double operations */
            double d2 = process_double(d1, base_double, &vsink_double);
            
            /* Mixed-type expression */
            float f5 = (float)d2 + f4 * 0.5f;
            vsink_float = f5;
            
            /* Inline asm with multiple clobbers */
            asm volatile("" : : "r"(i1), "x"(f5), "x"(d2));
        }
        
        /* Third loop with mixed operations */
        for (int m = 0; m < 6; m++) {
            /* Create many short-lived values */
            uint8_t b1 = (uint8_t)(base_int + m);
            int16_t s1 = (int16_t)(m * 100);
            int32_t i2 = base_int * m;
            float f6 = base_float * (m + 1);
            
            /* Function with multiple parameters forcing copies */
            int16_t result = mixed_ops(b1, s1, i2, f6, &vsink_int);
            
            /* Complex expression tree */
            int32_t i3 = (int32_t)result * 3 + m * 7;
            float f7 = (float)i3 * 0.01f + base_float;
            double d3 = (double)f7 * 1.1 + base_double;
            
            /* Force all values to be used */
            FORCE_USE(i3);
            vsink_float = f7;
            vsink_double = d3;
            
            /* Conditional with different operations */
            if (m % 3 == 0) {
                base_int = i3 & 0xFFFF;
            } else if (m % 3 == 1) {
                base_float = f7 * 0.9f;
            } else {
                base_double = d3 * 0.8;
            }
        }
        
        /* Write back results */
        int_data[i] = base_int;
        float_data[i] = base_float;
        double_data[i] = base_double;
        
        *global_sink = i;  /* Volatile sink */
    }
}

/* Initialize with pattern to avoid constant propagation */
void init_data(int32_t* int_data, float* float_data, double* double_data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        int_data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_data[i] = (float)((i * 17) % 100) * 0.123f;
        double_data[i] = (double)((i * 23) % 200) * 0.456;
    }
}

int main(void) {
    const size_t SIZE = 256;
    
    /* Allocate on heap to avoid stack optimizations */
    int32_t* int_data = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float* float_data = (float*)malloc(SIZE * sizeof(float));
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    volatile int global_sink = 0;
    
    if (!int_data || !float_data || !double_data) {
        return 1;
    }
    
    init_data(int_data, float_data, double_data, SIZE);
    
    /* Call kernel multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 3; iter++) {
        compute_kernel(int_data, float_data, double_data, SIZE, &global_sink);
        
        /* Scramble data between iterations */
        for (size_t i = 0; i < SIZE; i++) {
            int_data[i] ^= 0x12345678;
            float_data[i] += 0.789f;
            double_data[i] *= 1.234;
        }
    }
    
    /* Final volatile sink */
    asm volatile("" : : "r"(int_data), "r"(float_data), "r"(double_data));
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
