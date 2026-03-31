/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(var) asm volatile("" : "+r"(var))
#define VOLATILE_SINK(var) do { volatile int sink = (var); (void)sink; } while(0)
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Helper functions to force copy propagation contexts */
static inline int32_t process_int(int32_t a, int32_t b, int32_t c) {
    /* Multiple dependent operations to create recomputable values */
    int32_t t1 = a + b * 3;
    int32_t t2 = t1 - c;
    int32_t t3 = t2 ^ (a << 2);
    FORCE_COPY(t3);
    return t3;
}

static inline float process_float(float a, float b, float c) {
    /* Mixed precision operations */
    float t1 = a * 1.5f + b;
    float t2 = t1 / (c + 0.001f);
    float t3 = t2 - a * 0.25f;
    FORCE_COPY(t3);
    return t3;
}

static inline int64_t process_long(int64_t a, int64_t b, int8_t c) {
    /* Type promotions and demotions */
    int16_t s1 = (int16_t)(a & 0xFFFF);
    int32_t i1 = (int32_t)s1 * (int32_t)c;
    int64_t l1 = (int64_t)i1 + b;
    int64_t l2 = l1 ^ (a >> 8);
    FORCE_COPY(l2);
    return l2;
}

/* Function with complex control flow */
static void process_block(int8_t* data_int8, int16_t* data_int16, 
                         float* data_float, double* data_double,
                         int start, int end, int stride) {
    volatile int guard = 1;
    
    for (int i = start; i < end; i += stride) {
        /* Create many short-lived values with different modes */
        int32_t base = data_int8[i] * 3;
        float fbase = data_float[i] * 2.0f;
        
        /* Multiple nested conditionals create complex CFG */
        if (guard && (i & 1)) {
            /* First computation path */
            int32_t v1 = process_int(base, data_int16[i], i);
            float v2 = process_float(fbase, data_float[i+1], v1);
            
            /* Force register pressure with many temporaries */
            int32_t t1 = v1 + 7;
            int32_t t2 = t1 * 3;
            int32_t t3 = t2 - v1;
            int32_t t4 = t3 ^ 0xFF;
            float t5 = v2 * 1.1f;
            float t6 = t5 + v2;
            double t7 = (double)t4 + (double)t6;
            
            /* Use inline asm to clobber registers */
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
            asm volatile("" : : "x"(t5), "x"(t6));
            
            VOLATILE_SINK(t7);
        } else if (guard && (i & 2)) {
            /* Second computation path with different types */
            int64_t v1 = process_long(base, data_int16[i], data_int8[i]);
            double v2 = (double)data_float[i] * 3.14159;
            
            /* Chain of dependent operations */
            int16_t s1 = (int16_t)(v1 & 0xFFFF);
            int32_t i1 = s1 * 255;
            int64_t l1 = i1 + v1;
            float f1 = (float)l1 * 0.5f;
            double d1 = (double)f1 + v2;
            
            /* Force copies between different register classes */
            int64_t copy1 = l1;
            float copy2 = f1;
            double copy3 = d1;
            
            FORCE_COPY(copy1);
            FORCE_COPY(copy2);
            FORCE_COPY(copy3);
            
            VOLATILE_SINK(d1);
        } else {
            /* Third path with mixed operations */
            for (int j = 0; j < 3; j++) {
                /* Inner loop creates more pressure */
                int32_t tmp1 = base + j * 17;
                float tmp2 = fbase + j * 0.25f;
                
                /* Cross-type operations */
                int32_t tmp3 = (int32_t)(tmp2 * 100.0f);
                float tmp4 = (float)tmp1 * 0.01f;
                
                /* More recomputable values */
                int32_t tmp5 = tmp1 ^ tmp3;
                float tmp6 = tmp2 + tmp4;
                double tmp7 = (double)tmp5 * (double)tmp6;
                
                /* Force spills and remats */
                asm volatile("" : : "r"(tmp1), "r"(tmp3), "r"(tmp5));
                asm volatile("" : : "x"(tmp2), "x"(tmp4), "x"(tmp6));
                
                data_double[i + j] = tmp7;
            }
        }
        
        /* Periodic barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
}

/* Main driver with multiple phases */
int main(void) {
    const int SIZE = 1024;
    
    /* Allocate and initialize arrays with different patterns */
    int8_t* data_int8 = (int8_t*)malloc(SIZE * sizeof(int8_t));
    int16_t* data_int16 = (int16_t*)malloc(SIZE * sizeof(int16_t));
    float* data_float = (float*)malloc(SIZE * sizeof(float));
    double* data_double = (double*)malloc(SIZE * sizeof(double));
    
    for (int i = 0; i < SIZE; i++) {
        data_int8[i] = (int8_t)((i * 37) & 0xFF);
        data_int16[i] = (int16_t)((i * 73) & 0xFFFF);
        data_float[i] = (float)(i * 0.12345f);
        data_double[i] = (double)(i * 0.6789);
    }
    
    volatile int iter_guard = 1;
    
    /* Outer loop creates multiple compilation units */
    for (int outer = 0; outer < 4 && iter_guard; outer++) {
        /* Process in different strides and blocks */
        for (int block = 0; block < 8; block++) {
            int start = block * 128;
            int end = start + 128;
            int stride = 1 + (block & 3);
            
            /* Call processing function with different parameters */
            process_block(data_int8, data_int16, data_float, data_double,
                         start, end, stride);
            
            /* Mix in some direct computations */
            for (int i = start; i < start + 16; i++) {
                /* Create more virtual register pressure */
                int32_t a = data_int8[i];
                int32_t b = data_int16[i];
                float c = data_float[i];
                
                /* Long dependency chain */
                int32_t r1 = a + b;
                int32_t r2 = r1 * 2;
                float r3 = c * (float)r2;
                int32_t r4 = (int32_t)r3 + r1;
                float r5 = (float)r4 * 0.5f;
                double r6 = (double)r5 + (double)r2;
                
                /* Force copies through helper calls */
                int32_t copy1 = process_int(r1, r2, r4);
                float copy2 = process_float(r3, r5, (float)copy1);
                
                /* Use results to prevent elimination */
                data_double[i] += r6 + (double)copy1 + (double)copy2;
                
                /* More register clobbering */
                asm volatile("" : : "r"(r1), "r"(r2), "r"(r4), "r"(copy1));
                asm volatile("" : : "x"(r3), "x"(r5), "x"(copy2));
            }
        }
        
        /* Shuffle data to create new patterns */
        for (int i = 0; i < SIZE - 1; i++) {
            data_int8[i] ^= data_int8[i + 1];
            data_int16[i] += data_int16[i + 1];
            data_float[i] = data_float[i + 1] * 0.9f;
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Final reduction to use all values */
    double sum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        sum += data_double[i] + (double)data_int8[i] + (double)data_int16[i] + (double)data_float[i];
    }
    
    VOLATILE_SINK(sum);
    
    free(data_int8);
    free(data_int16);
    free(data_float);
    free(data_double);
    
    return (int)(sum / SIZE) & 0xFF;
}
