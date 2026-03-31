#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_ops(float a, float b, float c, float d) {
    volatile float v1 = a * b;
    volatile float v2 = c / d;
    asm volatile("" ::: "memory");
    return v1 + v2 - (a * c) + (b * d);
}

__attribute__((noinline))
static int helper_int_ops(int a, int b, int c, int d) {
    volatile int v1 = a ^ b;
    volatile int v2 = c | d;
    volatile int v3 = (a & b) << 2;
    asm volatile("" ::: "memory");
    return (v1 * v2) + (v3 >> 1) - (a % (b + 1));
}

__attribute__((noinline))
static double helper_mixed_ops(int a, float b, double c, int d) {
    volatile double v1 = (double)a * (double)b;
    volatile double v2 = c / (d + 1.0);
    asm volatile("" ::: "memory");
    return v1 + v2 + (a * b * c * d);
}

/* Complex main function with high register pressure */
int main(void) {
    /* High register pressure: many live variables of different types */
    volatile int outer_limit = 1000;
    volatile int inner1_limit = 50;
    volatile int inner2_limit = 20;
    
    /* Many scalar variables to pressure register allocator */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20, v21 = 21, v22 = 22;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    int *mem_ptr1, *mem_ptr2;
    float *fmem_ptr1, *fmem_ptr2;
    
    /* Allocate memory for memory access patterns */
    mem_ptr1 = (int*)malloc(1024 * sizeof(int));
    mem_ptr2 = (int*)malloc(1024 * sizeof(int));
    fmem_ptr1 = (float*)malloc(1024 * sizeof(float));
    fmem_ptr2 = (float*)malloc(1024 * sizeof(float));
    
    /* Initialize memory */
    for (int i = 0; i < 1024; i++) {
        mem_ptr1[i] = i;
        mem_ptr2[i] = 1024 - i;
        fmem_ptr1[i] = i * 0.1f;
        fmem_ptr2[i] = (1024 - i) * 0.1f;
    }
    
    volatile uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Nested loop 1 - variable bounds based on outer index */
        for (int i = 0; i < (inner1_limit + (outer % 10)); i++) {
            /* Mixed operation dependency chain */
            v1 = v2 + v3 * v4;
            f1 = (float)v1 * f2 + f3;
            
            /* Memory access creating dependencies */
            mem_ptr1[v1 % 1024] = v2 + v3;
            v5 = mem_ptr2[v4 % 1024] ^ v1;
            
            /* Floating point operation chain */
            f4 = f1 * f2 - f3 / f5;
            d1 = (double)f4 + d2 * d3;
            
            /* Inline assembly barrier */
            asm volatile("" ::: "memory");
            
            /* Call helper functions creating scheduling boundaries */
            f6 = helper_float_ops(f1, f2, f3, f4);
            v6 = helper_int_ops(v1, v2, v3, v4);
            d2 = helper_mixed_ops(v5, f5, d1, v6);
            
            /* More mixed operations */
            v7 = (v5 << 3) | (v6 >> 2);
            f7 = f6 * 1.5f - f5;
            v8 = (int)(f7 * 10.0f) + v7;
            
            /* Conditional execution paths */
            switch (i % 5) {
                case 0:
                    /* FP math path */
                    f8 = f7 * f6 / f5 + f4;
                    v9 = (int)(f8 * 100.0f);
                    d3 = d2 * 1.01 + (double)f8;
                    break;
                case 1:
                    /* Integer bit manipulation path */
                    v9 = (v7 ^ v8) & (v6 | v5);
                    v9 = (v9 << 1) | (v9 >> 31);
                    f8 = (float)v9 * 0.01f;
                    break;
                case 2:
                    /* Memory intensive path */
                    for (int j = 0; j < 5; j++) {
                        int idx = (v7 + j) % 1024;
                        mem_ptr1[idx] = mem_ptr2[idx] + v8;
                        fmem_ptr1[idx] = fmem_ptr2[idx] * f7;
                    }
                    v9 = mem_ptr1[v8 % 1024];
                    f8 = fmem_ptr1[v7 % 1024];
                    break;
                case 3:
                    /* Mixed computation path */
                    v9 = helper_int_ops(v7, v8, v5, v6);
                    f8 = helper_float_ops(f7, f6, f5, f4);
                    d3 = helper_mixed_ops(v9, f8, d2, v8);
                    break;
                default:
                    /* Simple arithmetic path */
                    v9 = v7 + v8 - v5 * v6;
                    f8 = f7 + f6 - f5 * f4;
                    d3 = d2 + d1;
            }
            
            /* Nested loop 2 - depends on outer and inner indices */
            for (int k = 0; k < (inner2_limit + (i % 3)); k++) {
                /* Complex dependency chain within innermost loop */
                v10 = v9 * k + v8;
                f9 = f8 * (float)k + f7;
                
                /* Memory load/store with dependency */
                int mem_idx = (v10 + k) % 1024;
                v11 = mem_ptr1[mem_idx] + v10;
                mem_ptr2[mem_idx] = v11 ^ k;
                
                /* Floating point computation */
                f10 = f9 * 2.0f - f8 / (float)(k + 1);
                fmem_ptr1[mem_idx] = f10;
                
                /* Integer computation using FP result */
                v12 = (int)(f10 * 100.0f) + v11;
                
                /* Another barrier */
                asm volatile("" ::: "memory");
                
                /* More operations to increase pressure */
                v13 = v12 & 0xFF;
                v14 = v13 | 0x80;
                v15 = v14 << 2;
                v16 = v15 >> 1;
                
                f5 = f10 * 0.5f;
                f6 = f5 + 1.0f;
                
                /* Update checksum */
                checksum ^= (uint64_t)v16;
                checksum += (uint64_t)(f6 * 1000.0f);
            }
            
            /* Update outer variables to maintain dependencies */
            v2 = v9 + v10;
            v3 = v11 ^ v12;
            v4 = v13 | v14;
            f2 = f8 + f9;
            f3 = f10 * 0.9f;
            
            /* Accumulate to checksum */
            checksum += (uint64_t)v2 * (uint64_t)v3;
            checksum ^= (uint64_t)(f2 * 100.0f);
        }
        
        /* Additional operations between outer loop iterations */
        v17 = v1 + v2 + v3 + v4;
        v18 = v5 * v6 - v7;
        v19 = v8 ^ v9 | v10;
        v20 = v11 & v12 << v13;
        
        f4 = f1 * f2 + f3;
        d4 = d1 + d2 + d3;
        
        /* Memory operations spanning iterations */
        for (int m = 0; m < 10; m++) {
            int idx = (outer * 10 + m) % 1024;
            mem_ptr1[idx] = mem_ptr1[idx] + v17;
            fmem_ptr1[idx] = fmem_ptr1[idx] * f4;
        }
        
        /* Final checksum update per outer iteration */
        checksum += (uint64_t)v17 * (uint64_t)v18;
        checksum ^= (uint64_t)(f4 * 1000.0f);
        checksum += (uint64_t)d4;
    }
    
    /* Additional cleanup and final computation */
    v21 = 0;
    for (int i = 0; i < 1024; i++) {
        v21 ^= mem_ptr1[i];
        v21 += (int)fmem_ptr1[i];
    }
    
    checksum += (uint64_t)v21;
    
    /* Print final checksum to prevent dead code elimination */
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    /* Free allocated memory */
    free(mem_ptr1);
    free(mem_ptr2);
    free(fmem_ptr1);
    free(fmem_ptr2);
    
    return 0;
}
