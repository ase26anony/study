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
    volatile double d2 = c / ((double)a + 1.0);
    asm volatile("" ::: "memory");
    return d1 - d2;
}

/* Complex function with high register pressure and mixed operations */
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
        /* Mixed operation dependency chain */
        v1 = v2 * v3 + i;
        f1 = (float)v1 / f2;
        *ptr1 = (int)(f1 * 100.0f);
        v2 = *ptr1 ^ v4;
        
        asm volatile("" ::: "memory");  /* Barrier 1 */
        
        /* Nested loops with variable bounds */
        volatile int inner_limit = (i % 10) + 5;
        for (j = 0; j < inner_limit; j++) {
            volatile int deeper_limit = (j % 3) + 2;
            for (k = 0; k < deeper_limit; k++) {
                /* Interleaved integer and float operations */
                v3 = v1 + v2 * k;
                f2 = f1 * f3 + (float)v3;
                d1 = (double)f2 * d2;
                v4 = (int)d1 ^ v3;
                
                /* Conditional execution paths */
                switch ((i + j + k) % 4) {
                    case 0:  /* FP math path */
                        f3 = helper_float_ops(f1, f2, f3);
                        d2 = helper_mixed_ops(v1, f3, d1);
                        v5 = (int)(d2 * 1000.0);
                        break;
                    case 1:  /* Integer bit manipulation path */
                        v6 = helper_int_ops(v1, v2, v3);
                        v7 = (v6 << 2) | (v6 >> 30);
                        v8 = v7 ^ 0xAAAAAAAA;
                        break;
                    case 2:  /* Memory access intensive path */
                        *ptr1 = v1 + v2;
                        *ptr2 = v3 * v4;
                        *fptr1 = f1 + f2;
                        *fptr2 = f3 * f4;
                        v9 = *ptr1 + *ptr2;
                        f5 = *fptr1 - *fptr2;
                        break;
                    case 3:  /* Mixed type conversion path */
                        f6 = (float)v5 / (float)v6;
                        d3 = (double)f6 * (double)v7;
                        v10 = (int)(d3 * 100.0);
                        d4 = helper_mixed_ops(v10, f6, d3);
                        break;
                }
                
                asm volatile("" ::: "memory");  /* Barrier 2 */
                
                /* More dependency chains */
                f4 = f3 * 2.0f - f2;
                v1 = v1 + (int)f4;
                d5 = d1 + d2 * 0.5;
                v2 = v2 ^ (int)d5;
                
                /* Function calls with scheduling side effects */
                if ((j + k) % 3 == 0) {
                    f7 = helper_float_ops(f4, f5, f6);
                    v3 = helper_int_ops(v1, v2, v3);
                } else if ((j + k) % 3 == 1) {
                    d6 = helper_mixed_ops(v4, f7, d5);
                    v4 = (int)d6;
                }
                
                /* Accumulate to checksum */
                checksum ^= (uint64_t)v1;
                checksum ^= (uint64_t)v2 << 8;
                checksum ^= (uint64_t)v3 << 16;
                checksum ^= (uint64_t)(*(uint32_t*)&f1) << 24;
                checksum ^= (uint64_t)(*(uint32_t*)&f2) << 32;
            }
            
            /* Additional operations between inner loops */
            v5 = v1 * v2 - v3;
            f8 = f3 / f4 + f5;
            d7 = d3 * d4 - d5;
            
            asm volatile("" ::: "memory");  /* Barrier 3 */
        }
        
        /* Complex conditional block */
        if (i % 7 == 0) {
            v6 = (v4 << 3) | (v5 >> 2);
            f9 = helper_float_ops(f6, f7, f8);
            d8 = (double)v6 * d7;
        } else if (i % 7 == 1) {
            v7 = helper_int_ops(v5, v6, v7);
            d9 = helper_mixed_ops(v7, f9, d8);
            f10 = (float)d9;
        } else {
            v8 = v6 ^ v7 ^ v8;
            f10 = f8 * f9 - f10;
            d10 = d8 + d9 * 0.25;
        }
        
        /* Final dependency chain in outer loop */
        v9 = v7 + v8 * i;
        f1 = f9 + f10 * (float)i;
        d1 = d9 + d10;
        v10 = (int)(f1 * d1) ^ v9;
        
        checksum ^= (uint64_t)v10 << 40;
        checksum ^= (uint64_t)(*(uint64_t*)&d1) << 48;
    }
    
    return checksum;
}

int main(void) {
    volatile int outer_limit = 1000;  /* Volatile to prevent constant propagation */
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_function(outer_limit);
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
