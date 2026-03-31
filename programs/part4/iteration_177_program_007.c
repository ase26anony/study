#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_ops(float a, float b, float c, float d) {
    volatile float barrier = 0.0f;
    float t1 = a * b + barrier;
    float t2 = c / d - barrier;
    asm volatile("" ::: "memory");
    return t1 * t2 + a - b + c - d;
}

__attribute__((noinline))
int helper_int_ops(int a, int b, int c, int d, int e) {
    volatile int barrier = 0;
    int t1 = (a & b) | (c ^ d);
    int t2 = (a << 3) + (b >> 2) - (c * d);
    asm volatile("" ::: "memory");
    return (t1 + t2) * e - barrier;
}

__attribute__((noinline))
double helper_mixed_ops(int a, float b, double c, int d) {
    volatile double barrier = 0.0;
    double t1 = (double)a * (double)b + barrier;
    double t2 = c / (d + 1.0) - barrier;
    asm volatile("" ::: "memory");
    return t1 * t2 + (double)a - (double)b;
}

/* Complex function with high register pressure and mixed operations */
__attribute__((noinline))
uint64_t complex_scheduling_function(volatile int outer_iterations) {
    /* High register pressure: many local variables of different types */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8;
    int *ptr1, *ptr2;
    volatile int mem_barrier = 0;
    
    /* Initialize variables to create dependencies */
    v1 = outer_iterations;
    v2 = v1 * 2;
    v3 = v2 + 1;
    v4 = v3 ^ 0x55AA55AA;
    v5 = v4 << 3;
    v6 = v5 >> 1;
    v7 = v6 & 0x0F0F0F0F;
    v8 = v7 | 0x00FF00FF;
    v9 = v8 - v1;
    v10 = v9 * 3;
    
    f1 = (float)v1 * 0.5f;
    f2 = f1 + 1.0f;
    f3 = f2 * 2.0f;
    f4 = f3 / 3.0f;
    f5 = f4 - f1;
    f6 = f5 * f2;
    f7 = f6 / f3;
    f8 = f7 + f4;
    f9 = f8 - f5;
    f10 = f9 * f6;
    
    d1 = (double)v2 * 0.25;
    d2 = d1 + 1.0;
    d3 = d2 * 3.0;
    d4 = d3 / 2.0;
    d5 = d4 - d1;
    d6 = d5 * d2;
    d7 = d6 / d3;
    d8 = d7 + d4;
    
    /* Allocate small arrays to force memory operations */
    int arr1[16], arr2[16];
    ptr1 = arr1;
    ptr2 = arr2;
    
    volatile uint64_t checksum = 0;
    volatile int loop_counter = 0;
    
    /* Outer loop with volatile limit */
    while (loop_counter < outer_iterations) {
        /* Nested loops with variable bounds */
        int inner1 = (loop_counter % 8) + 4;  /* Variable trip count */
        volatile int inner1_vol = inner1;
        
        for (int i = 0; i < inner1_vol; i++) {
            /* Mixed operation dependency chain */
            v1 = v10 + i;
            f1 = (float)v1 * 0.123f;
            d1 = (double)f1 * 2.345;
            
            /* Memory access pattern */
            ptr1[i % 16] = v1;
            ptr2[i % 16] = ptr1[(i + 1) % 16] + i;
            
            /* Use result in next operation */
            v2 = (int)d1 + ptr2[i % 16];
            f2 = f1 * (float)v2;
            
            /* Inline assembly barrier */
            asm volatile("" ::: "memory");
            
            /* Another level of nesting with different operation mix */
            int inner2 = (i % 3) + 2;
            for (int j = 0; j < inner2; j++) {
                /* Integer bit manipulation */
                v3 = v2 ^ (j << 4);
                v4 = (v3 & 0xFF) | ((v3 >> 8) & 0xFF00);
                v5 = v4 * 7 - j;
                
                /* Floating point operations */
                f3 = f2 + (float)j * 0.01f;
                f4 = f3 * f3 - f2;
                
                /* Dependency across types */
                v6 = (int)(f4 * 100.0f) + v5;
                f5 = (float)v6 / 50.0f;
                
                /* Memory store with dependency */
                arr1[(i + j) % 16] = v6;
                arr2[(i + j) % 16] = (int)f5;
                
                asm volatile("" ::: "memory");
            }
            
            /* Conditional execution paths */
            switch (i % 4) {
                case 0:
                    /* FP math path */
                    f6 = helper_float_ops(f1, f2, f3, f4);
                    d2 = (double)f6 * d1;
                    v7 = (int)d2;
                    break;
                case 1:
                    /* Integer bit manipulation path */
                    v7 = helper_int_ops(v1, v2, v3, v4, v5);
                    v7 = (v7 << 1) | (v7 >> 31);  /* Rotate */
                    break;
                case 2:
                    /* Mixed operations path */
                    d2 = helper_mixed_ops(v1, f1, d1, v2);
                    v7 = (int)d2;
                    f6 = (float)(v7 % 100);
                    break;
                default:
                    /* Memory intensive path */
                    for (int k = 0; k < 4; k++) {
                        ptr1[k] = ptr2[k] + i + k;
                        ptr2[k] = ptr1[k] * 2 - k;
                    }
                    v7 = ptr1[i % 4] + ptr2[i % 4];
                    f6 = (float)v7;
                    break;
            }
            
            /* Create cross-iteration dependencies */
            v8 = v7 + v6;
            f7 = f6 + f5;
            d3 = d2 + d1;
            
            /* Update checksum with all values */
            checksum ^= (uint64_t)v8;
            checksum ^= (uint64_t)(*(uint32_t*)&f7);
            checksum ^= (uint64_t)(*(uint64_t*)&d3);
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Update loop variables with dependencies */
        v9 = v8 * 3 - loop_counter;
        f8 = f7 * 1.5f + (float)loop_counter;
        d4 = d3 * 0.75 + (double)loop_counter;
        
        /* Function call that forces scheduler context save/restore */
        if (loop_counter % 3 == 0) {
            v10 = helper_int_ops(v9, v8, v7, v6, v5);
        } else if (loop_counter % 3 == 1) {
            f9 = helper_float_ops(f8, f7, f6, f5);
            v10 = (int)f9;
        } else {
            d5 = helper_mixed_ops(v9, f8, d4, v8);
            v10 = (int)d5;
        }
        
        loop_counter++;
        mem_barrier = loop_counter;  /* Volatile write */
    }
    
    /* Final computation using all variables */
    int final_int = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    float final_float = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
    double final_double = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
    
    checksum ^= (uint64_t)final_int;
    checksum ^= (uint64_t)(*(uint32_t*)&final_float);
    checksum ^= (uint64_t)(*(uint64_t*)&final_double);
    
    return checksum;
}

int main() {
    volatile int iterations = 1000;  /* Prevent constant propagation */
    
    printf("Starting complex scheduling test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Result checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
