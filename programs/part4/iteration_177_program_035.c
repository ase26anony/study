#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_ops(float a, float b, float c, int iterations) {
    volatile float result = a;
    for (int i = 0; i < iterations; i++) {
        result = result * b + c;
        result = result / (b + 1.0f);
        asm volatile("" ::: "memory");  // Memory barrier
    }
    return result;
}

__attribute__((noinline))
static int helper_int_ops(int a, int b, int c, int iterations) {
    volatile int result = a;
    for (int i = 0; i < iterations; i++) {
        result = (result ^ b) + c;
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        asm volatile("" ::: "memory");  // Memory barrier
    }
    return result;
}

__attribute__((noinline))
static double helper_mixed_ops(double a, int b, float c, int iterations) {
    volatile double result = a;
    for (int i = 0; i < iterations; i++) {
        result = result + (double)b * 0.5;
        result = result * (double)c - 1.0;
        asm volatile("" ::: "memory");  // Memory barrier
    }
    return result;
}

/* Main complex function with high register pressure */
static uint64_t complex_scheduling_function(volatile int outer_iterations) {
    /* Many local variables to create register pressure (30+) */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile int *ptr1 = &v1, *ptr2 = &v2;
    volatile float *fptr1 = &f1, *fptr2 = &f2;
    volatile double *dptr1 = &d1, *dptr2 = &d2;
    volatile int arr1[8], arr2[8];
    volatile float farr[8];
    volatile double darr[8];
    
    /* Initialize arrays with volatile accesses */
    for (int i = 0; i < 8; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
        asm volatile("" ::: "memory");  // Memory barrier
    }
    
    uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        /* Nested loops with variable bounds */
        volatile int middle_limit = (outer % 5) + 3;
        
        for (volatile int middle = 0; middle < middle_limit; middle++) {
            /* Inner loop with dependency on outer and middle */
            volatile int inner_limit = ((outer * 7 + middle * 3) % 8) + 2;
            
            for (volatile int inner = 0; inner < inner_limit; inner++) {
                /* Mixed operation dependency chains */
                int temp_int = v1 + v2 * v3 - v4;
                float temp_float = f1 * f2 + (float)temp_int;
                double temp_double = d1 / d2 + (double)temp_float;
                
                /* Memory operations with dependencies */
                arr1[inner % 8] = temp_int + inner;
                farr[middle % 8] = temp_float * 1.1f;
                darr[outer % 8] = temp_double * 0.9;
                
                /* Load and use results */
                v5 = arr1[(inner + 1) % 8] * 2;
                f3 = farr[(middle + 1) % 8] / 2.0f;
                d3 = darr[(outer + 1) % 8] + 1.0;
                
                /* More complex dependency chains */
                v6 = (v5 ^ v1) + (v2 & v3) | v4;
                f4 = f3 * f1 - f2 / f3;
                d4 = d3 * d1 + d2 - d3;
                
                asm volatile("" ::: "memory");  // Memory barrier
                
                /* Conditional execution paths */
                switch ((inner + middle + outer) % 4) {
                    case 0:
                        /* FP math branch */
                        f5 = helper_float_ops(f4, f1, f2, 2);
                        d5 = d4 * 3.14159 + d3;
                        v7 = (int)(f5 * 100.0f) + v6;
                        break;
                    case 1:
                        /* Integer bit manipulation branch */
                        v7 = helper_int_ops(v6, v1, v2, 2);
                        v8 = (v7 << 3) | (v7 >> 29);
                        v9 = v8 ^ 0xAAAAAAAA;
                        break;
                    case 2:
                        /* Mixed operations branch */
                        d5 = helper_mixed_ops(d4, v6, f3, 2);
                        f5 = (float)d5 * 2.0f;
                        v7 = (int)d5 + v6 * 3;
                        break;
                    case 3:
                        /* Memory intensive branch */
                        for (int i = 0; i < 4; i++) {
                            arr2[i] = arr1[i] * arr1[i+1];
                            farr[i] = farr[i] * 1.5f + (float)arr2[i];
                            darr[i] = darr[i] / 1.7 - (double)arr2[i];
                        }
                        v7 = arr2[0] + arr2[1] + arr2[2];
                        break;
                }
                
                /* Update checksum with all live variables */
                checksum ^= (uint64_t)v7;
                checksum ^= (uint64_t)(*(uint32_t*)&f5);
                checksum ^= (uint64_t)(*(uint64_t*)&d5);
                checksum += (uint64_t)arr1[inner % 8];
                checksum ^= (uint64_t)(*(uint32_t*)&farr[middle % 8]);
                
                asm volatile("" ::: "memory");  // Memory barrier
            }
            
            /* Function call with scheduling side effects */
            if (middle % 2 == 0) {
                v10 = helper_int_ops(v7, v8, v9, 1);
                f1 = helper_float_ops(f5, f2, f3, 1);
            } else {
                d1 = helper_mixed_ops(d5, v10, f1, 1);
            }
        }
        
        /* More operations between outer loop iterations */
        v1 = v1 + v10;
        v2 = v2 * v7 - v8;
        f2 = f1 * 1.618f + f3;
        d2 = d1 * 0.7071 + d3;
        
        /* Pointer chasing to create memory dependencies */
        *ptr1 = v1;
        *ptr2 = *ptr1 + v2;
        *fptr1 = f2;
        *fptr2 = *fptr1 * 0.5f;
        *dptr1 = d2;
        *dptr2 = *dptr1 + 1.0;
        
        asm volatile("" ::: "memory");  // Memory barrier
    }
    
    /* Final accumulation from all variables */
    checksum ^= (uint64_t)v1 ^ (uint64_t)v2 ^ (uint64_t)v3;
    checksum ^= (uint64_t)v4 ^ (uint64_t)v5 ^ (uint64_t)v6;
    checksum ^= (uint64_t)v7 ^ (uint64_t)v8 ^ (uint64_t)v9;
    checksum ^= (uint64_t)v10;
    checksum ^= (uint64_t)(*(uint32_t*)&f1) ^ (uint64_t)(*(uint32_t*)&f2);
    checksum ^= (uint64_t)(*(uint32_t*)&f3) ^ (uint64_t)(*(uint32_t*)&f4);
    checksum ^= (uint64_t)(*(uint32_t*)&f5);
    checksum ^= (uint64_t)(*(uint64_t*)&d1) ^ (uint64_t)(*(uint64_t*)&d2);
    checksum ^= (uint64_t)(*(uint64_t*)&d3) ^ (uint64_t)(*(uint64_t*)&d4);
    checksum ^= (uint64_t)(*(uint64_t*)&d5);
    
    for (int i = 0; i < 8; i++) {
        checksum ^= (uint64_t)arr1[i];
        checksum ^= (uint64_t)arr2[i];
        checksum ^= (uint64_t)(*(uint32_t*)&farr[i]);
        checksum ^= (uint64_t)(*(uint64_t*)&darr[i]);
    }
    
    return checksum;
}

int main(void) {
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
