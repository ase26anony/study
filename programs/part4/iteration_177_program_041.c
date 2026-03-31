#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_op(float a, float b, float c) {
    volatile float v1 = a * b + c;
    volatile float v2 = a / (b + 1.0f);
    asm volatile("" ::: "memory");
    return v1 - v2;
}

__attribute__((noinline))
static int helper_int_op(int a, int b, int c) {
    volatile int v1 = (a ^ b) | c;
    volatile int v2 = (a & b) << 3;
    asm volatile("" ::: "memory");
    return v1 + v2;
}

__attribute__((noinline))
static double helper_double_op(double a, double b, int c) {
    volatile double v1 = a * b;
    volatile double v2 = a / (b + 1.0);
    asm volatile("" ::: "memory");
    return v1 - v2 + c;
}

__attribute__((noinline))
static void* helper_mem_op(void* ptr, int offset, int value) {
    volatile int* p = (volatile int*)ptr;
    p[offset] = value;
    asm volatile("" ::: "memory");
    return ptr;
}

/* Main complex function with high register pressure */
static uint64_t complex_scheduling_function(void) {
    /* High register pressure: 30+ variables of mixed types */
    volatile int v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5;
    volatile int v5 = 6, v6 = 7, v7 = 8, v8 = 9, v9 = 10;
    volatile float f0 = 1.1f, f1 = 2.2f, f2 = 3.3f, f3 = 4.4f, f4 = 5.5f;
    volatile float f5 = 6.6f, f6 = 7.7f, f7 = 8.8f, f8 = 9.9f, f9 = 10.10f;
    volatile double d0 = 1.01, d1 = 2.02, d2 = 3.03, d3 = 4.04, d4 = 5.05;
    volatile double d5 = 6.06, d6 = 7.07, d7 = 8.08, d8 = 9.09, d9 = 10.10;
    volatile int* mem_ptr;
    volatile int arr[64];
    volatile uint64_t checksum = 0;
    
    /* Initialize array with volatile writes */
    for (int i = 0; i < 64; i++) {
        arr[i] = i * 3;
        asm volatile("" ::: "memory");
    }
    
    mem_ptr = (volatile int*)arr;
    
    /* Outer loop with volatile limit */
    volatile int outer_limit = 100;
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        
        /* Nested loop with variable bounds */
        volatile int middle_limit = outer + 5;
        for (volatile int middle = 0; middle < middle_limit; middle++) {
            
            /* Inner loop with complex dependency chain */
            volatile int inner_limit = (middle * 3) % 10 + 2;
            for (volatile int inner = 0; inner < inner_limit; inner++) {
                
                /* Mixed operation dependency chains */
                /* int -> float -> memory -> int */
                v0 = v1 + v2;
                f0 = (float)v0 * f1;
                arr[v0 % 64] = (int)f0;
                v3 = arr[(v0 + 1) % 64];
                
                /* float -> double -> int -> memory */
                f2 = f3 * f4 + f5;
                d0 = (double)f2 * d1;
                v4 = (int)d0;
                helper_mem_op((void*)arr, v4 % 64, v4);
                
                /* Memory barrier to force scheduler work */
                asm volatile("" ::: "memory");
                
                /* Conditional execution paths */
                switch ((inner + outer) % 4) {
                    case 0: /* FP math path */
                        f3 = helper_float_op(f3, f4, f5);
                        d2 = helper_double_op(d2, d3, v5);
                        v5 = (int)(f3 * 100.0f);
                        break;
                    case 1: /* Integer bit manipulation path */
                        v6 = helper_int_op(v6, v7, v8);
                        v7 = (v6 << 3) ^ (v7 >> 2);
                        v8 = ~v8 | v7;
                        break;
                    case 2: /* Mixed type path */
                        f4 = helper_float_op(f4, f5, f6);
                        v9 = helper_int_op(v9, v0, v1);
                        d3 = (double)f4 * (double)v9;
                        break;
                    case 3: /* Memory intensive path */
                        for (int j = 0; j < 4; j++) {
                            arr[(inner + j) % 64] = arr[(middle + j) % 64] + 
                                                   arr[(outer + j) % 64];
                        }
                        asm volatile("" ::: "memory");
                        break;
                }
                
                /* More mixed dependencies */
                f5 = f6 * f7 - f8;
                v1 = (int)f5 ^ v2;
                d4 = d5 / (d6 + 1.0);
                v2 = v3 + (int)d4;
                
                /* Function calls with scheduling side effects */
                if (inner % 3 == 0) {
                    f6 = helper_float_op(f6, f7, f8);
                } else if (inner % 3 == 1) {
                    v3 = helper_int_op(v3, v4, v5);
                } else {
                    d5 = helper_double_op(d5, d6, v6);
                }
                
                /* Complex expression with many live variables */
                v4 = ((v5 * v6) + (v7 / (v8 + 1))) | 
                     ((int)f7 & (int)f8) ^ 
                     (arr[v9 % 64] << 2);
                
                /* Update checksum with all live variables */
                checksum ^= (uint64_t)v0;
                checksum ^= (uint64_t)v1 << 8;
                checksum ^= (uint64_t)v2 << 16;
                checksum ^= (uint64_t)(f0 * 1000.0f);
                checksum ^= (uint64_t)(d0 * 1000.0);
            }
            
            /* Additional operations between loop levels */
            f7 = helper_float_op(f7, f8, f9);
            v5 = helper_int_op(v5, v6, v7);
            d6 = helper_double_op(d6, d7, v8);
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Update array based on outer loop */
        for (int i = 0; i < 8; i++) {
            arr[(outer + i) % 64] += i;
        }
    }
    
    /* Final accumulation from all variables */
    checksum ^= (uint64_t)v0;
    checksum ^= (uint64_t)v1 << 1;
    checksum ^= (uint64_t)v2 << 2;
    checksum ^= (uint64_t)v3 << 3;
    checksum ^= (uint64_t)v4 << 4;
    checksum ^= (uint64_t)v5 << 5;
    checksum ^= (uint64_t)v6 << 6;
    checksum ^= (uint64_t)v7 << 7;
    checksum ^= (uint64_t)v8 << 8;
    checksum ^= (uint64_t)v9 << 9;
    
    checksum ^= (uint64_t)(f0 * 100.0f);
    checksum ^= (uint64_t)(f1 * 200.0f);
    checksum ^= (uint64_t)(f2 * 300.0f);
    checksum ^= (uint64_t)(f3 * 400.0f);
    checksum ^= (uint64_t)(f4 * 500.0f);
    
    checksum ^= (uint64_t)(d0 * 1000.0);
    checksum ^= (uint64_t)(d1 * 2000.0);
    checksum ^= (uint64_t)(d2 * 3000.0);
    checksum ^= (uint64_t)(d3 * 4000.0);
    
    /* Final array checksum */
    for (int i = 0; i < 64; i++) {
        checksum ^= (uint64_t)arr[i] << (i % 16);
    }
    
    return checksum;
}

int main(void) {
    uint64_t result = complex_scheduling_function();
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    return 0;
}
