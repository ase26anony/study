#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_op(float a, float b, float c) {
    volatile float barrier = a * b;
    asm volatile("" ::: "memory");
    return barrier + c * 0.5f;
}

__attribute__((noinline))
int helper_int_op(int a, int b, int c) {
    volatile int barrier = a ^ b;
    asm volatile("" ::: "memory");
    return barrier + (c << 3);
}

__attribute__((noinline))
double helper_mem_op(double* arr, int idx1, int idx2) {
    volatile double temp = arr[idx1] * 1.5;
    asm volatile("" ::: "memory");
    arr[idx2] = temp + 0.25;
    return arr[idx1] + arr[idx2];
}

/* Main complex function with high register pressure */
volatile uint64_t complex_scheduling_test(volatile int outer_iterations) {
    /* High register pressure: many local variables of different types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    volatile int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    
    /* Arrays for memory operations */
    double mem_array[32];
    for (int i = 0; i < 32; i++) {
        mem_array[i] = i * 0.5;
    }
    
    uint64_t checksum = 0;
    volatile int outer_limit = outer_iterations;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Mixed operation dependency chain */
        v1 = v2 + v3;
        asm volatile("" ::: "memory");
        f1 = (float)v1 * f2;
        asm volatile("" ::: "memory");
        d1 = (double)f1 + d2;
        asm volatile("" ::: "memory");
        mem_array[v1 % 32] = d1;
        asm volatile("" ::: "memory");
        v2 = (int)mem_array[(v1 + 1) % 32];
        
        /* Nested loops with variable bounds */
        volatile int middle_limit = (outer % 8) + 3;
        for (volatile int middle = 0; middle < middle_limit; middle++) {
            volatile int inner_limit = (middle * 2) + 1;
            
            for (volatile int inner = 0; inner < inner_limit; inner++) {
                /* Complex conditional execution paths */
                switch ((inner + outer) % 5) {
                    case 0: /* FP math path */
                        f3 = helper_float_op(f1, f2, f3);
                        f4 = f3 * f5 - f6;
                        d3 = (double)f4 * d4;
                        checksum ^= *(uint64_t*)&d3;
                        break;
                        
                    case 1: /* Integer bit manipulation path */
                        v3 = helper_int_op(v1, v2, v3);
                        v4 = (v3 << 2) | (v4 >> 1);
                        v5 = v4 ^ v5 ^ v6;
                        checksum ^= v5;
                        break;
                        
                    case 2: /* Memory operation path */
                        d5 = helper_mem_op(mem_array, inner % 32, (inner + 1) % 32);
                        f5 = (float)d5 * 2.0f;
                        v6 = (int)f5 + v7;
                        checksum ^= v6;
                        break;
                        
                    case 3: /* Mixed type conversion path */
                        f6 = (float)v8 * 0.25f;
                        d6 = (double)f6 + d7;
                        v9 = (int)d6 * v10;
                        mem_array[v9 % 32] = d6;
                        checksum ^= *(uint64_t*)&mem_array[v9 % 32];
                        break;
                        
                    case 4: /* Dependency chain across types */
                        v11 = v12 * v13 + v14;
                        asm volatile("" ::: "memory");
                        f7 = helper_float_op((float)v11, f8, f9);
                        asm volatile("" ::: "memory");
                        d8 = (double)f7 * d9;
                        asm volatile("" ::: "memory");
                        v15 = helper_int_op((int)d8, v16, v17);
                        asm volatile("" ::: "memory");
                        mem_array[v15 % 32] = d8;
                        checksum ^= v15;
                        break;
                }
                
                /* Additional barriers to prevent reordering */
                asm volatile("" ::: "memory");
                
                /* More mixed operations to keep variables live */
                v18 = v19 + v20;
                f8 = f9 * f10;
                d9 = d10 * 1.1;
                
                /* Function calls with scheduling side effects */
                if (inner % 3 == 0) {
                    f9 = helper_float_op(f8, f7, f6);
                    checksum ^= *(uint32_t*)&f9;
                }
                
                if (inner % 4 == 0) {
                    v19 = helper_int_op(v18, v17, v16);
                    checksum ^= v19;
                }
                
                if (inner % 5 == 0) {
                    d10 = helper_mem_op(mem_array, 
                                       (inner + outer) % 32, 
                                       (inner + middle) % 32);
                    checksum ^= *(uint64_t*)&d10;
                }
            }
            
            /* Inter-loop dependencies */
            v20 = v1 + v2 + v3;
            f10 = f1 + f2 + f3;
            d1 = d2 + d3 + d4;
        }
        
        /* Cross-iteration dependencies */
        v1 = v20 ^ checksum;
        f1 = f10 * 0.99f;
        d2 = d1 * 1.01;
        
        /* Memory barrier between outer iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Final checksum calculation using all variables */
    checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5;
    checksum ^= v6 ^ v7 ^ v8 ^ v9 ^ v10;
    checksum ^= v11 ^ v12 ^ v13 ^ v14 ^ v15;
    checksum ^= v16 ^ v17 ^ v18 ^ v19 ^ v20;
    
    uint32_t f_checksum = *(uint32_t*)&f1 ^ *(uint32_t*)&f2 ^ 
                         *(uint32_t*)&f3 ^ *(uint32_t*)&f4 ^ 
                         *(uint32_t*)&f5;
    f_checksum ^= *(uint32_t*)&f6 ^ *(uint32_t*)&f7 ^ 
                  *(uint32_t*)&f8 ^ *(uint32_t*)&f9 ^ 
                  *(uint32_t*)&f10;
    
    checksum ^= f_checksum;
    checksum ^= *(uint64_t*)&d1 ^ *(uint64_t*)&d2 ^ 
                *(uint64_t*)&d3 ^ *(uint64_t*)&d4 ^ 
                *(uint64_t*)&d5;
    checksum ^= *(uint64_t*)&d6 ^ *(uint64_t*)&d7 ^ 
                *(uint64_t*)&d8 ^ *(uint64_t*)&d9 ^ 
                *(uint64_t*)&d10;
    
    /* Include array contents in final checksum */
    for (int i = 0; i < 32; i++) {
        checksum ^= *(uint64_t*)&mem_array[i];
    }
    
    return checksum;
}

int main() {
    /* Force creation of scheduler context with sufficient iterations */
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_test(iterations);
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
