/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -fno-omit-frame-pointer -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7")

/* Helper functions to force copy propagation contexts */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    int32_t t1 = a + b;
    int32_t t2 = t1 ^ c;
    FORCE_COPY(t2);
    return t2 * 3;
}

static inline float float_copy_mul(float a, float b, float c) {
    float t1 = a * b;
    float t2 = t1 + c;
    asm volatile("" : "+f"(t2));
    return t2 * 2.0f;
}

static inline int16_t narrow_compute(int32_t a, int32_t b) {
    int16_t s1 = (int16_t)(a & 0xFFFF);
    int16_t s2 = (int16_t)(b & 0xFFFF);
    FORCE_COPY(s1);
    FORCE_COPY(s2);
    return (int16_t)(s1 + s2);
}

/* Complex kernel with mixed operations */
static void compute_kernel(int32_t *int_data, float *float_data, 
                          int8_t *char_data, double *double_data,
                          int size) {
    volatile int outer_sink = 0;
    
    for (int i = 0; i < size; i++) {
        /* Create register pressure with many live values */
        int32_t base = int_data[i];
        float fbase = float_data[i];
        int8_t cbase = char_data[i];
        double dbase = double_data[i];
        
        /* Force copies between different scopes */
        {
            int32_t tmp1 = base * 2;
            float ftmp1 = fbase * 3.14f;
            FORCE_COPY(tmp1);
            FORCE_COPY(ftmp1);
            
            /* Mixed-type computation chain */
            for (int j = 0; j < 4; j++) {
                /* Create recomputable values */
                int32_t recompute1 = tmp1 + j * 7;
                float recompute2 = ftmp1 + j * 1.5f;
                
                /* Use helper functions to force copy contexts */
                int32_t copied1 = copy_and_add(recompute1, j, i);
                float copied2 = float_copy_mul(recompute2, 1.1f, (float)j);
                
                /* Narrowing conversions */
                int16_t narrow1 = narrow_compute(copied1, i);
                int16_t narrow2 = narrow_compute(j, copied1);
                
                /* More register pressure with different modes */
                double d1 = (double)copied2 * 2.0;
                double d2 = dbase + (double)narrow1 * 0.01;
                double d3 = d1 * d2;
                
                /* Conditional block to split control flow */
                if ((i ^ j) & 1) {
                    int32_t cond_val = copied1 * 3;
                    float cond_float = copied2 * 1.5f;
                    FORCE_COPY(cond_val);
                    FORCE_COPY(cond_float);
                    
                    /* More copies in conditional path */
                    cond_val = copy_and_add(cond_val, narrow1, narrow2);
                    cond_float = float_copy_mul(cond_float, 2.0f, (float)cond_val);
                    
                    VOLATILE_SINK(cond_val);
                    VOLATILE_SINK(cond_float);
                } else {
                    /* Alternative path with different operations */
                    double alt_d = d3 * 0.5;
                    int32_t alt_int = (int32_t)alt_d;
                    FORCE_COPY(alt_d);
                    FORCE_COPY(alt_int);
                    
                    /* Chain of dependent operations */
                    for (int k = 0; k < 2; k++) {
                        alt_int += k * 11;
                        alt_d += (double)alt_int * 0.001;
                        CLASH_REGISTERS;  /* Force register shuffling */
                    }
                    
                    VOLATILE_SINK(alt_d);
                    VOLATILE_SINK(alt_int);
                }
                
                /* Consume all values to prevent elimination */
                VOLATILE_SINK(recompute1);
                VOLATILE_SINK(recompute2);
                VOLATILE_SINK(copied1);
                VOLATILE_SINK(copied2);
                VOLATILE_SINK(narrow1);
                VOLATILE_SINK(narrow2);
                VOLATILE_SINK(d1);
                VOLATILE_SINK(d2);
                VOLATILE_SINK(d3);
            }
        }
        
        /* Outer loop computations with different modes */
        {
            int64_t wide1 = (int64_t)base * 1000000LL;
            int64_t wide2 = wide1 + (int64_t)i * 77777LL;
            FORCE_COPY(wide1);
            FORCE_COPY(wide2);
            
            /* Mix with floating point */
            float fwide = (float)wide2 * 0.0001f;
            double dwide = (double)wide1 * 1e-6;
            
            /* More copy contexts */
            int32_t final1 = copy_and_add((int32_t)wide1, (int32_t)wide2, i);
            float final2 = float_copy_mul(fwide, 2.0f, (float)dwide);
            
            outer_sink += final1;
            outer_sink += (int)final2;
        }
    }
    
    VOLATILE_SINK(outer_sink);
}

/* Initialize with patterns that create varied computations */
static void init_data(int32_t *int_data, float *float_data,
                     int8_t *char_data, double *double_data, int size) {
    for (int i = 0; i < size; i++) {
        int_data[i] = (i * 37) & 0xFFF;
        float_data[i] = (float)(i * 19) * 0.123f;
        char_data[i] = (int8_t)((i * 53) & 0x7F);
        double_data[i] = (double)(i * 71) * 0.456;
    }
}

int main(void) {
    const int SIZE = 128;
    
    /* Allocate aligned to avoid unnecessary spills */
    int32_t *int_data = __builtin_aligned_alloc(64, SIZE * sizeof(int32_t));
    float *float_data = __builtin_aligned_alloc(64, SIZE * sizeof(float));
    int8_t *char_data = __builtin_aligned_alloc(64, SIZE * sizeof(int8_t));
    double *double_data = __builtin_aligned_alloc(64, SIZE * sizeof(double));
    
    if (!int_data || !float_data || !char_data || !double_data) {
        return 1;
    }
    
    init_data(int_data, float_data, char_data, double_data, SIZE);
    
    /* Multiple iterations to increase optimization opportunities */
    for (int iter = 0; iter < 10; iter++) {
        compute_kernel(int_data, float_data, char_data, double_data, SIZE);
        
        /* Slightly modify data each iteration */
        for (int i = 0; i < SIZE; i++) {
            int_data[i] += iter;
            float_data[i] += (float)iter * 0.1f;
        }
    }
    
    free(int_data);
    free(float_data);
    free(char_data);
    free(double_data);
    
    return 0;
}
