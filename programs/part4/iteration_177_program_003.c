#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_ops(float a, float b, float c, float d, float e) {
    volatile float v1 = a * b + c;
    float v2 = d / (e + 1.0f);
    asm volatile("" ::: "memory");
    return v1 - v2 * 0.5f;
}

__attribute__((noinline))
int helper_int_ops(int a, int b, int c, int d, int e) {
    volatile int v1 = (a ^ b) & (c | d);
    int v2 = (e << 3) | (e >> 5);
    asm volatile("" ::: "memory");
    return v1 * v2 + (d % 7);
}

__attribute__((noinline))
double helper_mixed_ops(int a, float b, double c, int d) {
    volatile double v1 = (double)a * (double)b + c;
    double v2 = (double)d / 3.14159;
    asm volatile("" ::: "memory");
    return v1 * v2 - c / 2.0;
}

/* Main complex function with high register pressure */
void complex_scheduling_function(volatile int outer_iterations) {
    /* High register pressure: 30+ local variables of mixed types */
    volatile int v0 = 1;
    int v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6, v6 = 7, v7 = 8, v8 = 9, v9 = 10;
    int v10 = 11, v11 = 12, v12 = 13, v13 = 14, v14 = 15, v15 = 16;
    int v16 = 17, v17 = 18, v18 = 19, v19 = 20, v20 = 21;
    
    float f0 = 1.1f, f1 = 2.2f, f2 = 3.3f, f3 = 4.4f, f4 = 5.5f;
    float f5 = 6.6f, f6 = 7.7f, f7 = 8.8f, f8 = 9.9f, f9 = 10.10f;
    
    double d0 = 1.01, d1 = 2.02, d2 = 3.03, d3 = 4.04, d4 = 5.05;
    double d5 = 6.06, d6 = 7.07, d7 = 8.08, d8 = 9.09, d9 = 10.10;
    
    /* Arrays for memory access patterns */
    int arr_int[32];
    float arr_float[32];
    double arr_double[32];
    
    volatile int checksum = 0;
    volatile int outer_counter = 0;
    
    /* Outer loop with volatile limit */
    while (outer_counter < outer_iterations) {
        /* Initialize arrays with mixed patterns */
        for (int i = 0; i < 32; i++) {
            arr_int[i] = i * outer_counter;
            arr_float[i] = (float)i * 0.5f + outer_counter;
            arr_double[i] = (double)i * 0.25 + outer_counter * 0.1;
        }
        
        /* Middle loop with variable bounds */
        volatile int middle_limit = (outer_counter % 8) + 3;
        for (int m = 0; m < middle_limit; m++) {
            /* Inner loop with complex dependency chains */
            volatile int inner_limit = (m % 4) + 2;
            for (int n = 0; n < inner_limit; n++) {
                /* Mixed operation dependency chains */
                v1 = v0 + v2 * v3 - v4;
                f0 = (float)v1 * f1 + f2 / f3;
                arr_float[n] = f0 * 2.0f;
                
                /* Memory barrier to prevent reordering */
                asm volatile("" ::: "memory");
                
                /* Integer -> Float -> Memory -> Integer chain */
                v5 = (int)arr_float[n] ^ v6;
                d0 = (double)v5 * d1 - d2;
                arr_double[m] = d0 + d3;
                v7 = (int)arr_double[m] | v8;
                
                /* Call helper functions with dependencies */
                f3 = helper_float_ops(f0, f1, f2, f3, f4);
                v9 = helper_int_ops(v5, v6, v7, v8, v9);
                d4 = helper_mixed_ops(v9, f3, d0, v10);
                
                /* Conditional execution paths */
                switch (n % 4) {
                    case 0:
                        /* FP math branch */
                        f4 = f3 * f2 - f1 / f0;
                        d5 = d4 * 1.41421356 + d3;
                        v10 = (int)(f4 * d5) ^ v11;
                        break;
                    case 1:
                        /* Integer bit manipulation branch */
                        v11 = (v10 << (n % 8)) | (v10 >> (8 - (n % 8)));
                        v12 = v11 ^ v10 & v9 | v8;
                        f5 = (float)v12 * 0.01f;
                        break;
                    case 2:
                        /* Memory intensive branch */
                        for (int k = 0; k < 4; k++) {
                            arr_int[(m * 4 + k) % 32] = v10 + v11 * k;
                            arr_float[(n * 2 + k) % 32] = f4 + f5 * (float)k;
                        }
                        v13 = arr_int[m % 32] + arr_int[n % 32];
                        f6 = arr_float[m % 32] * arr_float[n % 32];
                        break;
                    case 3:
                        /* Mixed operations branch */
                        v14 = v13 * v12 - v11 / (v10 + 1);
                        f7 = helper_float_ops(f4, f5, f6, f7, f8);
                        d6 = helper_mixed_ops(v14, f7, d5, v15);
                        arr_double[(m + n) % 32] = d6;
                        break;
                }
                
                /* More dependency chains */
                v15 = v14 + (int)(f5 * 100.0f);
                f8 = (float)v15 * 0.01f + f6;
                d7 = (double)v16 * 0.001 + d6;
                v16 = (int)(d7 * 1000.0) & v15;
                
                /* Another memory barrier */
                asm volatile("" ::: "memory");
                
                /* Update checksum with mixed operations */
                checksum ^= v15 + (int)f8 + (int)d7;
            }
            
            /* Inter-loop dependencies */
            v17 = v16 * m + v15;
            f9 = f8 * (float)m + f7;
            d8 = d7 * (double)m + d6;
            
            /* Call helper across loop iterations */
            if (m % 2 == 0) {
                v18 = helper_int_ops(v17, v16, v15, v14, v13);
                f0 = helper_float_ops(f9, f8, f7, f6, f5);
            }
        }
        
        /* Post-loop processing */
        v19 = v18 + v17 * outer_counter;
        d9 = d8 * (double)outer_counter + d7;
        f1 = f0 * (float)outer_counter + f9;
        
        /* Final complex expression */
        v20 = (int)((d9 * 1000.0) + (double)(v19 * 100) + (double)f1 * 10.0);
        checksum += v20;
        
        outer_counter++;
        
        /* Prevent loop unrolling with volatile */
        asm volatile("" ::: "memory");
    }
    
    /* Ensure all code is used */
    printf("Checksum: %d\n", checksum);
    printf("Final values: v20=%d, f1=%f, d9=%f\n", v20, f1, d9);
}

int main() {
    /* Volatile to prevent constant propagation */
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling test...\n");
    
    /* Run the complex function */
    complex_scheduling_function(iterations);
    
    printf("Test completed.\n");
    
    return 0;
}
