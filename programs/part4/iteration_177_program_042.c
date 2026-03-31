#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_ops(float a, float b, float c) {
    volatile float v1 = a * b + c;
    volatile float v2 = b / (a + 1.0f);
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
static uint64_t complex_scheduling_function(volatile int outer_iterations) {
    /* High register pressure: many local variables of different types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    volatile int arr1[32], arr2[32];
    volatile float farr[32];
    volatile double darr[32];
    
    uint64_t checksum = 0;
    volatile int i, j, k;
    
    /* Initialize arrays with volatile accesses */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
        asm volatile("" ::: "memory");
    }
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_iterations; i++) {
        volatile int inner_limit = (i % 8) + 4;  /* Variable loop bounds */
        
        /* Nested loops with data-dependent trip counts */
        for (j = 0; j < inner_limit; j++) {
            volatile int innermost_limit = (j % 4) + 2;
            
            for (k = 0; k < innermost_limit; k++) {
                /* Mixed operation dependency chains */
                v1 = v2 + v3 * v4;
                f1 = f2 * f3 + (float)v1;
                d1 = (double)f1 + d2 * d3;
                
                /* Memory accesses interleaved with computation */
                arr1[(v1 + k) % 32] = v2 + v3;
                farr[(j + k) % 32] = f1 * f2;
                darr[(i + j) % 32] = d1 - d2;
                
                /* Call helper functions creating cross-call dependencies */
                f3 = helper_float_ops(f1, f2, f3);
                v4 = helper_int_ops(v1, v2, v3);
                d3 = helper_mixed_ops(v4, f3, d3);
                
                /* Conditional execution paths with different operation mixes */
                switch ((i + j + k) % 5) {
                    case 0:
                        /* FP math intensive path */
                        f4 = f5 * f6 + f7 / f8;
                        d4 = d5 * d6 - d7 / d8;
                        v5 = (int)(f4 * 100.0f) ^ (int)(d4 * 100.0);
                        asm volatile("" ::: "memory");
                        break;
                    case 1:
                        /* Integer bit manipulation path */
                        v6 = (v7 << 3) | (v8 >> 2);
                        v7 = v6 ^ v9 & v10;
                        v8 = ~v7 | (v6 & 0xFF);
                        asm volatile("" ::: "memory");
                        break;
                    case 2:
                        /* Memory intensive path */
                        arr2[(v6 + i) % 32] = v7 + v8;
                        v9 = arr1[(v8 + j) % 32] * arr2[(v9 + k) % 32];
                        f5 = farr[(v10 + i) % 32] + farr[(v1 + j) % 32];
                        asm volatile("" ::: "memory");
                        break;
                    case 3:
                        /* Mixed type conversion path */
                        f6 = (float)v9 / (float)(v10 + 1);
                        d5 = (double)f6 * (double)v9;
                        v10 = (int)d5 ^ (int)(f6 * 1000.0f);
                        asm volatile("" ::: "memory");
                        break;
                    default:
                        /* All operations path */
                        v1 = v2 * v3 + v4;
                        f7 = helper_float_ops(f6, f7, f8);
                        d6 = d5 * d7 + helper_mixed_ops(v1, f7, d8);
                        arr1[(i + k) % 32] = v1 + (int)f7;
                        asm volatile("" ::: "memory");
                }
                
                /* Additional dependency chains */
                v2 = v3 ^ v4;
                f2 = f3 * 1.5f - f4;
                d2 = d3 / 2.0 + d4;
                
                /* More memory operations */
                v3 = arr1[(v2 + i) % 32] + arr2[(v3 + j) % 32];
                f8 = farr[(v4 + k) % 32] * 2.0f;
                d7 = darr[(v5 + i) % 32] / 3.0;
                
                /* Final barrier in innermost loop */
                asm volatile("" ::: "memory");
            }
            
            /* Update checksum with various values */
            checksum ^= (uint64_t)v1;
            checksum += (uint64_t)(f1 * 1000.0f);
            checksum ^= (uint64_t)(d1 * 1000.0);
            checksum += (uint64_t)arr1[j % 32];
        }
        
        /* Re-initialize some values to prevent optimization */
        if (i % 16 == 0) {
            v1 = i;
            f1 = i * 1.1f;
            d1 = i * 1.01;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Final accumulation from all arrays */
    for (i = 0; i < 32; i++) {
        checksum ^= (uint64_t)arr1[i];
        checksum += (uint64_t)(farr[i] * 100.0f);
        checksum ^= (uint64_t)(darr[i] * 100.0);
        checksum += (uint64_t)arr2[i];
    }
    
    return checksum;
}

int main(void) {
    volatile int iterations = 1000;  /* Prevent constant propagation */
    uint64_t result;
    
    printf("Starting complex scheduling test...\n");
    
    /* Execute the complex function */
    result = complex_scheduling_function(iterations);
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    
    /* Additional volatile store to ensure all operations complete */
    volatile uint64_t final_result = result;
    (void)final_result;
    
    return 0;
}
