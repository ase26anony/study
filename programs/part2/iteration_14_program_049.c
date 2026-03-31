/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent elimination */
static volatile int volatile_sink_int;
static volatile double volatile_sink_double;

/* Helper functions to force copies and parameter passing */
static inline int32_t compute_int(int32_t a, int32_t b, int32_t c) {
    /* Force register copies through parameter passing */
    asm volatile("" : : "r"(a), "r"(b), "r"(c));
    return (a * b) + (c >> 3);
}

static inline float compute_float(float a, float b, int c) {
    /* Mixed-type computation */
    asm volatile("" : : "r"(a), "r"(b), "r"(c));
    return a * b + (float)c * 0.5f;
}

static inline double compute_double(double a, double b, int64_t c) {
    /* Force 64-bit operations */
    asm volatile("" : : "r"(a), "r"(b), "r"(c));
    return a / (b + 1.0) + (double)c * 0.25;
}

/* Function with complex control flow to create many basic blocks */
static void process_block(int8_t *data8, int16_t *data16, int32_t *data32,
                          float *fdata, double *ddata, int size) {
    int i, j, k;
    
    /* Outer loop creates register pressure */
    for (i = 0; i < size; i++) {
        int32_t acc_int = 0;
        float acc_float = 0.0f;
        double acc_double = 0.0;
        
        /* First inner loop - integer operations */
        for (j = 0; j < 8; j++) {
            /* Create dependent chain of integer computations */
            int32_t tmp1 = data32[i] + j * 17;
            int32_t tmp2 = data16[i * 2] - j * 3;
            int32_t tmp3 = (int32_t)data8[i * 4] * j;
            
            /* Force copies through helper function */
            int32_t combined = compute_int(tmp1, tmp2, tmp3);
            
            /* Conditional to create control flow */
            if (combined & 1) {
                acc_int += combined * 2;
            } else {
                acc_int -= combined / 2;
            }
            
            /* Volatile write to prevent elimination */
            volatile_sink_int = acc_int;
        }
        
        /* Second inner loop - mixed integer/float */
        for (k = 0; k < 4; k++) {
            /* Mixed precision calculations */
            float f1 = fdata[i] + k * 0.125f;
            float f2 = fdata[size - i - 1] - k * 0.0625f;
            
            /* Integer to float conversion */
            int32_t int_val = data32[i] + k * 7;
            float f3 = compute_float(f1, f2, int_val);
            
            /* Double precision operations */
            double d1 = ddata[i] * (k + 1);
            int64_t long_val = (int64_t)acc_int * k;
            double d2 = compute_double(d1, f3, long_val);
            
            /* Complex conditional with different modes */
            if ((i + k) % 3 == 0) {
                acc_float += f3 * 1.5f;
                acc_double = d2 * 0.8;
            } else if ((i + k) % 3 == 1) {
                acc_float = f3 - acc_float;
                acc_double += d2 / 1.3;
            } else {
                acc_float *= 0.9f;
                acc_double = compute_double(acc_double, d2, long_val >> 2);
            }
            
            /* More volatile sinks */
            volatile_sink_double = acc_double;
            
            /* Inline asm that clobbers registers */
            asm volatile("" : : : "memory", "r0", "r1", "r2", "r3",
                         "r4", "r5", "r6", "r7", "r8", "r9", "r10");
        }
        
        /* Store results back with type conversions */
        if (i % 2 == 0) {
            fdata[i] = acc_float + (float)acc_int * 0.01f;
            ddata[i] = acc_double + (double)acc_int * 0.001;
        } else {
            data32[i] = acc_int + (int32_t)acc_float + (int32_t)acc_double;
        }
    }
}

/* Another function with different pattern to increase pressure */
static void alternate_path(short *sdata, int *idata, float *fdata, int len) {
    int i;
    char temp_chars[16];
    
    for (i = 0; i < len; i++) {
        /* Create many short-lived values */
        short s1 = sdata[i];
        short s2 = sdata[(i + 1) % len];
        short s3 = sdata[(i + 2) % len];
        
        int t1 = s1 * 3;
        int t2 = s2 * 5;
        int t3 = s3 * 7;
        
        /* Chain of dependent computations */
        int r1 = t1 + (t2 >> 1);
        int r2 = t2 - (t3 << 1);
        int r3 = compute_int(r1, r2, t3);
        
        float f1 = fdata[i];
        float f2 = fdata[(i + 3) % len];
        float f3 = compute_float(f1, f2, r3);
        
        /* Use alloca to force stack activity */
        for (int j = 0; j < 16; j++) {
            temp_chars[j] = (char)((r3 + j) & 0xFF);
        }
        
        /* Complex conditional with mode mixing */
        if (r3 > 1000) {
            idata[i] = r3 + (int)f3;
            fdata[i] = f3 * 2.0f;
        } else if (r3 < -1000) {
            idata[i] = r3 - (int)(f3 * 10.0f);
            fdata[i] = f3 / 2.0f;
        } else {
            /* Force more register copies */
            int tmp = r3 * 2;
            float ftmp = f3 * 3.0f;
            idata[i] = compute_int(tmp, idata[i], (int)ftmp);
            fdata[i] = compute_float(ftmp, fdata[i], tmp);
        }
        
        /* Periodic volatile access */
        if (i % 7 == 0) {
            volatile_sink_int = idata[i];
        }
    }
}

int main(void) {
    const int SIZE = 256;
    
    /* Allocate arrays with different types and alignments */
    int8_t *data8 = (int8_t*)malloc(SIZE * 4 * sizeof(int8_t));
    int16_t *data16 = (int16_t*)malloc(SIZE * 2 * sizeof(int16_t));
    int32_t *data32 = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float *fdata = (float*)malloc(SIZE * sizeof(float));
    double *ddata = (double*)malloc(SIZE * sizeof(double));
    short *sdata = (short*)malloc(SIZE * sizeof(short));
    int *idata = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with varied patterns */
    for (int i = 0; i < SIZE; i++) {
        data8[i * 4] = (int8_t)(i * 3);
        data16[i * 2] = (int16_t)(i * 5 - 128);
        data32[i] = i * 7 - 512;
        fdata[i] = (float)i * 0.123f - 15.0f;
        ddata[i] = (double)i * 0.456 - 30.0;
        sdata[i] = (short)(i * 11);
        idata[i] = i * 13 - 256;
    }
    
    /* Multiple passes to increase optimization opportunities */
    for (int pass = 0; pass < 3; pass++) {
        /* Call both functions to create interprocedural pressure */
        process_block(data8, data16, data32, fdata, ddata, SIZE);
        alternate_path(sdata, idata, fdata, SIZE);
        
        /* Additional computations between calls */
        for (int i = 0; i < SIZE / 2; i++) {
            /* More mixed-mode operations */
            double d = ddata[i] * 1.1;
            float f = fdata[SIZE - i - 1] * 0.9f;
            int32_t i1 = data32[i];
            int32_t i2 = data32[SIZE - i - 1];
            
            /* Force register copies with mode changes */
            int64_t long_result = (int64_t)i1 * i2;
            float float_result = (float)d + f;
            double double_result = (double)float_result * d;
            
            /* Use inline asm to prevent optimization */
            asm volatile("" : : "r"(long_result), "r"(float_result), 
                         "r"(double_result));
            
            /* Conditional store with type conversion */
            if ((i + pass) % 4 == 0) {
                data32[i] = (int32_t)(long_result & 0xFFFFFFFF);
                fdata[i] = float_result;
                ddata[i] = double_result;
            }
        }
    }
    
    /* Final sink to prevent elimination of entire computation */
    volatile_sink_int = data32[0] + idata[0];
    volatile_sink_double = ddata[0] + fdata[0];
    
    /* Cleanup */
    free(data8);
    free(data16);
    free(data32);
    free(fdata);
    free(ddata);
    free(sdata);
    free(idata);
    
    return 0;
}
