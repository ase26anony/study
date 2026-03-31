#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float fp_helper(float a, float b, float c) {
    volatile float t1 = a * b;
    volatile float t2 = t1 + c;
    asm volatile("" ::: "memory");
    return t2 - a;
}

__attribute__((noinline))
int int_helper(int x, int y, int z) {
    volatile int r = (x ^ y) | z;
    asm volatile("" ::: "memory");
    return r & 0x7FFFFFFF;
}

__attribute__((noinline))
double mem_helper(double* arr, int idx1, int idx2) {
    volatile double d1 = arr[idx1];
    volatile double d2 = arr[idx2];
    asm volatile("" ::: "memory");
    return d1 * 0.5 + d2 * 0.5;
}

/* Main stress function */
int main(void) {
    /* High register pressure: many live variables */
    volatile int outer_limit = 1000;
    volatile int i, j, k;
    
    /* Mixed type variables */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    int v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25;
    int v26 = 26, v27 = 27, v28 = 28, v29 = 29, v30 = 30;
    
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    /* Arrays for memory operations */
    double arr[64];
    int iarr[128];
    
    /* Initialize arrays */
    for (i = 0; i < 64; i++) {
        arr[i] = i * 0.5;
    }
    for (i = 0; i < 128; i++) {
        iarr[i] = i * 3;
    }
    
    /* Volatile checksum to prevent dead code elimination */
    volatile uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        /* Nested loops with variable bounds */
        volatile int inner_limit1 = (i % 16) + 8;
        volatile int inner_limit2 = (i % 8) + 4;
        
        for (j = 0; j < inner_limit1; j++) {
            /* Mixed operation dependency chains */
            v1 = v1 + v2 * v3;
            f1 = fp_helper(f1, f2, f3);
            v4 = v4 ^ (v5 << 2);
            d1 = arr[v1 % 64] * d2 + d3;
            
            asm volatile("" ::: "memory");
            
            /* Conditional execution paths */
            switch (j % 4) {
                case 0:
                    /* FP math path */
                    f3 = f1 * f2 + f4;
                    f5 = fp_helper(f3, f4, f5);
                    v6 = int_helper(v6, v7, v8);
                    arr[v6 % 64] = d1 * 0.3;
                    break;
                case 1:
                    /* Integer bit manipulation path */
                    v9 = (v9 | v10) ^ v11;
                    v12 = v12 + (v13 << 1);
                    v14 = v14 * v15 - v16;
                    iarr[v9 % 128] = v12 + v14;
                    break;
                case 2:
                    /* Memory intensive path */
                    d2 = mem_helper(arr, v17 % 64, v18 % 64);
                    d3 = arr[v19 % 64] * d4;
                    v20 = iarr[v21 % 128] + iarr[v22 % 128];
                    f6 = fp_helper(f6, f7, f8);
                    break;
                case 3:
                    /* Mixed operations */
                    v23 = int_helper(v23, v24, v25);
                    f9 = fp_helper(f9, f10, f1);
                    d5 = arr[v26 % 64] + arr[v27 % 64];
                    v28 = v28 * v29 / (v30 + 1);
                    break;
            }
            
            asm volatile("" ::: "memory");
            
            /* Deeply nested loop */
            for (k = 0; k < inner_limit2; k++) {
                /* Interleaved dependencies */
                v2 = v1 + v3;
                f2 = f1 * 1.1f + f3;
                v5 = v4 | v6;
                d4 = d1 * 0.25 + d2;
                
                /* Function call with cross-type dependencies */
                if (k % 2 == 0) {
                    v7 = int_helper(v2, v5, v8);
                    f4 = fp_helper(f2, f3, f5);
                } else {
                    v9 = v7 * v10 + v11;
                    f7 = f4 * f6 - f8;
                }
                
                /* Memory store with computed index */
                arr[(v1 + v2 + k) % 64] = d3 + d4;
                iarr[(v3 + v4) % 128] = v5 + v6;
                
                asm volatile("" ::: "memory");
                
                /* More mixed operations */
                v12 = v7 ^ v8;
                f8 = f5 + f6 * f7;
                v13 = v9 * v10 - v11;
                d1 = arr[v12 % 64] * 0.7;
                
                /* Another function call */
                v14 = int_helper(v12, v13, v14);
                f9 = fp_helper(f7, f8, f9);
                
                /* Update checksum */
                checksum ^= (uint64_t)v1;
                checksum ^= (uint64_t)(*(uint32_t*)&f1);
                checksum ^= (uint64_t)(*(uint64_t*)&d1);
            }
            
            /* Update more variables */
            v15 = v14 + v16;
            f10 = f9 * 0.9f;
            v17 = v15 ^ v16;
            d2 = d1 * 1.1;
            
            /* Cross-type conversion and store */
            f1 = (float)v17 * 0.01f;
            v18 = (int)(f10 * 100.0f);
            arr[v18 % 64] = (double)v19;
            
            asm volatile("" ::: "memory");
        }
        
        /* Periodic state mixing */
        if (i % 100 == 0) {
            v20 = int_helper(v20, v21, v22);
            f2 = fp_helper(f2, f3, f4);
            v23 = v20 * v21 + v22;
            d3 = mem_helper(arr, v23 % 64, v24 % 64);
        }
        
        /* Update checksum with more values */
        checksum ^= (uint64_t)v24;
        checksum ^= (uint64_t)v25;
        checksum ^= (uint64_t)(*(uint32_t*)&f2);
        checksum ^= (uint64_t)(*(uint64_t*)&d2);
    }
    
    /* Final mixing */
    v26 = int_helper(v26, v27, v28);
    f3 = fp_helper(f3, f4, f5);
    v29 = v26 + v27 * v28;
    d4 = mem_helper(arr, v29 % 64, v30 % 64);
    
    /* Final checksum calculation */
    checksum ^= (uint64_t)v30;
    checksum ^= (uint64_t)(*(uint32_t*)&f3);
    checksum ^= (uint64_t)(*(uint64_t*)&d4);
    
    /* Print to prevent optimization */
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
