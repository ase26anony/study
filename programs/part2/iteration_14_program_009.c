/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Helper functions to force copy propagation contexts */
static inline int32_t process_int(int32_t a, int32_t b, int32_t c) {
    int32_t t1 = a + b;
    int32_t t2 = t1 * c;
    int32_t t3 = t2 >> 3;
    FORCE_COPY(t3);
    return t3 - a;
}

static inline float process_float(float a, float b, float c) {
    float t1 = a * b;
    float t2 = t1 + c;
    float t3 = t2 * 0.5f;
    FORCE_COPY(t3);
    return t3 - a;
}

static inline int16_t narrow_copy(int32_t val) {
    int16_t narrow = (int16_t)CLAMP(val, -32768, 32767);
    FORCE_COPY(narrow);
    return narrow;
}

static inline double promote_and_compute(int16_t s, float f) {
    double d1 = (double)s;
    double d2 = (double)f;
    double result = d1 * d2 + 1.41421356;
    FORCE_COPY(result);
    return result;
}

/* Main computation kernel */
void compute_kernel(int32_t *int_data, float *float_data, 
                    int16_t *short_data, double *double_data,
                    int size) {
    volatile int outer_counter = 0;
    
    for (int i = 0; i < size; i++) {
        /* Create register pressure with many live values */
        int32_t base = int_data[i];
        float fbase = float_data[i];
        int16_t sbase = short_data[i];
        
        /* Multiple dependent computations creating virtual registers */
        for (int j = 0; j < 8; j++) {
            /* Integer chain with mixed operations */
            int32_t v1 = base + j * 17;
            int32_t v2 = v1 ^ 0x55AA55AA;
            int32_t v3 = v2 * 3;
            int32_t v4 = v3 >> (j & 3);
            int32_t v5 = process_int(v4, v1, v2);
            
            /* Floating-point chain */
            float f1 = fbase + j * 0.125f;
            float f2 = f1 * 3.14159f;
            float f3 = process_float(f2, f1, fbase);
            float f4 = f3 * f3 - f2;
            
            /* Type conversions and promotions */
            int16_t narrow1 = narrow_copy(v5);
            int16_t narrow2 = narrow_copy(v4);
            double promoted = promote_and_compute(narrow1, f4);
            
            /* More computations with different modes */
            int64_t wide1 = (int64_t)v5 * (int64_t)narrow1;
            int64_t wide2 = wide1 + (int64_t)(j * 1000);
            
            /* Conditional block creating control flow complexity */
            if ((i ^ j) & 1) {
                float f5 = f4 * 2.0f;
                int32_t v6 = v5 + narrow1;
                double d3 = promoted * 0.70710678;
                
                /* Force register moves with inline asm */
                asm volatile("" : : "r"(v6), "r"(narrow1), "x"(f5), "x"(d3));
                
                VOLATILE_SINK(f5);
                VOLATILE_SINK(d3);
            } else {
                int32_t v7 = v5 - narrow2;
                float f6 = f4 * 0.5f;
                
                asm volatile("" : : "r"(v7), "r"(narrow2), "x"(f6));
                
                VOLATILE_SINK(v7);
                VOLATILE_SINK(f6);
            }
            
            /* Store results creating more register pressure */
            double_data[i * 8 + j] = promoted;
            short_data[i] = narrow1;
            
            /* Volatile access to prevent optimization */
            outer_counter++;
            
            /* Another inner loop for additional pressure */
            for (int k = 0; k < 2; k++) {
                int32_t tmp = v5 + k;
                float ftmp = f4 + k * 0.25f;
                double dtmp = promoted + k * 0.1;
                
                /* Force copies between different register classes */
                int32_t tmp_copy = tmp;
                float ftmp_copy = ftmp;
                FORCE_COPY(tmp_copy);
                FORCE_COPY(ftmp_copy);
                
                /* Mixed-mode computation */
                double mixed = (double)tmp_copy + (double)ftmp_copy + dtmp;
                VOLATILE_SINK(mixed);
            }
        }
        
        /* Additional computation between outer loop iterations */
        if (i > 0) {
            int32_t prev = int_data[i-1];
            float fprev = float_data[i-1];
            int32_t combined = process_int(base, prev, i);
            float fcombined = process_float(fbase, fprev, i * 0.01f);
            
            /* Force spilling with many live values */
            asm volatile("" : : "r"(combined), "r"(base), "r"(prev),
                         "x"(fcombined), "x"(fbase), "x"(fprev));
        }
    }
}

/* Initialize with patterns that create varied computation paths */
void init_data(int32_t *int_data, float *float_data,
               int16_t *short_data, double *double_data, int size) {
    for (int i = 0; i < size; i++) {
        int_data[i] = (i * 37) & 0xFFF;
        float_data[i] = (i * 0.073f) - 0.5f;
        short_data[i] = (i * 59) & 0x7FFF;
        double_data[i] = i * 0.0017;
    }
}

int main(void) {
    const int SIZE = 128;
    const int TOTAL_SIZE = SIZE * 8;
    
    /* Allocate aligned data to avoid unnecessary memory ops */
    int32_t *int_data = __builtin_aligned_alloc(64, SIZE * sizeof(int32_t));
    float *float_data = __builtin_aligned_alloc(64, SIZE * sizeof(float));
    int16_t *short_data = __builtin_aligned_alloc(64, SIZE * sizeof(int16_t));
    double *double_data = __builtin_aligned_alloc(64, TOTAL_SIZE * sizeof(double));
    
    if (!int_data || !float_data || !short_data || !double_data) {
        return 1;
    }
    
    init_data(int_data, float_data, short_data, double_data, SIZE);
    
    /* Multiple passes to increase optimization opportunities */
    for (int pass = 0; pass < 3; pass++) {
        compute_kernel(int_data, float_data, short_data, double_data, SIZE);
        
        /* Modify data slightly between passes */
        for (int i = 0; i < SIZE; i++) {
            int_data[i] += pass * 11;
            float_data[i] += pass * 0.011f;
        }
    }
    
    /* Consume final results */
    volatile double sum = 0.0;
    for (int i = 0; i < TOTAL_SIZE; i++) {
        sum += double_data[i];
    }
    VOLATILE_SINK(sum);
    
    free(int_data);
    free(float_data);
    free(short_data);
    free(double_data);
    
    return 0;
}
