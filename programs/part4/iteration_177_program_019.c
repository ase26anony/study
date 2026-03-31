#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to force scheduler state saves/restores */
__attribute__((noinline)) 
float helper_float(float a, float b, float c) {
    volatile float barrier = a * b;
    asm volatile("" ::: "memory");
    return barrier + c * 0.5f;
}

__attribute__((noinline))
int helper_int(int a, int b, int c) {
    volatile int tmp = (a ^ b) & c;
    asm volatile("" ::: "memory");
    return tmp * 3 + 1;
}

__attribute__((noinline))
double helper_double(double a, double b, int c) {
    volatile double d = a / (b + 1.0);
    asm volatile("" ::: "memory");
    return d * c;
}

int main(void) {
    /* High register pressure: many live variables of different types */
    volatile int outer_limit = 1000; /* Prevent constant propagation */
    int i, j, k;
    
    /* Integer variables */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Floating point variables */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    /* Double precision variables */
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    /* Pointer/array variables */
    int arr[100];
    volatile int *ptr = arr;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Final checksum */
    volatile uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        volatile int inner_limit = (i % 50) + 10; /* Variable loop bounds */
        
        /* Nested loop level 1 */
        for (j = 0; j < inner_limit; j++) {
            volatile int innermost_limit = (j % 10) + 5;
            
            /* Nested loop level 2 */
            for (k = 0; k < innermost_limit; k++) {
                /* Complex conditional with different operation mixes */
                switch ((i + j + k) % 7) {
                    case 0: {
                        /* Integer arithmetic chain */
                        v1 = v2 + v3;
                        v2 = v3 * v4;
                        v3 = v4 ^ v5;
                        v4 = v5 & v6;
                        v5 = v6 | v7;
                        asm volatile("" ::: "memory");
                        
                        /* Mixed int -> float -> memory */
                        f1 = (float)v1 * 0.5f;
                        f2 = f1 + f3;
                        *ptr = (int)f2; /* Store to memory */
                        v6 = arr[k];    /* Load from memory */
                        break;
                    }
                    
                    case 1: {
                        /* Floating point intensive */
                        f3 = f4 * f5;
                        f4 = helper_float(f3, f5, f6);
                        f5 = f6 / f7;
                        f6 = f7 + f8;
                        asm volatile("" ::: "memory");
                        
                        /* FP -> int conversion */
                        v7 = (int)f3;
                        v8 = v7 * v9;
                        break;
                    }
                    
                    case 2: {
                        /* Memory access pattern */
                        v9 = arr[(i + j) % 100];
                        v10 = arr[(j + k) % 100];
                        v11 = arr[(k + i) % 100];
                        
                        /* Call helper with dependencies */
                        v12 = helper_int(v9, v10, v11);
                        asm volatile("" ::: "memory");
                        
                        /* Update array */
                        arr[(i * j + k) % 100] = v12;
                        break;
                    }
                    
                    case 3: {
                        /* Double precision operations */
                        d1 = d2 * d3;
                        d2 = helper_double(d1, d3, v13);
                        d3 = d4 / d5;
                        asm volatile("" ::: "memory");
                        
                        /* Double -> float -> int chain */
                        f7 = (float)d1;
                        v13 = (int)f7;
                        break;
                    }
                    
                    case 4: {
                        /* Bit manipulation chain */
                        v14 = (v15 << 3) | (v16 >> 2);
                        v15 = ~v14;
                        v16 = v14 ^ v15;
                        v17 = v16 & 0xFFFF;
                        asm volatile("" ::: "memory");
                        
                        /* Call helper */
                        v18 = helper_int(v14, v15, v16);
                        break;
                    }
                    
                    case 5: {
                        /* Mixed type dependency chain */
                        f8 = (float)v19 * 1.5f;
                        d4 = (double)f8 * 2.0;
                        v20 = (int)d4;
                        arr[v20 % 100] = v19;
                        v19 = arr[(v20 + 1) % 100];
                        asm volatile("" ::: "memory");
                        break;
                    }
                    
                    case 6: {
                        /* All types combined */
                        v1 = v2 + v3;
                        f1 = (float)v1;
                        d1 = (double)f1;
                        v4 = (int)d1;
                        arr[i % 100] = v4;
                        v5 = arr[j % 100];
                        f2 = helper_float(f1, (float)v5, 3.14f);
                        asm volatile("" ::: "memory");
                        break;
                    }
                }
                
                /* Periodic memory barrier */
                if ((k % 3) == 0) {
                    asm volatile("" ::: "memory");
                }
            }
            
            /* Update checksum with multiple variables */
            checksum ^= (uint64_t)v1;
            checksum ^= (uint64_t)v2 << 8;
            checksum ^= (uint64_t)(f1 * 1000);
            checksum ^= (uint64_t)(d1 * 1000) << 16;
        }
        
        /* Call helpers to force scheduler state changes */
        if ((i % 100) == 0) {
            f10 = helper_float(f1, f2, f3);
            v10 = helper_int(v1, v2, v3);
            d5 = helper_double(d1, d2, v10);
        }
        
        /* Rotate variables to keep them all live */
        int tmp_int = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = v11;
        v11 = v12; v12 = v13; v13 = v14; v14 = v15; v15 = v16;
        v16 = v17; v17 = v18; v18 = v19; v19 = v20; v20 = tmp_int;
        
        float tmp_float = f1;
        f1 = f2; f2 = f3; f3 = f4; f4 = f5; f5 = f6;
        f6 = f7; f7 = f8; f8 = f9; f9 = f10; f10 = tmp_float;
        
        double tmp_double = d1;
        d1 = d2; d2 = d3; d3 = d4; d4 = d5; d5 = tmp_double;
    }
    
    /* Final checksum calculation using all variables */
    checksum ^= (uint64_t)v1;
    checksum ^= (uint64_t)v2 << 1;
    checksum ^= (uint64_t)v3 << 2;
    checksum ^= (uint64_t)v4 << 3;
    checksum ^= (uint64_t)v5 << 4;
    checksum ^= (uint64_t)v6 << 5;
    checksum ^= (uint64_t)v7 << 6;
    checksum ^= (uint64_t)v8 << 7;
    checksum ^= (uint64_t)v9 << 8;
    checksum ^= (uint64_t)v10 << 9;
    checksum ^= (uint64_t)(f1 * 1000);
    checksum ^= (uint64_t)(f2 * 1000) << 10;
    checksum ^= (uint64_t)(d1 * 1000) << 20;
    checksum ^= (uint64_t)(d2 * 1000) << 30;
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
