/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -finline-small-functions -fno-tree-pre -fdump-rtl-expand */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_USE(x) asm volatile("" : : "r"(x))
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Helper functions to force copies and virtual register usage */
static inline int32_t compute_int(int32_t a, int32_t b, int32_t c) {
    volatile int32_t v1 = a * 3;
    volatile int32_t v2 = b / 5;
    int32_t result = (v1 + v2) * c - 7;
    FORCE_USE(result);
    return result;
}

static inline float compute_float(float a, float b, int32_t c) {
    volatile float v1 = a * 1.5f;
    volatile float v2 = b + 2.8f;
    float result = (v1 - v2) * (float)c;
    FORCE_USE(result);
    return result;
}

static inline double compute_double(double a, int16_t b, uint8_t c) {
    volatile double v1 = a * 0.75;
    volatile double v2 = (double)b * 2.3;
    double result = v1 + v2 + (double)c;
    FORCE_USE(result);
    return result;
}

/* Function with mixed precision and complex control flow */
static void process_block(int32_t* int_data, float* float_data, 
                         double* double_data, size_t size, int iter) {
    int32_t acc_int = 0;
    float acc_float = 0.0f;
    double acc_double = 0.0;
    
    /* Outer loop creates register pressure */
    for (size_t i = 0; i < size; i++) {
        volatile int32_t base = int_data[i] ^ iter;
        
        /* First inner loop with integer operations */
        for (int j = 0; j < 4; j++) {
            int32_t tmp1 = base + j * 17;
            int32_t tmp2 = tmp1 - (j << 3);
            int32_t tmp3 = compute_int(tmp1, tmp2, j + 1);
            
            /* Conditional block to split control flow */
            if (tmp3 & 1) {
                float f_tmp = (float)tmp3 * 0.25f;
                float_data[i] += compute_float(f_tmp, acc_float, j);
                
                /* Nested conditional */
                if (j % 2 == 0) {
                    double d_tmp = (double)tmp3 * 0.125;
                    double_data[i] += compute_double(d_tmp, (int16_t)j, (uint8_t)i);
                }
            } else {
                int32_t alt = tmp3 | 0xFF;
                acc_int += compute_int(alt, base, j);
            }
            
            /* Force register clobbering */
            asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        }
        
        /* Second inner loop with different operations */
        for (int k = 0; k < 3; k++) {
            /* Mixed type calculations */
            int16_t s_tmp = (int16_t)(int_data[i] >> k);
            uint8_t c_tmp = (uint8_t)(k * 11);
            
            float f_val = compute_float(float_data[i], (float)s_tmp, c_tmp);
            double d_val = compute_double(double_data[i], s_tmp, c_tmp);
            
            /* Chain of dependent operations */
            int32_t chain1 = s_tmp * 3 + c_tmp;
            int32_t chain2 = compute_int(chain1, k, i);
            int32_t chain3 = chain2 - compute_int(c_tmp, s_tmp, k);
            
            /* Use volatile to prevent optimization */
            volatile int32_t sink = chain3;
            acc_int += CLAMP(sink, -1000, 1000);
            
            acc_float += f_val;
            acc_double += d_val;
        }
        
        /* Periodic register pressure spike */
        if (i % 8 == 0) {
            /* Complex expression with many temporaries */
            int32_t a = int_data[(i + 1) % size];
            int32_t b = int_data[(i + 2) % size];
            int32_t c = int_data[(i + 3) % size];
            
            float fa = float_data[(i + 1) % size];
            float fb = float_data[(i + 2) % size];
            
            double da = double_data[(i + 1) % size];
            double db = double_data[(i + 2) % size];
            
            /* Many intermediate values that need registers */
            int32_t t1 = compute_int(a, b, 1);
            int32_t t2 = compute_int(b, c, 2);
            int32_t t3 = compute_int(c, a, 3);
            int32_t t4 = t1 + t2 - t3;
            
            float ft1 = compute_float(fa, fb, t1);
            float ft2 = compute_float(fb, fa, t2);
            float ft3 = ft1 * 0.3f - ft2 * 0.7f;
            
            double dt1 = compute_double(da, (int16_t)t1, (uint8_t)t2);
            double dt2 = compute_double(db, (int16_t)t3, (uint8_t)t4);
            double dt3 = dt1 * 0.4 + dt2 * 0.6;
            
            /* Force all values to be used */
            volatile int32_t vs1 = t4;
            volatile float vs2 = ft3;
            volatile double vs3 = dt3;
            
            acc_int += vs1;
            acc_float += vs2;
            acc_double += vs3;
        }
    }
    
    /* Final sink to prevent elimination */
    volatile int32_t final_int = acc_int;
    volatile float final_float = acc_float;
    volatile double final_double = acc_double;
    FORCE_USE(final_int);
    FORCE_USE(final_float);
    FORCE_USE(final_double);
}

/* Main driver with initialization and multiple iterations */
int main(void) {
    const size_t SIZE = 256;
    
    /* Allocate and initialize arrays with different patterns */
    int32_t* int_data = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float* float_data = (float*)malloc(SIZE * sizeof(float));
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        return 1;
    }
    
    /* Initialize with varying patterns to prevent optimization */
    for (size_t i = 0; i < SIZE; i++) {
        int_data[i] = (int32_t)(i * 1103515245 + 12345);
        float_data[i] = (float)(i * 0.12345f + 1.2345f);
        double_data[i] = (double)(i * 0.6789 + 5.4321);
    }
    
    /* Multiple iterations to increase optimization opportunities */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the processing to create different register pressure patterns */
        process_block(int_data, float_data, double_data, SIZE, iter);
        
        /* Occasionally modify data to prevent complete optimization */
        if (iter % 10 == 0) {
            for (size_t i = 0; i < SIZE; i += 7) {
                int_data[i] ^= iter;
                float_data[i] += (float)iter * 0.01f;
                double_data[i] -= (double)iter * 0.001;
            }
        }
        
        /* Force register clobbering between iterations */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", 
                     "r4", "r5", "r6", "r7", "r8", "r9", "r10");
    }
    
    /* Final computation to use all data */
    int32_t total_int = 0;
    float total_float = 0.0f;
    double total_double = 0.0;
    
    for (size_t i = 0; i < SIZE; i++) {
        total_int += compute_int(int_data[i], i, total_int & 0xFF);
        total_float += compute_float(float_data[i], total_float, i & 0xF);
        total_double += compute_double(double_data[i], (int16_t)i, (uint8_t)total_int);
    }
    
    volatile int32_t output_int = total_int;
    volatile float output_float = total_float;
    volatile double output_double = total_double;
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return (output_int > 0) ? 0 : 1;
}
