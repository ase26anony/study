#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_op(float a, float b, float c) {
    volatile float barrier = a * b;
    asm volatile("" ::: "memory");
    return (barrier + c) * 0.5f;
}

__attribute__((noinline))
int helper_int_op(int a, int b, int c) {
    volatile int tmp = (a ^ b) & c;
    asm volatile("" ::: "memory");
    return tmp + (a >> 3) - (b << 2);
}

__attribute__((noinline))
double helper_mixed_op(int a, float b, double c) {
    volatile double d1 = (double)a * 1.5;
    volatile double d2 = (double)b * 2.0;
    asm volatile("" ::: "memory");
    return d1 + d2 + c * 0.75;
}

/* Complex function with high register pressure and mixed operations */
__attribute__((noinline))
uint64_t complex_scheduling_function(volatile int outer_iterations) {
    /* High register pressure: many variables of different types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    volatile int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    
    /* Arrays for memory operations */
    volatile int arr_int[32];
    volatile float arr_float[32];
    volatile double arr_double[32];
    
    /* Result accumulator */
    volatile uint64_t checksum = 0xDEADBEEF;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        /* Initialize arrays with pattern */
        for (int i = 0; i < 32; i++) {
            arr_int[i] = i + outer;
            arr_float[i] = (float)i * 1.5f + outer;
            arr_double[i] = (double)i * 2.5 + outer;
        }
        
        /* Nested loop with variable bounds */
        volatile int inner_limit = (outer % 16) + 8;
        for (volatile int mid = 0; mid < inner_limit; mid++) {
            /* Innermost loop with data-dependent bounds */
            int inner_inner = (mid * 3) % 16 + 4;
            for (int inner = 0; inner < inner_inner; inner++) {
                /* Mixed operation dependency chain */
                v1 = v2 + v3 * inner;
                f1 = (float)v1 * f2;
                asm volatile("" ::: "memory");
                
                /* Memory operations interleaved with computation */
                arr_int[inner] = v1 + arr_int[mid];
                f3 = arr_float[inner] * f1;
                
                /* Call helper functions creating scheduling boundaries */
                if (inner % 3 == 0) {
                    f4 = helper_float_op(f1, f2, f3);
                    v4 = helper_int_op(v1, v2, v3);
                } else if (inner % 3 == 1) {
                    d1 = helper_mixed_op(v1, f1, d1);
                    v5 = v4 ^ v6;
                }
                
                /* Complex conditional execution paths */
                switch (inner % 5) {
                    case 0:
                        /* FP math branch */
                        f5 = f4 * f6 + f7;
                        d2 = d1 * 1.234 + d3;
                        v6 = (int)(f5 * 100.0f);
                        break;
                    case 1:
                        /* Integer bit manipulation branch */
                        v7 = (v5 << 3) | (v6 >> 2);
                        v8 = v7 ^ 0xAAAAAAAA;
                        v9 = ~v8 & 0x55555555;
                        break;
                    case 2:
                        /* Memory intensive branch */
                        for (int j = 0; j < 4; j++) {
                            arr_int[(inner + j) % 32] = arr_int[(mid + j) % 32] + v1;
                            arr_float[(inner + j) % 32] = arr_float[(mid + j) % 32] * f2;
                        }
                        break;
                    case 3:
                        /* Mixed operations with barriers */
                        v10 = v9 * v8 - v7;
                        f6 = (float)v10 / 7.0f;
                        asm volatile("" ::: "memory");
                        d3 = (double)v10 * 0.12345;
                        f7 = f6 * 2.0f;
                        break;
                    case 4:
                        /* Another helper call */
                        d4 = helper_mixed_op(v10, f6, d3);
                        v11 = helper_int_op(v9, v10, v11);
                        break;
                }
                
                /* More dependency chains */
                v12 = v11 + arr_int[inner % 16];
                f8 = f7 * arr_float[inner % 16];
                d5 = d4 + arr_double[inner % 16];
                
                /* Update checksum with all live variables */
                checksum ^= (uint64_t)v12;
                checksum ^= *(uint64_t*)&f8;
                checksum ^= *(uint64_t*)&d5;
                checksum += (uint64_t)(f8 * 1000.0f);
            }
            
            /* Additional operations between inner loops */
            v13 = v12 * v11 - v10;
            f9 = helper_float_op(f8, f7, f6);
            d6 = d5 * 0.98765 + d4;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            
            /* Update array elements */
            arr_int[mid % 32] = v13;
            arr_float[mid % 32] = f9;
            arr_double[mid % 32] = d6;
        }
        
        /* Final accumulation per outer iteration */
        for (int i = 0; i < 16; i++) {
            v14 += arr_int[i];
            f10 += arr_float[i];
            d7 += arr_double[i];
        }
        
        checksum ^= (uint64_t)v14;
        checksum ^= *(uint64_t*)&f10;
        checksum ^= *(uint64_t*)&d7;
    }
    
    /* Final mixing of all variables */
    v15 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    v16 = v11 + v12 + v13 + v14 + v15;
    
    f1 = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
    d1 = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    
    checksum ^= (uint64_t)v16;
    checksum ^= *(uint64_t*)&f1;
    checksum ^= *(uint64_t*)&d1;
    
    return checksum;
}

int main() {
    /* Volatile to prevent optimization */
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    /* Call the complex function */
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: 0x%016llX\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
