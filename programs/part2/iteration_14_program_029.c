/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force copies and prevent optimization */
static inline int32_t copy_and_add(int32_t a, int32_t b, volatile int32_t* sink) {
    int32_t result = a + b;
    *sink = result;  /* volatile write to prevent elimination */
    return result;
}

static inline float promote_and_multiply(int16_t s, float f, volatile float* sink) {
    float promoted = (float)s;
    float result = promoted * f;
    *sink = result;
    return result;
}

static inline double mixed_calc(int32_t i, float f, double d, volatile double* sink) {
    double fi = (double)i;
    double ff = (double)f;
    double result = fi * ff + d;
    *sink = result;
    /* Force register clobbering */
    asm volatile("" : : "r"(result), "x"(result) : "memory");
    return result;
}

/* Function with complex control flow to create many basic blocks */
static void process_chunk(int8_t* data_int8, int16_t* data_int16, 
                         float* data_float, double* data_double,
                         int start, int end, volatile int* global_sink) {
    int i, j;
    
    /* Outer loop with multiple induction variables */
    for (i = start; i < end; i += 3) {
        int32_t base_val = (int32_t)data_int8[i] * 256;
        float float_acc = 0.0f;
        double double_acc = 0.0;
        
        /* Inner loop 1: integer operations */
        for (j = 0; j < 8; ++j) {
            /* Create dependent chain of integer computations */
            int32_t temp1 = base_val + j * 37;
            int32_t temp2 = temp1 ^ 0x55AA55AA;
            int32_t temp3 = temp2 * 3;
            
            /* Force copy propagation context */
            int32_t copied = copy_and_add(temp3, data_int16[j], (volatile int32_t*)global_sink);
            
            /* Use result in next iteration */
            base_val = (copied >> 4) + 1;
            
            /* Volatile read to block CSE */
            asm volatile("" : "+r"(base_val) : : "memory");
        }
        
        /* Inner loop 2: mixed integer/float operations */
        for (j = 0; j < 4; ++j) {
            /* Type conversions and promotions */
            int16_t short_val = (int16_t)(base_val & 0xFFFF);
            float float_val = data_float[(i + j) % 128];
            
            /* Multiple virtual register opportunities */
            float promoted = promote_and_multiply(short_val, float_val, 
                                                 (volatile float*)global_sink);
            
            /* Chain computations */
            float_acc = float_acc * 0.99f + promoted;
            
            /* Conditional to split basic block */
            if (float_acc > 1000.0f) {
                float_acc = float_acc * 0.5f;
                /* Inline asm to force spills */
                asm volatile("" : : "r"(float_acc), "m"(*global_sink) : "memory");
            }
        }
        
        /* Inner loop 3: double precision */
        for (j = 0; j < 2; ++j) {
            /* Complex expression with multiple intermediate values */
            double dbl_result = mixed_calc(base_val, float_acc, 
                                          data_double[(i + j) % 64],
                                          (volatile double*)global_sink);
            
            double_acc += dbl_result;
            
            /* More register pressure */
            int32_t int_from_double = (int32_t)dbl_result;
            base_val = base_val ^ int_from_double;
            
            /* Force virtual register creation */
            asm volatile("" : "+r"(base_val), "+r"(double_acc) : : 
                        "xmm0", "xmm1", "xmm2", "xmm3", "memory");
        }
        
        /* Store final result (volatile to prevent elimination) */
        *global_sink = (int)(float_acc + double_acc + base_val);
    }
}

/* Main function with initialization and multiple passes */
int main(void) {
    const int SIZE = 256;
    int8_t* data_int8 = (int8_t*)malloc(SIZE * sizeof(int8_t));
    int16_t* data_int16 = (int16_t*)malloc(SIZE * sizeof(int16_t));
    float* data_float = (float*)malloc(SIZE * sizeof(float));
    double* data_double = (double*)malloc(SIZE * sizeof(double));
    
    volatile int global_sink = 0;
    
    /* Initialize with different patterns */
    for (int i = 0; i < SIZE; ++i) {
        data_int8[i] = (int8_t)((i * 13) & 0xFF);
        data_int16[i] = (int16_t)((i * 17) & 0xFFFF);
        data_float[i] = (float)(i * 0.123f);
        data_double[i] = (double)(i * 0.456);
    }
    
    /* Multiple processing passes with different chunk sizes */
    for (int pass = 0; pass < 3; ++pass) {
        int chunk_size = 32 >> pass;  /* Varying chunk sizes: 32, 16, 8 */
        
        for (int chunk = 0; chunk < SIZE; chunk += chunk_size) {
            /* Process with different starting alignments */
            int start = chunk + (pass % 4);
            if (start >= SIZE) start = SIZE - 1;
            
            int end = start + chunk_size;
            if (end > SIZE) end = SIZE;
            
            /* This creates the virtual register pressure context */
            process_chunk(data_int8, data_int16, data_float, data_double,
                         start, end, &global_sink);
            
            /* Additional computation between chunks to force rematerialization */
            int32_t temp = global_sink;
            for (int k = 0; k < 16; ++k) {
                temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
                /* Force copies between different type temporaries */
                int32_t temp2 = temp;
                float temp_f = (float)temp2;
                double temp_d = (double)temp_f;
                int32_t temp3 = (int32_t)temp_d;
                
                /* Use inline asm to prevent coalescing */
                asm volatile("" : "+r"(temp), "+r"(temp2), "+r"(temp3) : : "memory");
                
                global_sink = temp3;
            }
        }
    }
    
    /* Final mixed-type computation chain */
    double final_acc = 0.0;
    for (int i = 0; i < 100; ++i) {
        int32_t int_val = global_sink + i;
        float float_val = data_float[i % SIZE];
        double double_val = data_double[i % SIZE];
        
        /* Complex expression with many virtual registers */
        double result = mixed_calc(int_val, float_val, double_val, 
                                  (volatile double*)&global_sink);
        
        final_acc += result;
        
        /* Conditional with different mode registers */
        if (i % 7 == 0) {
            float temp_f = (float)final_acc;
            int temp_i = (int)temp_f;
            global_sink = copy_and_add(temp_i, i, &global_sink);
            
            /* Force different register modes */
            asm volatile("" : : "r"(temp_i), "r"(temp_f), "x"(final_acc) : 
                        "xmm4", "xmm5", "xmm6", "memory");
        }
    }
    
    free(data_int8);
    free(data_int16);
    free(data_float);
    free(data_double);
    
    return (int)final_acc & 0xFF;
}
