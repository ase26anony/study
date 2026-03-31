#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_ops(float a, float b, float c, int iter) {
    volatile float result = 0.0f;
    for (int i = 0; i < (iter & 0x3F) + 1; i++) {
        result += a * b - c;
        a = b + 1.0f;
        b = c * 0.5f;
        c = result * 0.25f;
        asm volatile("" ::: "memory");
    }
    return result;
}

__attribute__((noinline))
static int helper_int_ops(int a, int b, int c, int d) {
    volatile int result = 0;
    /* Complex integer dependency chain */
    result = (a ^ b) + (c & d);
    result = (result << 3) | (result >> 29);
    result = result * 1103515245 + 12345;
    asm volatile("" ::: "memory");
    return result & 0x7FFFFFFF;
}

__attribute__((noinline))
static double helper_mixed_ops(int a, float b, double c) {
    volatile double result = 0.0;
    result = (double)a * 1.5 + (double)b * 2.0 + c * 0.75;
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
    return result;
}

/* Main complex scheduling stress function */
__attribute__((noinline))
static uint64_t complex_scheduling_stress(volatile int outer_limit) {
    /* High register pressure: many local variables of different types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    volatile int *ptr1 = &v1, *ptr2 = &v2;
    volatile float *fptr1 = &f1, *fptr2 = &f2;
    volatile double *dptr1 = &d1, *dptr2 = &d2;
    
    /* Additional variables for more pressure */
    int a1 = 100, a2 = 200, a3 = 300, a4 = 400, a5 = 500;
    int a6 = 600, a7 = 700, a8 = 800, a9 = 900, a10 = 1000;
    float b1 = 100.1f, b2 = 200.2f, b3 = 300.3f, b4 = 400.4f, b5 = 500.5f;
    double c1 = 1000.01, c2 = 2000.02, c3 = 3000.03, c4 = 4000.04, c5 = 5000.05;
    
    volatile uint64_t checksum = 0;
    
    /* Outer loop with volatile limit to prevent constant propagation */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Nested loops with variable bounds */
        int inner_limit = (outer & 0x7) + 3;  /* Variable from 3 to 10 */
        
        for (int middle = 0; middle < inner_limit; middle++) {
            /* Innermost loop with volatile counter */
            volatile int inner_counter = (middle * 7 + outer * 3) & 0xF;
            
            for (volatile int inner = 0; inner < inner_counter + 1; inner++) {
                /* Mixed operation dependency chains */
                v1 = v2 + v3 * v4 - v5;
                f1 = f2 * f3 + f4 / f5;
                d1 = d2 - d3 * d4 + d5;
                
                /* Memory operations interleaved with computation */
                *ptr1 = v6 + v7;
                v8 = *ptr2 * v9;
                
                *fptr1 = f6 * 1.5f + f7;
                f8 = *fptr2 / 2.0f - f9;
                
                *dptr1 = d6 * 0.75 + d7;
                d8 = *dptr2 * 1.25 - d9;
                
                /* Complex conditional execution paths */
                switch ((inner + middle + outer) & 0x3) {
                    case 0:
                        /* Branch A: FP math intensive */
                        f10 = helper_float_ops(f1, f2, f3, inner);
                        d10 = helper_mixed_ops(v1, f10, d1);
                        v10 = (int)(f10 * d10) & 0xFFF;
                        break;
                    case 1:
                        /* Branch B: Integer bit manipulation */
                        v10 = helper_int_ops(v1, v2, v3, v4);
                        v11 = (v10 ^ v5) + (v6 & v7);
                        v12 = (v11 << (inner & 0x7)) | (v11 >> (32 - (inner & 0x7)));
                        break;
                    case 2:
                        /* Branch C: Mixed operations with memory */
                        *ptr1 = helper_int_ops(*ptr1, v8, v9, v10);
                        *fptr1 = helper_float_ops(*fptr1, f8, f9, middle);
                        *dptr1 = helper_mixed_ops(*ptr1, *fptr1, *dptr1);
                        break;
                    case 3:
                        /* Branch D: Long dependency chain */
                        v13 = v14 * v15 + a1;
                        f3 = f4 * f5 - b1;
                        d3 = d4 / d5 + c1;
                        a1 = v13 + (int)f3;
                        b1 = f3 * 0.5f + (float)a1;
                        c1 = d3 * 2.0 + (double)b1;
                        break;
                }
                
                /* More arithmetic to create dependencies */
                a2 = a3 * a4 - a5 + v1;
                b2 = b3 * b4 + b5 / f1;
                c2 = c3 - c4 * c5 + d1;
                
                a3 = a4 ^ a5 + a6 & a7;
                b3 = b4 * 1.1f - b5 * 0.9f;
                c3 = c4 / 1.5 + c5 * 2.0;
                
                /* Memory barrier to force scheduler to handle dependencies */
                asm volatile("" ::: "memory");
                
                /* Update checksum with various values */
                checksum ^= (uint64_t)v1;
                checksum ^= (uint64_t)(*(uint32_t*)&f1);
                checksum ^= (uint64_t)(*(uint64_t*)&d1);
                checksum ^= (uint64_t)a2;
                checksum ^= (uint64_t)(*(uint32_t*)&b2);
                checksum ^= (uint64_t)(*(uint64_t*)&c2);
            }
            
            /* Additional operations between inner loops */
            v14 = v15 * a8 + v1;
            f4 = f5 * b8 + f1;
            d4 = d5 * c8 + d1;
            
            /* Call helper with current state */
            if (middle & 0x1) {
                f5 = helper_float_ops(f4, f3, f2, middle);
            } else {
                v15 = helper_int_ops(v14, v13, v12, v11);
            }
        }
        
        /* Update outer loop variables */
        v2 = v3 + outer;
        f2 = f3 * (1.0f + outer * 0.01f);
        d2 = d3 / (1.0 + outer * 0.005);
        
        a4 = a5 ^ outer;
        b4 = b5 + outer * 0.1f;
        c4 = c5 - outer * 0.01;
    }
    
    /* Final accumulation */
    checksum ^= (uint64_t)v1 ^ (uint64_t)v2 ^ (uint64_t)v3;
    checksum ^= (uint64_t)(*(uint32_t*)&f1) ^ (uint64_t)(*(uint32_t*)&f2);
    checksum ^= (uint64_t)(*(uint64_t*)&d1) ^ (uint64_t)(*(uint64_t*)&d2);
    checksum ^= (uint64_t)a1 ^ (uint64_t)a2 ^ (uint64_t)a3;
    checksum ^= (uint64_t)(*(uint32_t*)&b1) ^ (uint64_t)(*(uint32_t*)&b2);
    checksum ^= (uint64_t)(*(uint64_t*)&c1) ^ (uint64_t)(*(uint64_t*)&c2);
    
    return checksum;
}

int main(void) {
    volatile int iterations = 1000;  /* Prevent constant propagation */
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_stress(iterations);
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
