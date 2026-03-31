/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force copies and prevent optimization */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    volatile int32_t v1 = a;
    int32_t tmp = v1 + b;
    asm volatile("" : "+r"(tmp) : : "memory");
    return tmp + c;
}

static inline double promote_and_multiply(float f, double d, int32_t i) {
    volatile float vf = f;
    double promoted = (double)vf;
    asm volatile("" : "+r"(i) : : "memory");
    return promoted * d * i;
}

static inline int64_t mix_types(int16_t s, int32_t i, uint8_t c) {
    volatile int16_t vs = s;
    int64_t result = (int64_t)vs * i;
    asm volatile("" : "+r"(c) : : "memory");
    return result + c;
}

/* Function to create complex control flow */
static void process_block(int32_t *int_data, float *float_data, 
                         double *double_data, int start, int end) {
    volatile int32_t sink;
    
    for (int i = start; i < end; i++) {
        /* Create many short-lived recomputable values */
        int32_t t1 = int_data[i] * 3;
        int32_t t2 = t1 + 7;
        int32_t t3 = copy_and_add(t2, int_data[i+1], 11);
        
        /* Force register pressure with mixed types */
        float f1 = float_data[i] * 2.5f;
        double d1 = promote_and_multiply(f1, double_data[i], t3);
        
        /* More computations with different modes */
        int16_t s1 = (int16_t)(t3 & 0xFFFF);
        uint8_t c1 = (uint8_t)(t3 & 0xFF);
        int64_t l1 = mix_types(s1, t3, c1);
        
        /* Conditional block to split control flow */
        if (i % 3 == 0) {
            int32_t t4 = t3 * 2;
            float f2 = f1 + 1.0f;
            double d2 = d1 * 1.5;
            
            /* More copies and computations */
            t4 = copy_and_add(t4, int_data[i], 13);
            d2 = promote_and_multiply(f2, d2, t4);
            
            /* Use volatile to prevent elimination */
            sink = t4;
            asm volatile("" : : "r"(d2) : "memory");
        } else if (i % 3 == 1) {
            /* Different computation path */
            int32_t t5 = t3 / 2;
            for (int j = 0; j < 2; j++) {
                t5 = copy_and_add(t5, j, 17);
                float f3 = float_data[i+j] * 0.75f;
                asm volatile("" : : "r"(f3) : "memory");
            }
            sink = t5;
        } else {
            /* Third path with more complex operations */
            int64_t l2 = l1 * 3;
            double d3 = (double)l2 / 1000.0;
            float f4 = (float)d3;
            
            /* Chain of dependent operations */
            for (int k = 0; k < 3; k++) {
                f4 = f4 * 1.1f + k;
                d3 = d3 * 0.9 - k;
                asm volatile("" : : "r"(f4), "r"(d3) : "memory");
            }
            sink = (int32_t)l2;
        }
        
        /* Final sink to prevent dead code elimination */
        asm volatile("" : : "r"(sink) : "memory");
    }
}

/* Main function with nested loops and complex data flow */
int main(void) {
    const int SIZE = 256;
    
    /* Initialize arrays with different patterns */
    int32_t *int_data = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float *float_data = (float*)malloc(SIZE * sizeof(float));
    double *double_data = (double*)malloc(SIZE * sizeof(double));
    
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = (i * 37) % 101;
        float_data[i] = (float)((i * 19) % 97) * 0.1f;
        double_data[i] = (double)((i * 53) % 103) * 0.01;
    }
    
    volatile int32_t global_sink = 0;
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Multiple inner loops with different strides */
        for (int block = 0; block < 4; block++) {
            int start = block * 64;
            int end = start + 64;
            
            /* Process with complex control flow */
            process_block(int_data, float_data, double_data, start, end);
            
            /* Inter-block computations */
            for (int i = start; i < end; i += 8) {
                int32_t sum = 0;
                float fsum = 0.0f;
                double dsum = 0.0;
                
                for (int j = 0; j < 8; j++) {
                    sum = copy_and_add(sum, int_data[i+j], j);
                    fsum += float_data[i+j];
                    dsum += double_data[i+j];
                }
                
                /* Force copies between different representations */
                int32_t isum = (int32_t)fsum;
                float ffromi = (float)sum;
                double dfromf = (double)fsum;
                
                /* Use inline asm to clobber registers */
                asm volatile("" : : "r"(sum), "r"(isum), "r"(ffromi), "r"(dfromf) : 
                           "xmm0", "xmm1", "xmm2", "xmm3");
                
                global_sink += sum;
            }
        }
        
        /* Modify data slightly each outer iteration */
        for (int i = 0; i < SIZE; i++) {
            int_data[i] = copy_and_add(int_data[i], outer, 1);
            float_data[i] = float_data[i] * (1.0f + outer * 0.01f);
        }
    }
    
    /* Final result computation with mixed types */
    int64_t final_result = 0;
    for (int i = 0; i < SIZE; i++) {
        int16_t s = (int16_t)(int_data[i] & 0xFFFF);
        uint8_t c = (uint8_t)(int_data[i] & 0xFF);
        final_result += mix_types(s, int_data[i], c);
        
        float f = float_data[i];
        double d = double_data[i];
        final_result += (int64_t)promote_and_multiply(f, d, i);
    }
    
    /* Prevent everything from being optimized away */
    asm volatile("" : : "r"(final_result), "r"(global_sink) : "memory");
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return (int)(final_result % 1000);
}
