/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent optimization */
static volatile int volatile_sink_int;
static volatile double volatile_sink_double;

/* Helper functions to force copies and parameter passing */
static inline int32_t compute_int(int32_t a, int32_t b, int32_t c) {
    /* Force register copies through parameter passing */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c));
    return (a * b) + (c >> 3);
}

static inline float compute_float(float a, float b, int c) {
    /* Mixed-type computation */
    asm volatile("" : "+r"(c));
    return a * b + (float)c * 0.5f;
}

static inline double compute_double(double a, double b, double c, int d) {
    /* Force multiple parameter copies */
    asm volatile("" : "+r"(d));
    return (a + b) * c - (double)d;
}

/* Function to create complex control flow */
static int process_chunk(int8_t* data_int8, int16_t* data_int16, 
                         float* data_float, double* data_double,
                         int start, int end, int outer_idx) {
    int result = 0;
    float float_acc = 0.0f;
    double double_acc = 0.0;
    
    /* Create multiple basic blocks with conditionals */
    for (int i = start; i < end; i++) {
        /* Multiple dependent computations */
        int8_t val8 = data_int8[i];
        int16_t val16 = data_int16[i % 256];
        
        /* Force type promotions and demotions */
        int temp1 = (int)val8 * 3;
        int temp2 = (int)val16 / 2;
        
        /* Use helper to force copy propagation */
        int temp3 = compute_int(temp1, temp2, outer_idx + i);
        
        /* Mixed precision floating point */
        float f1 = data_float[i % 128] * 1.5f;
        float f2 = compute_float(f1, (float)temp3, i);
        
        /* More integer computations */
        int temp4 = temp3 ^ (i * 7);
        int temp5 = (temp4 << 2) | (temp4 >> 30);
        
        /* Double precision with parameter passing */
        double d1 = data_double[i % 64] * 2.0;
        double d2 = compute_double(d1, (double)f2, 3.14159, temp5);
        
        /* Accumulate results */
        float_acc += f2;
        double_acc += d2;
        result += temp5;
        
        /* Volatile writes to prevent elimination */
        if (i % 32 == 0) {
            volatile_sink_int = temp3;
            volatile_sink_double = d2;
        }
        
        /* Create another basic block with conditional */
        if (temp5 > 1000) {
            /* More computations in conditional block */
            int temp6 = compute_int(temp5, i, outer_idx);
            float f3 = compute_float(float_acc, (float)temp6, i);
            result -= temp6;
            float_acc -= f3;
            
            /* Inline asm to clobber registers */
            asm volatile("" : : "r"(temp6), "r"(f3) : "memory");
        }
    }
    
    /* Final mixing */
    result += (int)float_acc + (int)double_acc;
    return result;
}

int main(void) {
    /* Allocate arrays with different patterns */
    const int SIZE = 1024;
    int8_t* data_int8 = (int8_t*)malloc(SIZE * sizeof(int8_t));
    int16_t* data_int16 = (int16_t*)malloc(256 * sizeof(int16_t));
    float* data_float = (float*)malloc(128 * sizeof(float));
    double* data_double = (double*)malloc(64 * sizeof(double));
    
    /* Initialize with patterns */
    for (int i = 0; i < SIZE; i++) {
        data_int8[i] = (int8_t)((i * 37) & 0xFF);
    }
    for (int i = 0; i < 256; i++) {
        data_int16[i] = (int16_t)((i * 73) & 0x7FFF);
    }
    for (int i = 0; i < 128; i++) {
        data_float[i] = (float)(i * 0.12345f);
    }
    for (int i = 0; i < 64; i++) {
        data_double[i] = (double)(i * 0.98765);
    }
    
    int total_result = 0;
    
    /* Outer loop creating register pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Multiple inner loops with different strides */
        for (int chunk = 0; chunk < 4; chunk++) {
            int start = chunk * 256;
            int end = start + 256;
            
            /* Process with mixed types */
            int chunk_result = process_chunk(data_int8, data_int16, 
                                            data_float, data_double,
                                            start, end, outer);
            
            /* More computations between chunks */
            int temp = compute_int(chunk_result, outer, chunk);
            float ftemp = compute_float((float)chunk_result, 
                                       (float)outer, chunk);
            
            total_result += temp + (int)ftemp;
            
            /* Volatile barrier */
            asm volatile("" : : "r"(temp), "r"(ftemp) : "memory");
        }
        
        /* Additional loop with different computation pattern */
        double loop_acc = 0.0;
        for (int i = 0; i < 128; i++) {
            /* Chain of dependent floating point ops */
            double d1 = data_double[i % 64];
            double d2 = compute_double(d1, (double)i, 2.71828, outer);
            double d3 = d2 * d2 - d1;
            loop_acc += d3;
            
            /* Integer side computation */
            int idx = (i + outer) & 0xFF;
            int8_t v8 = data_int8[idx];
            int16_t v16 = data_int16[idx % 256];
            int side = compute_int(v8, v16, i);
            
            /* Mix results */
            if (side % 7 == 0) {
                loop_acc += (double)side * 0.01;
            }
        }
        
        total_result += (int)loop_acc;
        volatile_sink_double = loop_acc;
    }
    
    /* Final output to prevent dead code elimination */
    volatile_sink_int = total_result;
    
    free(data_int8);
    free(data_int16);
    free(data_float);
    free(data_double);
    
    return total_result & 0xFF;
}
