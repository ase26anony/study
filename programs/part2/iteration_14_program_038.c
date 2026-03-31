/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force copies and prevent optimization */
static inline int32_t copy_and_add(int32_t a, int32_t b, volatile int32_t* sink) {
    int32_t result = a + b;
    *sink = result;  /* Volatile write to prevent elimination */
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

static inline float promote_and_multiply(int16_t a, float b, volatile float* sink) {
    float promoted = (float)a;
    float result = promoted * b;
    *sink = result;
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

static inline double mixed_calc(int32_t i, float f, double d, volatile double* sink) {
    double temp1 = (double)i + (double)f;
    double temp2 = temp1 * d;
    double result = temp2 / (i + 1);
    *sink = result;
    asm volatile("" : : "r"(temp1), "r"(temp2), "r"(result) : "memory");
    return result;
}

/* Function with complex control flow to create many virtual registers */
void process_data(int8_t* arr1, int16_t* arr2, int32_t* arr3, 
                  float* arr4, double* arr5, size_t size) {
    volatile int32_t vsink1 = 0;
    volatile float vsink2 = 0.0f;
    volatile double vsink3 = 0.0;
    
    /* Outer loop creates register pressure */
    for (size_t outer = 0; outer < 3; ++outer) {
        int32_t accum_int = outer * 100;
        float accum_float = (float)outer * 0.5f;
        double accum_double = (double)outer * 1.5;
        
        /* First inner loop with integer operations */
        for (size_t i = 0; i < size; ++i) {
            /* Chain of dependent integer operations */
            int32_t t1 = arr1[i] * 3;
            int32_t t2 = t1 + arr2[i];
            int32_t t3 = t2 << 2;
            
            /* Force copy propagation context */
            int32_t copied = copy_and_add(t3, accum_int, &vsink1);
            
            /* Mixed-type operations */
            float ft1 = promote_and_multiply(arr2[i], accum_float, &vsink2);
            
            /* Conditional block creates control flow complexity */
            if (copied > 1000) {
                double dt1 = mixed_calc(t3, ft1, accum_double, &vsink3);
                arr5[i] = dt1;
                
                /* More operations in conditional path */
                int32_t t4 = copied - arr3[i];
                float ft2 = ft1 * 2.0f;
                asm volatile("" : : "r"(t4), "r"(ft2) : "memory");
            } else {
                /* Alternative path with different operations */
                double dt2 = (double)copied / (accum_double + 1.0);
                arr5[i] = dt2;
                
                /* Inline asm to clobber registers */
                asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5");
            }
            
            /* Update accumulators creating loop-carried dependencies */
            accum_int += arr3[i] / 2;
            accum_float += (float)arr1[i] * 0.1f;
            accum_double += (double)arr2[i] * 0.01;
        }
        
        /* Second inner loop with different operations */
        for (size_t j = 0; j < size / 2; ++j) {
            /* More mixed-precision calculations */
            int16_t s1 = arr2[j] + outer;
            int32_t i1 = (int32_t)s1 * 7;
            float f1 = (float)i1 * 3.14f;
            double d1 = (double)f1 * 2.71828;
            
            /* Chain of operations with volatile reads */
            volatile int32_t vread = arr3[j];
            int32_t i2 = i1 + vread;
            float f2 = f1 + arr4[j];
            
            /* Force register moves with inline asm */
            asm volatile("" : "=r"(i2), "=r"(f2) : "0"(i2), "1"(f2) : "memory");
            
            /* Store results */
            arr4[j] = f2;
            arr5[j + size/2] = d1 + (double)i2;
        }
        
        /* Third loop with pointer arithmetic */
        for (int8_t* ptr = arr1; ptr < arr1 + size; ptr += 4) {
            int32_t sum = 0;
            for (int k = 0; k < 4 && (ptr + k) < arr1 + size; ++k) {
                sum += ptr[k] * (k + 1);
            }
            
            /* Complex expression with multiple temporary values */
            double result = (double)sum / ((double)outer + 1.0);
            result = result * result - result;
            
            /* Volatile sink and asm to prevent optimization */
            vsink3 = result;
            asm volatile("" : : "r"(sum), "r"(result) : 
                        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        }
    }
}

/* Initialize arrays with different patterns */
void init_arrays(int8_t* arr1, int16_t* arr2, int32_t* arr3, 
                 float* arr4, double* arr5, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        arr1[i] = (int8_t)((i * 13) % 256 - 128);
        arr2[i] = (int16_t)((i * 17) % 65536 - 32768);
        arr3[i] = (int32_t)(i * 23 - size/2);
        arr4[i] = (float)(i * 0.37f - size * 0.5f);
        arr5[i] = (double)(i * 0.123 - size * 0.25);
    }
}

int main() {
    const size_t SIZE = 256;
    
    /* Allocate arrays with different types */
    int8_t* arr1 = (int8_t*)malloc(SIZE * sizeof(int8_t));
    int16_t* arr2 = (int16_t*)malloc(SIZE * sizeof(int16_t));
    int32_t* arr3 = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float* arr4 = (float*)malloc(SIZE * sizeof(float));
    double* arr5 = (double*)malloc(SIZE * sizeof(double));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5) {
        return 1;
    }
    
    /* Initialize with different data patterns */
    init_arrays(arr1, arr2, arr3, arr4, arr5, SIZE);
    
    /* Process data multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 10; ++iter) {
        process_data(arr1, arr2, arr3, arr4, arr5, SIZE);
        
        /* Modify arrays slightly between iterations */
        for (size_t i = 0; i < SIZE; i += 8) {
            arr1[i] += iter;
            arr2[i] -= iter * 2;
            arr3[i] += iter * 3;
            arr4[i] += (float)iter * 0.1f;
            arr5[i] += (double)iter * 0.05;
        }
    }
    
    /* Compute final checksum */
    volatile double final_sink = 0.0;
    for (size_t i = 0; i < SIZE; ++i) {
        final_sink += (double)arr1[i] + (double)arr2[i] + (double)arr3[i] 
                    + (double)arr4[i] + arr5[i];
    }
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    
    return (int)final_sink % 256;
}
