#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_ops(float a, float b, float c, float d) {
    volatile float barrier = a + b;
    asm volatile("" ::: "memory");
    float t1 = barrier * c;
    float t2 = t1 / (d + 1.0f);
    float t3 = t2 - barrier;
    asm volatile("" ::: "memory");
    return t3 * 0.5f;
}

__attribute__((noinline))
static int helper_int_ops(int a, int b, int c, int d) {
    volatile int barrier = a ^ b;
    asm volatile("" ::: "memory");
    int t1 = barrier & c;
    int t2 = t1 | d;
    int t3 = t2 << 3;
    int t4 = t3 >> 1;
    asm volatile("" ::: "memory");
    return t4 + barrier;
}

__attribute__((noinline))
static double helper_double_chain(double a, double b, int iter) {
    volatile double acc = a;
    asm volatile("" ::: "memory");
    for (int i = 0; i < iter; i++) {
        acc = acc * 1.1 + b * 0.9;
        if (i % 4 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    return acc;
}

/* Main complex scheduling function */
static uint64_t complex_scheduling_function(volatile int outer_limit) {
    /* High register pressure: 30+ scalar variables of mixed types */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8;
    int *ptr1, *ptr2;
    float *fptr1, *fptr2;
    volatile int mem_barrier;
    
    /* Initialize variables to create dependencies */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04;
    d5 = 5.05; d6 = 6.06; d7 = 7.07; d8 = 8.08;
    
    /* Allocate small arrays to create memory dependencies */
    int arr1[16], arr2[16];
    float farr1[16], farr2[16];
    
    for (int i = 0; i < 16; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        farr1[i] = i * 0.5f;
        farr2[i] = i * 1.5f;
    }
    
    ptr1 = arr1;
    ptr2 = arr2;
    fptr1 = farr1;
    fptr2 = farr2;
    
    volatile uint64_t checksum = 0;
    volatile int loop_counter = 0;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Mixed operation dependency chains across types */
        v1 = v2 + v3;
        f1 = (float)v1 * f2;
        mem_barrier = (int)f1;
        asm volatile("" ::: "memory");
        
        /* Nested loops with variable bounds */
        volatile int middle_limit = (outer % 8) + 4;
        for (int middle = 0; middle < middle_limit; middle++) {
            /* Inner loop with volatile-dependent bound */
            volatile int inner_limit = (middle % 4) + 2;
            for (volatile int inner = 0; inner < inner_limit; inner++) {
                /* Complex mixed-type dependency chain */
                v4 = v5 * v6 + inner;
                f3 = helper_float_ops(f4, f5, (float)v4, f6);
                d1 = d2 + (double)f3;
                v7 = helper_int_ops(v8, v9, (int)d1, v10);
                f7 = f8 - f9 * (float)v7;
                
                /* Memory operations interleaved */
                arr1[inner % 16] = v7;
                farr1[inner % 16] = f7;
                asm volatile("" ::: "memory");
                
                /* Load and use stored values */
                v5 = arr2[(inner + 1) % 16];
                f5 = farr2[(inner + 2) % 16];
                
                /* Conditional execution paths */
                switch (inner % 5) {
                    case 0:
                        /* FP math path */
                        d3 = d4 * d5 + helper_double_chain(d6, d7, 3);
                        f8 = (float)d3 * 0.25f;
                        v8 = (int)f8 ^ v9;
                        break;
                    case 1:
                        /* Integer bit manipulation path */
                        v9 = (v10 << 3) | (v1 >> 2);
                        v10 = v9 ^ ~v2;
                        v1 = v10 & 0xFFFF;
                        break;
                    case 2:
                        /* Mixed type conversion path */
                        f9 = (float)v3 * 0.333f;
                        d4 = (double)f9 * 1.414;
                        v2 = (int)d4 % 256;
                        f10 = (float)v2 / 128.0f;
                        break;
                    case 3:
                        /* Memory intensive path */
                        for (int j = 0; j < 4; j++) {
                            arr2[(inner + j) % 16] = arr1[j] + v4;
                            farr2[(inner + j) % 16] = farr1[j] * f3;
                        }
                        asm volatile("" ::: "memory");
                        break;
                    case 4:
                        /* Function call intensive path */
                        v3 = helper_int_ops(v4, v5, v6, inner);
                        f4 = helper_float_ops(f5, f6, f7, (float)v3);
                        d5 = helper_double_chain(d6, d7, 2);
                        break;
                }
                
                /* Another memory barrier */
                asm volatile("" ::: "memory");
                
                /* Update checksum with all live variables */
                checksum ^= (uint64_t)v1;
                checksum ^= (uint64_t)(*(uint32_t*)&f1);
                checksum ^= (uint64_t)(*(uint64_t*)&d1);
                checksum ^= (uint64_t)v2;
                checksum ^= (uint64_t)arr1[inner % 16];
            }
            
            /* Additional operations between inner loops */
            v6 = v7 + middle;
            f6 = helper_float_ops(f7, f8, (float)v6, f9);
            d6 = d7 * 0.99 + (double)f6;
            
            /* Call helper with many live variables */
            if (middle % 3 == 0) {
                v7 = helper_int_ops(v8, v9, (int)d6, v10);
            }
        }
        
        /* Update loop counter and checksum */
        loop_counter++;
        checksum ^= (uint64_t)outer;
        checksum ^= (uint64_t)loop_counter;
        
        /* More mixed operations */
        v8 = v9 * v10 - outer;
        f8 = (float)v8 / 3.14159f;
        d7 = (double)f8 * 2.71828;
        v9 = (int)d7 & 0xFF;
        
        /* Store to all arrays */
        for (int i = 0; i < 8; i++) {
            arr1[i] = v9 + i;
            arr2[i + 8] = v8 - i;
            farr1[i] = f8 + (float)i;
            farr2[i + 8] = (float)v9 * 0.1f;
        }
        asm volatile("" ::: "memory");
    }
    
    /* Final accumulation from all arrays */
    for (int i = 0; i < 16; i++) {
        checksum ^= (uint64_t)arr1[i];
        checksum ^= (uint64_t)arr2[i];
        checksum ^= (uint64_t)(*(uint32_t*)&farr1[i]);
        checksum ^= (uint64_t)(*(uint32_t*)&farr2[i]);
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
