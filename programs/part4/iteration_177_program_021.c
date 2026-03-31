#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_op(float a, float b, float c) {
    volatile float v1 = a * b;
    volatile float v2 = b / c;
    asm volatile("" ::: "memory");
    return v1 + v2 - (a * c);
}

__attribute__((noinline))
static int helper_int_op(int a, int b, int c) {
    volatile int v1 = a ^ b;
    volatile int v2 = b | c;
    volatile int v3 = a & ~c;
    asm volatile("" ::: "memory");
    return (v1 + v2) * v3;
}

__attribute__((noinline))
static double helper_double_op(double a, double b, double* mem) {
    volatile double temp = *mem;
    *mem = a * b + temp;
    asm volatile("" ::: "memory");
    return (*mem) / (a + 1.0);
}

/* Complex function with high register pressure and mixed operations */
__attribute__((noinline))
static uint64_t complex_scheduling_function(volatile int outer_limit) {
    /* Create high register pressure with many live variables */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    
    /* Additional variables for memory operations */
    int arr_int[32];
    float arr_float[32];
    double arr_double[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 1.5f;
        arr_double[i] = i * 2.5;
    }
    
    volatile uint64_t checksum = 0;
    
    /* Outer loop with volatile limit to prevent optimization */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Nested loops with variable bounds */
        volatile int inner_limit = (outer % 8) + 2;
        
        for (volatile int mid = 0; mid < inner_limit; mid++) {
            /* Innermost loop with data-dependent trip count */
            int inner_count = (v1 + v2 + mid) % 16 + 4;
            
            for (int inner = 0; inner < inner_count; inner++) {
                /* Mixed operation dependency chains */
                
                /* Integer arithmetic chain */
                v1 = v1 * 1103515245 + 12345;
                v2 = v2 ^ v1;
                v3 = v3 + (v2 >> 3);
                v4 = v4 | (v3 << 2);
                v5 = helper_int_op(v4, v5, inner);
                
                /* Floating-point chain with memory accesses */
                f1 = f1 * 1.1f + f2;
                f2 = f2 / (f3 + 0.1f);
                f3 = helper_float_op(f1, f2, f3);
                
                /* Memory load/store operations */
                int idx = (v1 + inner) & 31;
                arr_int[idx] = arr_int[idx] + v2;
                arr_float[idx] = arr_float[idx] * f3;
                
                /* Double precision operations */
                d1 = d1 * 1.01 - d2;
                d2 = helper_double_op(d1, d2, &arr_double[idx]);
                
                /* Conditional execution paths */
                switch (inner & 7) {
                    case 0:
                        /* FP math branch */
                        f4 = f4 * f5 - f6;
                        f5 = helper_float_op(f4, f5, f6);
                        v6 = (int)(f5 * 100.0f);
                        break;
                    case 1:
                        /* Integer bit manipulation branch */
                        v7 = v7 ^ v8;
                        v8 = (v8 << 3) | (v7 >> 5);
                        v9 = helper_int_op(v7, v8, v9);
                        f6 = (float)v9 * 0.01f;
                        break;
                    case 2:
                        /* Memory intensive branch */
                        for (int j = 0; j < 4; j++) {
                            int idx2 = (idx + j) & 31;
                            arr_int[idx2] += v10;
                            arr_float[idx2] *= 1.01f;
                            arr_double[idx2] = helper_double_op(arr_double[idx2], d3, &d4);
                        }
                        break;
                    case 3:
                        /* Mixed type conversions */
                        v10 = (int)(f7 * 10.0f);
                        f7 = (float)v10 / 10.0f;
                        d3 = (double)v10 + d4;
                        break;
                    case 4:
                        /* Complex dependency chain */
                        v11 = v11 * 3 + v12;
                        v12 = v12 ^ v11;
                        f8 = f8 + (float)v12;
                        d4 = d4 * (double)f8;
                        arr_int[idx] = (int)d4;
                        break;
                    case 5:
                        /* Another mixed chain */
                        v13 = helper_int_op(v13, v14, v15);
                        f9 = helper_float_op(f9, f10, (float)v13);
                        d5 = (double)f9 * d6;
                        break;
                    case 6:
                        /* Memory barrier intensive */
                        asm volatile("" ::: "memory");
                        v14 = arr_int[idx];
                        asm volatile("" ::: "memory");
                        f10 = arr_float[idx];
                        asm volatile("" ::: "memory");
                        d6 = arr_double[idx];
                        asm volatile("" ::: "memory");
                        break;
                    case 7:
                        /* All variable update */
                        v15 = v1 + v2 + v3 + v4 + v5;
                        f1 = f1 + f2 + f3 + f4;
                        d7 = d1 + d2 + d3 + d4;
                        break;
                }
                
                /* Additional memory barrier to inhibit reordering */
                asm volatile("" ::: "memory");
                
                /* Update checksum with various values */
                checksum ^= (uint64_t)v1;
                checksum ^= (uint64_t)(*(uint32_t*)&f1);
                checksum ^= (uint64_t)(*(uint64_t*)&d1);
                checksum ^= (uint64_t)arr_int[idx];
            }
            
            /* Call helper functions across loop iterations */
            if (mid % 3 == 0) {
                v16 = helper_int_op(v1, v2, v3);
                f11 = helper_float_op(f1, f2, f3);
                d8 = helper_double_op(d1, d2, &d3);
            }
            
            /* More register pressure variables */
            volatile int v16 = v1 + v2;
            volatile int v17 = v3 * v4;
            volatile int v18 = v5 ^ v6;
            volatile int v19 = v7 | v8;
            volatile int v20 = v9 & v10;
            volatile float f11 = f1 * f2;
            volatile float f12 = f3 / f4;
            volatile float f13 = f5 + f6;
            volatile float f14 = f7 - f8;
            volatile float f15 = f9 * f10;
            volatile double d8 = d1 * d2;
            volatile double d9 = d3 / d4;
            volatile double d10 = d5 + d6;
            
            /* Use all variables to keep them live */
            v16 = v16 + v17 - v18 + v19 ^ v20;
            f11 = f11 * f12 + f13 - f14 / f15;
            d8 = d8 * d9 + d10;
        }
        
        /* Update array elements based on outer iteration */
        for (int i = 0; i < 32; i += 4) {
            arr_int[i] += outer;
            arr_float[i] *= 1.0f + (float)outer * 0.01f;
            arr_double[i] = helper_double_op(arr_double[i], d1, &d2);
        }
    }
    
    /* Final accumulation from arrays */
    for (int i = 0; i < 32; i++) {
        checksum ^= (uint64_t)arr_int[i];
        checksum ^= (uint64_t)(*(uint32_t*)&arr_float[i]);
        checksum ^= (uint64_t)(*(uint64_t*)&arr_double[i]);
    }
    
    return checksum;
}

int main(void) {
    volatile int iterations = 100;
    volatile uint64_t result = 0;
    
    printf("Starting complex scheduling stress test...\n");
    
    /* Run multiple times to increase chance of hitting scheduler paths */
    for (int run = 0; run < 3; run++) {
        result ^= complex_scheduling_function(iterations + run);
        printf("Run %d complete, partial result: %llu\n", 
               run, (unsigned long long)result);
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)result);
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
