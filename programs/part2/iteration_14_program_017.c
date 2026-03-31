/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Helper functions to force copy propagation */
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
    float t3 = t2 * 1.5f;
    FORCE_COPY(t3);
    return t3 / a;
}

static inline int16_t process_short(int16_t a, int16_t b, int16_t c) {
    int16_t t1 = (int16_t)(a * b);
    int16_t t2 = (int16_t)(t1 + c);
    int16_t t3 = (int16_t)(t2 >> 1);
    FORCE_COPY(t3);
    return (int16_t)(t3 - a);
}

static inline double process_double(double a, double b, double c) {
    double t1 = a + b;
    double t2 = t1 * c;
    double t3 = t2 / 2.0;
    FORCE_COPY(t3);
    return t3 - a;
}

/* Function to create complex control flow */
static int kernel_loop(int8_t* arr1, int16_t* arr2, int32_t* arr3, 
                       float* arr4, double* arr5, int size) {
    int32_t acc_int = 0;
    float acc_float = 0.0f;
    double acc_double = 0.0;
    
    /* Outer loop - creates register pressure */
    for (int i = 0; i < size; i++) {
        volatile int outer_sink = i;
        (void)outer_sink;
        
        /* First inner loop with integer operations */
        for (int j = 0; j < 8; j++) {
            int32_t v1 = arr1[i] * 3;
            int32_t v2 = arr2[i] + j;
            int32_t v3 = v1 + v2;
            
            /* Force copy propagation context */
            int32_t v4 = process_int(v3, arr3[i], j + 1);
            FORCE_COPY(v4);
            
            /* Mixed-type computation */
            float f1 = (float)v4 * 0.5f;
            float f2 = process_float(f1, arr4[i], (float)j);
            FORCE_COPY(f2);
            
            /* Conditional block to split control flow */
            if (j & 1) {
                int16_t s1 = (int16_t)(v4 & 0xFFFF);
                int16_t s2 = process_short(s1, (int16_t)arr2[i], (int16_t)j);
                acc_int += s2;
            } else {
                double d1 = (double)v4 * 1.25;
                double d2 = process_double(d1, arr5[i], (double)j);
                acc_double += d2;
            }
            
            /* More arithmetic to increase register pressure */
            int32_t t1 = v4 * 7;
            int32_t t2 = t1 - arr3[(i + j) % size];
            int32_t t3 = t2 >> 2;
            int32_t t4 = process_int(t3, t1, t2);
            
            /* Volatile sink to prevent elimination */
            VOLATILE_SINK(t4);
            
            /* Another conditional with different types */
            if (t4 > 1000) {
                float ft1 = arr4[i] * 2.0f;
                float ft2 = ft1 + (float)t4;
                acc_float += ft2;
            }
        }
        
        /* Second inner loop with different operations */
        for (int k = 0; k < 4; k++) {
            /* Create dependent chain of computations */
            int32_t base = arr3[i] + k * 17;
            for (int m = 0; m < 3; m++) {
                int32_t val1 = base + m * 11;
                int32_t val2 = val1 * 3;
                int32_t val3 = val2 - arr1[(i + m) % size];
                
                /* Force register copies with inline function */
                int32_t result = process_int(val3, val1, val2);
                
                /* Mixed precision operations */
                double dval = (double)result * 0.333;
                float fval = (float)dval;
                
                /* More copies */
                int16_t sval = (int16_t)(result & 0x7FFF);
                int32_t ival = (int32_t)sval * 2;
                
                /* Use all values to prevent dead code elimination */
                asm volatile("" : : "r"(result), "r"(ival), "x"(dval), "x"(fval));
                
                acc_int += ival;
                acc_double += dval;
            }
        }
    }
    
    /* Final mixing of accumulators */
    int32_t final_int = acc_int + (int32_t)acc_float + (int32_t)acc_double;
    VOLATILE_SINK(final_int);
    
    return final_int;
}

/* Initialize arrays with different patterns */
static void init_arrays(int8_t* arr1, int16_t* arr2, int32_t* arr3,
                        float* arr4, double* arr5, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int8_t)((i * 13) & 0xFF);
        arr2[i] = (int16_t)((i * 17) & 0x7FFF);
        arr3[i] = i * 23;
        arr4[i] = (float)i * 0.73f;
        arr5[i] = (double)i * 1.17;
    }
}

int main() {
    const int SIZE = 256;
    
    /* Allocate arrays with different types */
    int8_t* arr1 = (int8_t*)malloc(SIZE * sizeof(int8_t));
    int16_t* arr2 = (int16_t*)malloc(SIZE * sizeof(int16_t));
    int32_t* arr3 = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float* arr4 = (float*)malloc(SIZE * sizeof(float));
    double* arr5 = (double*)malloc(SIZE * sizeof(double));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5) {
        return 1;
    }
    
    init_arrays(arr1, arr2, arr3, arr4, arr5, SIZE);
    
    /* Run multiple iterations to increase optimization opportunities */
    int total = 0;
    for (int iter = 0; iter < 3; iter++) {
        int result = kernel_loop(arr1, arr2, arr3, arr4, arr5, SIZE);
        total += result;
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < SIZE; i++) {
            arr1[i] += (int8_t)(iter * 5);
            arr3[i] += iter * 7;
        }
    }
    
    VOLATILE_SINK(total);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    
    return 0;
}
