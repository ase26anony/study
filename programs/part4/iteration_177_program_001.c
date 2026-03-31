#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_ops(float a, float b, float c) {
    volatile float v1 = a * b + c;
    volatile float v2 = a / (b + 1.0f);
    asm volatile("" ::: "memory");
    return v1 - v2;
}

__attribute__((noinline))
static int helper_int_ops(int a, int b, int c) {
    volatile int v1 = (a ^ b) | c;
    volatile int v2 = (a & b) << 3;
    asm volatile("" ::: "memory");
    return v1 + v2;
}

__attribute__((noinline))
static double helper_mixed_ops(int a, float b, double c) {
    volatile double d1 = (double)a * (double)b + c;
    volatile double d2 = c / (double)(a + 1);
    asm volatile("" ::: "memory");
    return d1 - d2;
}

/* Complex function with high register pressure and mixed operations */
__attribute__((noinline))
static uint64_t complex_scheduling_function(volatile int outer_limit) {
    /* High register pressure: 30+ scalar variables of different types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    volatile int *ptr1 = &v1, *ptr2 = &v2;
    volatile float *fptr1 = &f1, *fptr2 = &f2;
    volatile double *dptr1 = &d1, *dptr2 = &d2;
    
    uint64_t checksum = 0;
    volatile int i, j, k;
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        /* Mixed operation dependency chains */
        v1 = v2 + v3 * v4;
        f1 = (float)v1 * f2 + f3;
        *ptr1 = (int)(f1 * 100.0f);
        asm volatile("" ::: "memory");
        
        /* Nested loops with variable bounds */
        volatile int inner_limit = (i % 10) + 5;
        for (j = 0; j < inner_limit; j++) {
            /* More mixed operations creating dependencies */
            d1 = (double)v1 * d2 + d3;
            v2 = (int)d1 ^ v3;
            f2 = f3 * f4 - (float)v2;
            
            /* Deeply nested loop */
            for (k = 0; k < 3; k++) {
                /* Interleaved memory accesses */
                *ptr2 = *ptr1 + k;
                *fptr1 = *fptr2 * 1.1f;
                *dptr1 = *dptr2 / 2.0;
                
                /* Function calls with scheduling side effects */
                if (k == 0) {
                    f3 = helper_float_ops(f1, f2, f3);
                } else if (k == 1) {
                    v3 = helper_int_ops(v1, v2, v3);
                } else {
                    d3 = helper_mixed_ops(v1, f1, d1);
                }
                
                asm volatile("" ::: "memory");
            }
            
            /* Conditional execution paths */
            switch (j % 4) {
                case 0: /* FP math path */
                    f4 = f5 * f6 - f7;
                    f5 = f4 / f8 + f9;
                    checksum += (uint64_t)(f4 * 1000);
                    break;
                case 1: /* Integer bit manipulation path */
                    v4 = (v5 ^ v6) | v7;
                    v5 = (v4 << 2) & 0xFF;
                    v6 = ~v5;
                    checksum ^= (uint64_t)v4;
                    break;
                case 2: /* Memory intensive path */
                    *ptr1 = *ptr2 + v8;
                    v8 = *ptr1 * 2;
                    *ptr2 = v8 / 3;
                    checksum += (uint64_t)*ptr1;
                    break;
                case 3: /* Mixed type path */
                    d4 = (double)v9 * d5;
                    f6 = (float)d4 + f7;
                    v9 = (int)f6;
                    checksum ^= (uint64_t)(d4 * 100);
                    break;
            }
            
            /* Additional dependency chains */
            v7 = v8 + v9 * v10;
            f7 = f8 * f9 - f10;
            d7 = d8 * d9 + d10;
            
            /* Use results to create true dependencies */
            v8 = (int)(f7 * 10.0f) + v7;
            f8 = (float)v8 / 100.0f + f7;
            d8 = (double)v8 + d7;
            
            asm volatile("" ::: "memory");
        }
        
        /* More operations to increase scheduling complexity */
        v9 = helper_int_ops(v1, v2, v3);
        f9 = helper_float_ops(f1, f2, f3);
        d9 = helper_mixed_ops(v4, f4, d4);
        
        /* Accumulate to checksum with volatile barrier */
        checksum += (uint64_t)v9 + (uint64_t)(f9 * 100) + (uint64_t)(d9 * 1000);
        asm volatile("" ::: "memory");
    }
    
    /* Final complex calculation using all variables */
    volatile int final_int = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    volatile float final_float = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
    volatile double final_double = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    
    checksum ^= (uint64_t)final_int;
    checksum += (uint64_t)(final_float * 10000);
    checksum ^= (uint64_t)(final_double * 100000);
    
    return checksum;
}

int main(void) {
    volatile int outer_iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    /* Execute the complex function */
    uint64_t result = complex_scheduling_function(outer_iterations);
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: %llu\n", (unsigned long long)result);
    
    /* Additional volatile operations to ensure all code paths are used */
    volatile int dummy = 0;
    for (volatile int i = 0; i < 10; i++) {
        dummy += i * 2;
        asm volatile("" ::: "memory");
    }
    
    printf("Test completed.\n");
    return (int)(result % 256);
}
