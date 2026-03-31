/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_USE(x) asm volatile("" : : "r"(x))
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Helper functions to force copies and register pressure */
static inline int32_t compute_int(int32_t a, int32_t b, int32_t c) {
    volatile int32_t v1 = a + b;  /* Force memory access */
    int32_t r = (v1 * c) ^ (a - b);
    FORCE_USE(r);
    return r;
}

static inline float compute_float(float a, float b, float c) {
    volatile float v1 = a + b;  /* Force memory access */
    float r = (v1 * c) / (a - b + 1.0f);
    FORCE_USE(r);
    return r;
}

static inline double compute_double(double a, double b, double c) {
    volatile double v1 = a * b;  /* Force memory access */
    double r = (v1 + c) / (a - b);
    FORCE_USE(r);
    return r;
}

/* Function with mixed operations to create various register modes */
static void process_block(int8_t* data8, int16_t* data16, int32_t* data32,
                         float* fdata, double* ddata, size_t n) {
    int32_t acc_int = 0;
    float acc_float = 0.0f;
    double acc_double = 0.0;
    
    /* Outer loop creates control flow complexity */
    for (size_t i = 0; i < n; i++) {
        /* First inner loop with integer operations */
        for (size_t j = 0; j < 8; j++) {
            /* Chain of dependent operations */
            int32_t t1 = data8[i] * 3;
            int32_t t2 = data16[i] + t1;
            int32_t t3 = data32[i] ^ t2;
            
            /* Force copy propagation context */
            int32_t t4 = compute_int(t1, t2, t3);
            int32_t t5 = compute_int(t2, t3, t4);
            
            /* Mixed precision calculations */
            float f1 = (float)t4 * 1.5f;
            float f2 = (float)t5 * 2.5f;
            float f3 = compute_float(f1, f2, acc_float);
            
            /* More copies with different types */
            double d1 = (double)f3 * 0.75;
            double d2 = (double)acc_float * 1.25;
            double d3 = compute_double(d1, d2, acc_double);
            
            /* Register clobbering inline asm */
            asm volatile("" : : "r"(t3), "r"(t4), "r"(t5),
                         "x"(f1), "x"(f2), "x"(f3),
                         "x"(d1), "x"(d2), "x"(d3));
            
            /* Accumulate with conditional */
            acc_int += (t3 > 0) ? t4 : t5;
            acc_float += (f1 > f2) ? f3 : f1;
            acc_double += (d1 > d2) ? d3 : d1;
        }
        
        /* Second inner loop with different operations */
        for (size_t k = 0; k < 4; k++) {
            /* More virtual register pressure */
            int16_t s1 = data16[i] << k;
            int32_t s2 = s1 * data32[i];
            int32_t s3 = compute_int(s1, s2, acc_int);
            
            /* Floating point conversions */
            float fs1 = fdata[i] * (float)k;
            float fs2 = compute_float(fs1, acc_float, fdata[i]);
            
            /* Double precision operations */
            double ds1 = ddata[i] / (k + 1);
            double ds2 = compute_double(ds1, acc_double, ddata[i]);
            
            /* Force register moves */
            volatile int32_t vs1 = s3;
            volatile float vfs1 = fs2;
            volatile double vds1 = ds2;
            
            /* Conditional updates */
            if ((i + k) % 3 == 0) {
                acc_int ^= vs1;
                acc_float = vfs1 * 0.9f;
                acc_double = vds1 * 0.8;
            }
        }
        
        /* Conditional block creating control flow edges */
        if (i % 5 == 0) {
            /* Different operation mix */
            int32_t tmp1 = data32[i] * 7;
            float tmp2 = fdata[i] * 3.14f;
            double tmp3 = ddata[i] * 2.71828;
            
            /* Force copies between different scopes */
            {
                int32_t local1 = compute_int(tmp1, acc_int, i);
                float local2 = compute_float(tmp2, acc_float, i);
                double local3 = compute_double(tmp3, acc_double, i);
                
                /* More inline asm clobbering */
                asm volatile("" : : "r"(local1), "x"(local2), "x"(local3));
                
                acc_int = CLAMP(local1, -1000, 1000);
                acc_float = local2;
                acc_double = local3;
            }
        }
    }
    
    /* Final volatile sink */
    volatile int32_t final_int = acc_int;
    volatile float final_float = acc_float;
    volatile double final_double = acc_double;
    FORCE_USE(final_int);
    FORCE_USE(final_float);
    FORCE_USE(final_double);
}

int main(void) {
    const size_t N = 256;
    
    /* Allocate and initialize arrays with different patterns */
    int8_t* data8 = (int8_t*)malloc(N * sizeof(int8_t));
    int16_t* data16 = (int16_t*)malloc(N * sizeof(int16_t));
    int32_t* data32 = (int32_t*)malloc(N * sizeof(int32_t));
    float* fdata = (float*)malloc(N * sizeof(float));
    double* ddata = (double*)malloc(N * sizeof(double));
    
    /* Initialize with varying patterns */
    for (size_t i = 0; i < N; i++) {
        data8[i] = (int8_t)((i * 13) % 256 - 128);
        data16[i] = (int16_t)((i * 17) % 65536 - 32768);
        data32[i] = (int32_t)(i * 23 - 1000);
        fdata[i] = (float)(i * 0.1f - 12.5f);
        ddata[i] = (double)(i * 0.05 - 25.0);
    }
    
    /* Process multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 3; iter++) {
        process_block(data8, data16, data32, fdata, ddata, N);
        
        /* Modify data slightly between iterations */
        for (size_t i = 0; i < N; i++) {
            data8[i] ^= 0x55;
            data16[i] += iter;
            data32[i] = (data32[i] * 3) / 2;
            fdata[i] *= 1.1f;
            ddata[i] *= 0.9;
        }
    }
    
    free(data8);
    free(data16);
    free(data32);
    free(fdata);
    free(ddata);
    
    return 0;
}
