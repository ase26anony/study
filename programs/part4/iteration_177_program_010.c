#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_op(float a, float b, float c) {
    volatile float barrier = 0.0f;
    asm volatile("" : "+m" (barrier));
    float result = (a * b) + (c / 2.0f);
    result = result * result - (a + b + c);
    asm volatile("" ::: "memory");
    return result;
}

__attribute__((noinline))
int helper_int_op(int a, int b, int c, int d) {
    volatile int barrier = 0;
    asm volatile("" : "+m" (barrier));
    int result = (a & b) | (c ^ d);
    result = (result << 3) | (result >> 29);
    result = result * 1103515245 + 12345;
    asm volatile("" ::: "memory");
    return result;
}

__attribute__((noinline))
double helper_mixed_op(int a, float b, double c) {
    volatile double barrier = 0.0;
    asm volatile("" : "+m" (barrier));
    double result = (double)a * 1.5 + (double)b * 2.0 + c * 0.75;
    result = result / (1.0 + (double)(a & 0xFF));
    asm volatile("" ::: "memory");
    return result;
}

/* Main complex function with high register pressure */
void complex_scheduling_function(volatile int outer_iterations) {
    /* High register pressure: 30+ local variables of mixed types */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8;
    int *p1, *p2, *p3;
    float *fp1, *fp2;
    volatile int checksum = 0;
    
    /* Initialize variables to create dependencies */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04;
    d5 = 5.05; d6 = 6.06; d7 = 7.07; d8 = 8.08;
    
    /* Allocate small arrays to create pointer dependencies */
    int arr1[16], arr2[16];
    float farr1[16], farr2[16];
    p1 = arr1; p2 = arr2; p3 = &v1;
    fp1 = farr1; fp2 = farr2;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        /* Mixed operation dependency chains */
        v1 = v2 + v3 * v4 - v5;
        f1 = (float)v1 * f2 + f3 / f4;
        asm volatile("" ::: "memory");  /* Barrier */
        
        /* Nested loops with variable bounds */
        volatile int middle_limit = (outer & 7) + 3;
        for (int middle = 0; middle < middle_limit; middle++) {
            volatile int inner_limit = (middle * 2 + 1) & 7;
            
            /* Inner loop with mixed operations */
            for (int inner = 0; inner < inner_limit; inner++) {
                /* Conditional execution paths */
                switch ((inner + middle + outer) & 3) {
                    case 0:  /* FP math path */
                        f5 = helper_float_op(f1, f2, f3);
                        d1 = helper_mixed_op(v1, f5, d2);
                        farr1[inner] = (float)d1 + f4;
                        v6 = (int)farr1[inner] * v7;
                        break;
                        
                    case 1:  /* Integer bit manipulation path */
                        v8 = helper_int_op(v2, v3, v4, v5);
                        v9 = (v8 << 2) | (v8 >> 30);
                        v10 = v9 ^ v6 ^ v7;
                        arr1[middle] = v10 + inner;
                        f6 = (float)arr1[middle] * 0.5f;
                        break;
                        
                    case 2:  /* Memory access intensive path */
                        arr2[inner] = v1 + v2 * v3;
                        farr2[middle] = f1 * f2 - f3;
                        v1 = arr2[inner] ^ arr1[middle];
                        f2 = farr2[middle] + farr1[inner];
                        d2 = (double)v1 * 0.25 + (double)f2;
                        break;
                        
                    case 3:  /* Mixed type conversion path */
                        d3 = (double)v3 * 1.5 + (double)f3 * 2.0;
                        v4 = (int)d3 * v5;
                        f4 = helper_float_op((float)v4, f5, f6);
                        d4 = helper_mixed_op(v6, f4, d3);
                        v7 = (int)d4 | v8;
                        break;
                }
                
                /* Additional dependency chain across iterations */
                v2 = v1 + v3;
                f3 = f2 * f1 - f4;
                d5 = (double)v2 * 0.33 + (double)f3 * 0.67;
                asm volatile("" ::: "memory");  /* Barrier */
                
                /* More mixed operations */
                v3 = v2 * v4 - v5;
                f7 = (float)v3 + f5 * f6;
                arr1[(inner + 1) & 15] = v3 + (int)f7;
                farr1[(middle + 1) & 15] = f7 * 0.9f;
            }
            
            /* Function call with scheduling side effects */
            if (middle & 1) {
                v5 = helper_int_op(v1, v2, v3, v4);
                f8 = helper_float_op(f1, f2, f3);
            } else {
                d6 = helper_mixed_op(v5, f8, d5);
                v6 = (int)d6 + v7;
            }
            
            /* Complex dependency web */
            v7 = v5 ^ v6;
            f9 = f8 * 1.1f + f7;
            d7 = (double)v7 * 0.5 + d6;
            arr2[middle & 15] = v7 + (int)f9;
        }
        
        /* Accumulate into checksum with volatile access */
        checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5;
        checksum ^= v6 ^ v7 ^ v8 ^ v9 ^ v10;
        checksum ^= (int)f1 ^ (int)f2 ^ (int)f3 ^ (int)f4;
        checksum ^= (int)f5 ^ (int)f6 ^ (int)f7 ^ (int)f8;
        checksum ^= (int)f9 ^ (int)f10;
        checksum ^= (int)d1 ^ (int)d2 ^ (int)d3 ^ (int)d4;
        checksum ^= (int)d5 ^ (int)d6 ^ (int)d7 ^ (int)d8;
        
        /* Additional barriers to prevent optimization */
        asm volatile("" ::: "memory");
    }
    
    /* Final volatile store to ensure all operations complete */
    volatile int final_result = checksum;
    printf("Checksum: %d\n", final_result);
}

int main() {
    /* Volatile iteration count to prevent constant propagation */
    volatile int iterations = 1000;
    
    /* Call the complex scheduling function */
    complex_scheduling_function(iterations);
    
    return 0;
}
