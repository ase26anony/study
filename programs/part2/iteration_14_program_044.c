/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -fno-omit-frame-pointer */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_USE(x) asm volatile("" : : "r"(x))
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Helper functions to force copies and virtual register creation */
static inline int32_t compute_int(int32_t a, int32_t b, int32_t c) {
    volatile int32_t v1 = a + b;  /* Force spill/reload */
    int32_t r = (v1 * c) ^ (a - b);
    FORCE_USE(r);
    return r;
}

static inline float compute_float(float a, float b, float c) {
    volatile float v1 = a * b;    /* Force spill/reload */
    float r = (v1 + c) * (a - b);
    FORCE_USE(r);
    return r;
}

static inline double compute_double(double a, double b, int32_t scale) {
    volatile double v1 = a / (b + 1.0);  /* Force spill/reload */
    double r = v1 * scale + a - b;
    FORCE_USE(r);
    return r;
}

/* Function with complex control flow to create many basic blocks */
static void process_block(int32_t* int_data, float* float_data, 
                         double* double_data, size_t size, int iter) {
    /* Multiple local variables to create register pressure */
    int32_t t1, t2, t3, t4, t5;
    float f1, f2, f3, f4;
    double d1, d2, d3;
    
    /* Outer loop with multiple induction variables */
    for (size_t i = 0; i < size; i++) {
        /* First basic block: integer computations */
        t1 = int_data[i] ^ iter;
        t2 = t1 * 1103515245 + 12345;
        t3 = compute_int(t1, t2, iter);
        
        /* Conditional block to split control flow */
        if (t3 & 0x100) {
            /* Branch 1: more integer ops */
            t4 = (t2 >> 16) & 0x7FFF;
            t5 = compute_int(t3, t4, i);
            
            /* Mixed integer/float conversions */
            f1 = (float)t5 * 0.5f;
            f2 = float_data[i] + f1;
            
            /* Force register pressure with inline asm */
            asm volatile("" : "+r"(t4), "+r"(t5) : "r"(f1), "r"(f2));
        } else {
            /* Branch 2: floating point computations */
            f3 = float_data[i] * 1.5f;
            f4 = compute_float(f3, (float)iter, (float)i);
            
            /* Convert to integer and back */
            t4 = (int32_t)f4;
            t5 = compute_int(t4, iter, i);
            
            /* Clobber registers to force moves */
            asm volatile("" : : "r"(f3), "r"(f4), "r"(t4), "r"(t5));
        }
        
        /* Merge point: use values from both branches */
        int32_t merged = t3 + t5;
        
        /* Inner loop for additional pressure */
        for (int j = 0; j < 3; j++) {
            /* Double precision computations */
            d1 = double_data[i] * (j + 1);
            d2 = compute_double(d1, (double)merged, j);
            
            /* More integer ops dependent on doubles */
            int32_t tmp = (int32_t)d2;
            tmp = compute_int(tmp, j, iter);
            
            /* Force use to prevent elimination */
            volatile int32_t sink = tmp;
            (void)sink;
            
            /* Mode mixing: char/short/int */
            char c1 = (char)(tmp & 0xFF);
            short s1 = (short)(tmp >> 8);
            int32_t combined = (int32_t)c1 * (int32_t)s1;
            
            /* Another volatile to force spills */
            volatile short vs = s1;
            (void)vs;
            
            /* Update array with mixed computation */
            int_data[i] = CLAMP(combined, -1000, 1000);
        }
        
        /* Final floating point update */
        float_data[i] = compute_float(float_data[i], (float)merged, 0.1f);
        
        /* Double precision update with mode mixing */
        double_data[i] = compute_double(double_data[i], (double)merged, 2);
    }
}

/* Another layer to increase complexity */
static void process_all(int32_t* int_data, float* float_data,
                       double* double_data, size_t size, int rounds) {
    for (int r = 0; r < rounds; r++) {
        /* Create local copies to force register moves */
        int32_t local_ints[4];
        float local_floats[4];
        
        for (int k = 0; k < 4; k++) {
            local_ints[k] = int_data[k % size];
            local_floats[k] = float_data[k % size];
            
            /* Force computations with local copies */
            int32_t tmp = compute_int(local_ints[k], r, k);
            float ftmp = compute_float(local_floats[k], (float)r, (float)k);
            
            /* Cross-type computations */
            double dtmp = (double)tmp * (double)ftmp;
            dtmp = compute_double(dtmp, (double)k, r);
            
            /* Write back through volatile to force stores */
            volatile double vdtmp = dtmp;
            (void)vdtmp;
        }
        
        /* Call the main processing function */
        process_block(int_data, float_data, double_data, size, r);
    }
}

int main(void) {
    const size_t SIZE = 128;
    const int ROUNDS = 10;
    
    /* Allocate and initialize arrays with different patterns */
    int32_t* int_data = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float* float_data = (float*)malloc(SIZE * sizeof(float));
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        return 1;
    }
    
    /* Initialize with different data patterns */
    for (size_t i = 0; i < SIZE; i++) {
        int_data[i] = (int32_t)(i * 1103515245u) ^ 0xDEADBEEF;
        float_data[i] = (float)i * 0.12345f + 1.0f;
        double_data[i] = (double)i * 0.6789 + 2.0;
    }
    
    /* Main processing with nested loops */
    for (int outer = 0; outer < 3; outer++) {
        /* Create additional register pressure with local variables */
        int32_t accum_int = 0;
        float accum_float = 0.0f;
        double accum_double = 0.0;
        
        for (int inner = 0; inner < 5; inner++) {
            /* Process data */
            process_all(int_data, float_data, double_data, SIZE, ROUNDS);
            
            /* Accumulate results using different types */
            for (size_t i = 0; i < SIZE; i += 8) {
                accum_int += int_data[i];
                accum_float += float_data[i];
                accum_double += double_data[i];
                
                /* Force cross-type conversions */
                int32_t from_float = (int32_t)float_data[i];
                float from_int = (float)int_data[i];
                
                /* Use in computations that create copies */
                int32_t mixed = compute_int(from_float, accum_int, inner);
                float mixed_f = compute_float(from_int, accum_float, 0.5f);
                
                /* Force register clobbering */
                asm volatile("" : : "r"(mixed), "r"(mixed_f), 
                             "r"(accum_int), "r"(accum_float), "r"(accum_double));
            }
        }
        
        /* Final volatile sink to prevent elimination */
        volatile int32_t sink_int = accum_int;
        volatile float sink_float = accum_float;
        volatile double sink_double = accum_double;
        (void)sink_int; (void)sink_float; (void)sink_double;
    }
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
