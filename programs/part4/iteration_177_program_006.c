#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_ops(float a, float b, float c, float d, float e) {
    volatile float barrier = a + b;
    asm volatile("" ::: "memory");
    float t1 = barrier * c;
    float t2 = d / (e + 1.0f);
    asm volatile("" ::: "memory");
    return t1 - t2 + barrier;
}

__attribute__((noinline))
int helper_int_ops(int a, int b, int c, int d, int e, int f) {
    volatile int barrier = a ^ b;
    asm volatile("" ::: "memory");
    int t1 = barrier & c;
    int t2 = d | e;
    int t3 = (t1 + t2) * f;
    asm volatile("" ::: "memory");
    return t3 ^ barrier;
}

__attribute__((noinline))
double helper_mixed_ops(int a, float b, double c, int d) {
    volatile double barrier = (double)a + (double)b;
    asm volatile("" ::: "memory");
    double t1 = barrier * c;
    double t2 = (double)d / 7.0;
    asm volatile("" ::: "memory");
    return t1 + t2 - barrier;
}

/* Main complex function with high register pressure */
void complex_scheduling_function(volatile int outer_iterations) {
    /* Many local variables to create high register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5;
    int *ptr1, *ptr2;
    float *fptr1, *fptr2;
    volatile int mem_array[64];
    volatile float float_array[64];
    
    /* Initialize variables with different patterns */
    for (int i = 0; i < 64; i++) {
        mem_array[i] = i * 3;
        float_array[i] = i * 1.5f;
    }
    
    ptr1 = (int*)mem_array;
    ptr2 = (int*)mem_array + 32;
    fptr1 = (float*)float_array;
    fptr2 = (float*)float_array + 32;
    
    volatile int checksum = 0;
    volatile int outer_limit = outer_iterations;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Initialize many variables with inter-dependent values */
        v1 = outer * 2;
        v2 = v1 + 17;
        v3 = v2 ^ 0x55AA55AA;
        v4 = v3 - v1;
        v5 = v4 * 3;
        v6 = v5 / 2;
        v7 = v6 | 0xFF00FF00;
        v8 = v7 & 0x00FF00FF;
        v9 = v8 << 3;
        v10 = v9 >> 1;
        
        f1 = (float)v1 * 0.5f;
        f2 = f1 + 3.14f;
        f3 = f2 * f1;
        f4 = f3 / (f2 + 1.0f);
        f5 = f4 - f3;
        f6 = f5 * 2.0f;
        f7 = f6 + f4;
        f8 = f7 / 3.0f;
        f9 = f8 * f5;
        f10 = f9 - f6;
        
        d1 = (double)f1 * 1.234567;
        d2 = d1 + (double)v2;
        d3 = d2 * 0.987654;
        d4 = d3 / (d1 + 1.0);
        d5 = d4 - d2;
        
        /* Memory access pattern creating dependencies */
        asm volatile("" ::: "memory");
        mem_array[v1 & 63] = v2;
        float_array[v2 & 63] = f1;
        asm volatile("" ::: "memory");
        
        /* Nested loops with variable bounds */
        volatile int middle_limit = (outer & 7) + 3;
        for (volatile int middle = 0; middle < middle_limit; middle++) {
            /* Inner loop with data-dependent trip count */
            int inner_limit = (v3 + middle) & 15;
            for (int inner = 0; inner < inner_limit; inner++) {
                /* Mixed operation dependency chain */
                int temp_int = v4 + inner;
                float temp_float = (float)temp_int * f2;
                double temp_double = (double)temp_float + d3;
                
                /* Memory load with dependency */
                int loaded_int = mem_array[(temp_int + inner) & 63];
                float loaded_float = float_array[(inner + middle) & 63];
                
                /* Complex calculation chain */
                temp_int = loaded_int ^ temp_int;
                temp_float = loaded_float * temp_float;
                temp_double = temp_double / (temp_float + 1.0);
                
                /* Store results back with barrier */
                asm volatile("" ::: "memory");
                mem_array[inner & 63] = temp_int;
                float_array[inner & 63] = temp_float;
                asm volatile("" ::: "memory");
                
                /* Update checksum */
                checksum ^= temp_int;
                checksum += (int)temp_float;
                checksum ^= (int)(temp_double * 1000.0);
            }
            
            /* Conditional execution paths */
            switch (middle & 3) {
                case 0:
                    /* FP math intensive path */
                    f1 = helper_float_ops(f1, f2, f3, f4, f5);
                    d1 = helper_mixed_ops(v1, f1, d1, v2);
                    for (int i = 0; i < 4; i++) {
                        f1 = f1 * 1.1f - f2;
                        asm volatile("" ::: "memory");
                    }
                    break;
                    
                case 1:
                    /* Integer bit manipulation path */
                    v1 = helper_int_ops(v1, v2, v3, v4, v5, v6);
                    for (int i = 0; i < 4; i++) {
                        v1 = (v1 << 3) | (v1 >> 29);
                        v1 ^= 0xDEADBEEF;
                        asm volatile("" ::: "memory");
                    }
                    break;
                    
                case 2:
                    /* Memory intensive path */
                    for (int i = 0; i < 8; i++) {
                        int idx = (v1 + i) & 63;
                        mem_array[idx] = mem_array[(idx + 1) & 63] + i;
                        float_array[idx] = float_array[(idx + 2) & 63] * 1.01f;
                        asm volatile("" ::: "memory");
                    }
                    break;
                    
                case 3:
                    /* Mixed operations path */
                    v1 = helper_int_ops(v1, v2, v3, v4, v5, v6);
                    f1 = helper_float_ops(f1, f2, f3, f4, f5);
                    d1 = helper_mixed_ops(v1, f1, d1, v2);
                    asm volatile("" ::: "memory");
                    break;
            }
            
            /* Additional dependency chain across loop iterations */
            v2 = v1 + v2;
            f2 = f1 * f2 - f3;
            d2 = d1 + d2 * 0.99;
            
            /* Memory barrier to prevent reordering */
            asm volatile("" ::: "memory");
        }
        
        /* More complex dependency chains */
        int idx = outer & 63;
        v3 = mem_array[idx] + v3;
        f3 = float_array[idx] * f3;
        
        /* Call helper with many live variables */
        v4 = helper_int_ops(v3, v4, v5, v6, v7, v8);
        f4 = helper_float_ops(f3, f4, f5, f6, f7);
        
        /* Update checksum with all variables */
        checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
        checksum += (int)(f1 * 1000.0f);
        checksum += (int)(f2 * 1000.0f);
        checksum += (int)(d1 * 1000.0);
        checksum += (int)(d2 * 1000.0);
    }
    
    /* Final barrier and output */
    asm volatile("" ::: "memory");
    printf("Checksum: %d\n", checksum);
}

int main() {
    /* Volatile to prevent constant propagation */
    volatile int iterations = 1000;
    
    /* Call the complex scheduling function */
    complex_scheduling_function(iterations);
    
    return 0;
}
