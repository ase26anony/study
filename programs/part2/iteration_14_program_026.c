/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force copies and prevent optimization */
static inline int32_t copy_and_add(int32_t a, int32_t b, volatile int32_t *sink) {
    int32_t result = a + b;
    *sink = result;  /* Volatile write to prevent elimination */
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

static inline float promote_and_multiply(int16_t s, float f, volatile float *sink) {
    float result = (float)(int32_t)s * f;  /* Promotion and conversion */
    *sink = result;
    asm volatile("" : "+f"(result) : : "memory");
    return result;
}

static inline double mixed_calc(int8_t c, int32_t i, double d, volatile double *sink) {
    double temp1 = (double)c * 1.5;
    double temp2 = (double)i * d;
    double result = temp1 + temp2;
    *sink = result;
    /* Force register moves with clobber */
    asm volatile("" : "+r"(i), "+f"(result) : : "cc", "memory");
    return result;
}

/* Function with complex control flow to create many basic blocks */
static void process_data(int8_t *data1, int16_t *data2, int32_t *data3,
                         float *data4, double *data5, size_t size) {
    volatile int32_t vsink_int __attribute__((unused));
    volatile float vsink_float __attribute__((unused));
    volatile double vsink_double __attribute__((unused));
    
    /* Outer loop creates register pressure */
    for (size_t outer = 0; outer < 3; ++outer) {
        int32_t accum_int = outer * 100;
        float accum_float = (float)outer * 0.1f;
        double accum_double = (double)outer * 0.01;
        
        /* First inner loop with integer operations */
        for (size_t i = 0; i < size; ++i) {
            /* Create dependent chain of integer computations */
            int32_t val1 = data1[i] * 2;
            int32_t val2 = data2[i] + val1;
            int32_t val3 = data3[i] - val2;
            
            /* Force copy propagation context */
            int32_t copied = copy_and_add(val1, val2, &vsink_int);
            accum_int += copied + val3;
            
            /* Conditional block creates control flow complexity */
            if (i % 2 == 0) {
                int32_t temp = accum_int * 3;
                accum_int = temp / 2;
                asm volatile("" : : "r"(temp) : "memory");
            } else {
                accum_int = accum_int >> 1;
            }
            
            /* Mix in floating point to create different modes */
            if (i % 3 == 0) {
                float fval = promote_and_multiply(data2[i], data4[i], &vsink_float);
                accum_float += fval * 0.5f;
            }
        }
        
        /* Second inner loop with mixed precision */
        for (size_t i = 0; i < size; ++i) {
            /* Create recomputable values that are cheap to rematerialize */
            int8_t base = data1[i];
            int32_t expanded = (int32_t)base * 17;  /* Cheap to recompute */
            
            /* Multiple uses of same recomputable value */
            for (int j = 0; j < 2; ++j) {
                int32_t use1 = expanded + j;
                int32_t use2 = expanded - j;
                accum_int += use1 * use2;
                
                /* Force virtual register creation with inline asm */
                asm volatile("" : "+r"(use1), "+r"(use2) : : "cc");
            }
            
            /* Floating point chain with conversions */
            double dval = mixed_calc(data1[i], data3[i], data5[i], &vsink_double);
            accum_double += dval;
            
            /* Another conditional with different register types */
            if (i % 4 == 0) {
                float temp_f = (float)accum_double;
                accum_float = accum_float * 0.9f + temp_f;
                asm volatile("" : "+f"(temp_f) : : "memory");
            }
        }
        
        /* Third loop with nested conditionals */
        for (size_t i = 0; i < size; i += 2) {
            int32_t idx = i;
            while (idx < size && idx < i + 4) {
                /* Complex expression with many intermediate values */
                int32_t a = data1[idx] * 3;
                int32_t b = data2[idx] / 2;
                int32_t c = data3[idx] + a - b;
                
                /* Multiple dependent computations */
                for (int k = 0; k < 3; ++k) {
                    int32_t d = c + k * 7;
                    float e = (float)d * data4[idx];
                    double f = (double)e * 1.1;
                    
                    /* Use all results to prevent elimination */
                    accum_int += d;
                    accum_float += e;
                    accum_double += f;
                    
                    /* Force register pressure with inline asm clobbers */
                    asm volatile("" : : "r"(d), "f"(e), "f"(f) : "cc", "memory");
                }
                
                idx++;
            }
        }
        
        /* Final sink to prevent dead code elimination */
        vsink_int = accum_int;
        vsink_float = accum_float;
        vsink_double = accum_double;
    }
}

/* Initialize arrays with varied data patterns */
static void init_arrays(int8_t *d1, int16_t *d2, int32_t *d3,
                        float *d4, double *d5, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        d1[i] = (int8_t)(i % 256 - 128);
        d2[i] = (int16_t)(i * 3 % 65536 - 32768);
        d3[i] = (int32_t)(i * 7);
        d4[i] = (float)(i) * 0.123f;
        d5[i] = (double)(i) * 0.456;
    }
}

int main(void) {
    const size_t SIZE = 256;
    
    /* Allocate arrays with different types to force different register modes */
    int8_t *data1 = (int8_t*)malloc(SIZE * sizeof(int8_t));
    int16_t *data2 = (int16_t*)malloc(SIZE * sizeof(int16_t));
    int32_t *data3 = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float *data4 = (float*)malloc(SIZE * sizeof(float));
    double *data5 = (double*)malloc(SIZE * sizeof(double));
    
    if (!data1 || !data2 || !data3 || !data4 || !data5) {
        return 1;
    }
    
    init_arrays(data1, data2, data3, data4, data5, SIZE);
    
    /* Call processing function multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 10; ++iter) {
        process_data(data1, data2, data3, data4, data5, SIZE);
        
        /* Modify data slightly between iterations */
        for (size_t i = 0; i < SIZE; ++i) {
            data1[i] += 1;
            data3[i] += iter;
        }
    }
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    free(data5);
    
    return 0;
}
