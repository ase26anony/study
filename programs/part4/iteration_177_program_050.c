#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_op(float a, float b, float c) {
    volatile float barrier = a + b;
    asm volatile("" ::: "memory");
    return (a * b) + (c / (barrier + 1.0f));
}

__attribute__((noinline))
int helper_int_op(int a, int b, int c) {
    volatile int barrier = a ^ b;
    asm volatile("" ::: "memory");
    return (a * b) + (c & barrier);
}

__attribute__((noinline))
double helper_mixed_op(int a, float b, double c) {
    volatile double barrier = (double)a + (double)b;
    asm volatile("" ::: "memory");
    return c * barrier + (double)(a % 256);
}

/* Complex function with high register pressure and mixed operations */
__attribute__((noinline))
uint64_t complex_scheduling_function(volatile int outer_iterations) {
    /* High register pressure: 30+ local variables of various types */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8;
    int *ptr1, *ptr2;
    float *fptr1, *fptr2;
    volatile int mem_array[64];
    volatile float f_array[64];
    
    /* Initialize variables to create dependencies */
    v1 = outer_iterations;
    v2 = v1 * 2;
    v3 = v2 + 1;
    v4 = v3 ^ 0x55AA55AA;
    v5 = v4 % 17;
    v6 = v5 << 3;
    v7 = v6 | 0xFF;
    v8 = v7 - v1;
    v9 = v8 / (v2 + 1);
    v10 = v9 & 0x0F0F0F0F;
    
    f1 = (float)v1 * 0.1f;
    f2 = f1 + 3.14159f;
    f3 = f2 * f1;
    f4 = f3 / (f2 + 1.0f);
    f5 = f4 - f1;
    f6 = f5 * 2.0f;
    f7 = f6 + helper_float_op(f1, f2, f3);
    f8 = f7 * 0.5f;
    f9 = f8 - f4;
    f10 = f9 / (f5 + 0.001f);
    
    d1 = (double)v2 * 0.01;
    d2 = d1 + 2.71828;
    d3 = d2 * d1;
    d4 = d3 / (d2 + 1.0);
    d5 = d4 - d1;
    d6 = d5 * 3.0;
    d7 = d6 + helper_mixed_op(v3, f3, d3);
    d8 = d7 * 0.25;
    
    /* Initialize memory arrays */
    for (int i = 0; i < 64; i++) {
        mem_array[i] = i * v1;
        f_array[i] = (float)i * f1;
    }
    
    ptr1 = (int*)mem_array;
    ptr2 = ptr1 + 32;
    fptr1 = (float*)f_array;
    fptr2 = fptr1 + 32;
    
    volatile int loop_counter = 0;
    uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    while (loop_counter < outer_iterations) {
        /* Nested loops with variable bounds */
        volatile int inner_limit = (loop_counter % 16) + 4;
        
        /* First nested loop - integer operations */
        for (int i = 0; i < inner_limit; i++) {
            /* Mixed operation dependency chain */
            v1 = v10 + i;
            asm volatile("" ::: "memory");
            v2 = v1 * v9;
            v3 = v2 ^ v8;
            f1 = (float)v3 * 0.01f;
            asm volatile("" ::: "memory");
            f2 = f1 + f10;
            d1 = (double)f2 * 0.001;
            v4 = (int)d1 + v7;
            
            /* Memory access pattern */
            mem_array[i] = v4;
            asm volatile("" ::: "memory");
            v5 = mem_array[(i + 1) % 64];
            f_array[i] = (float)v5 * f9;
            f3 = f_array[(i + 2) % 64];
            
            /* Call helper with cross-type dependencies */
            v6 = helper_int_op(v4, v5, (int)f3);
            asm volatile("" ::: "memory");
        }
        
        /* Second nested loop - floating point intensive */
        volatile int fp_limit = (loop_counter % 8) + 8;
        for (int j = 0; j < fp_limit; j++) {
            /* FP dependency chain */
            f4 = f3 * 1.1f + (float)j;
            asm volatile("" ::: "memory");
            f5 = f4 / (f2 + 0.1f);
            d2 = (double)f5 + d8;
            d3 = d2 * 0.9;
            asm volatile("" ::: "memory");
            f6 = (float)d3 - f1;
            
            /* Mixed operations */
            v7 = (int)f6 ^ v6;
            v8 = v7 * v4;
            f7 = helper_float_op(f4, f5, f6);
            asm volatile("" ::: "memory");
            
            /* Memory store with barrier */
            f_array[(j + loop_counter) % 64] = f7;
            asm volatile("" ::: "memory");
        }
        
        /* Conditional execution paths with different operation mixes */
        switch (loop_counter % 5) {
            case 0:
                /* FP math path */
                for (int k = 0; k < 4; k++) {
                    d4 = d3 * 1.01 + (double)k;
                    f8 = (float)d4 * 0.5f;
                    d5 = helper_mixed_op(v8, f8, d4);
                    asm volatile("" ::: "memory");
                }
                v9 = (int)(d5 * 1000.0);
                break;
                
            case 1:
                /* Integer bit manipulation path */
                v9 = v8;
                for (int k = 0; k < 6; k++) {
                    v9 = (v9 << 3) | (v9 >> 29);
                    v9 ^= 0xDEADBEEF;
                    asm volatile("" ::: "memory");
                    v9 = helper_int_op(v9, k, v7);
                }
                break;
                
            case 2:
                /* Mixed operations path */
                v9 = v8 + (int)f7;
                f9 = helper_float_op((float)v9, f6, f5);
                d6 = helper_mixed_op(v9, f9, d3);
                v9 = (int)d6;
                asm volatile("" ::: "memory");
                break;
                
            case 3:
                /* Memory intensive path */
                for (int k = 0; k < 8; k++) {
                    int idx = (k + loop_counter) % 64;
                    mem_array[idx] = mem_array[(idx + 1) % 64] + k;
                    asm volatile("" ::: "memory");
                    f_array[idx] = f_array[(idx + 2) % 64] * 1.1f;
                    v9 = mem_array[idx] ^ (int)f_array[idx];
                }
                break;
                
            default:
                /* Dependency chain path */
                v9 = v8;
                for (int k = 0; k < 3; k++) {
                    v9 = v9 * 3 + 1;
                    f10 = (float)v9 * 0.01f;
                    d7 = (double)f10 + d2;
                    v9 = (int)d7;
                    asm volatile("" ::: "memory");
                }
                break;
        }
        
        /* Accumulate checksum from all variables */
        checksum ^= (uint64_t)v1;
        checksum ^= (uint64_t)v2 << 8;
        checksum ^= (uint64_t)v3 << 16;
        checksum ^= (uint64_t)v4 << 24;
        checksum ^= (uint64_t)v5 << 32;
        checksum ^= (uint64_t)v6 << 40;
        checksum ^= (uint64_t)(*(uint32_t*)&f7);
        checksum ^= (uint64_t)(*(uint32_t*)&f8) << 8;
        checksum ^= (uint64_t)(*(uint64_t*)&d3);
        checksum ^= (uint64_t)(*(uint64_t*)&d4) << 16;
        
        loop_counter++;
        asm volatile("" ::: "memory");
    }
    
    return checksum;
}

int main() {
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: 0x%016llX\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
