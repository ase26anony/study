#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_ops(float a, float b, float c) {
    volatile float v1 = a * b + c;
    volatile float v2 = a / (b + 1.0f);
    asm volatile("" ::: "memory");
    return v1 - v2;
}

__attribute__((noinline))
static int helper_int_ops(int a, int b, int c) {
    volatile int v1 = (a * b) ^ c;
    volatile int v2 = (a + b) | c;
    asm volatile("" ::: "memory");
    return v1 & ~v2;
}

__attribute__((noinline))
static double helper_mixed_ops(int a, float b, double c) {
    volatile double d1 = (double)a * (double)b + c;
    volatile double d2 = c / (double)(a + 1);
    asm volatile("" ::: "memory");
    return d1 - d2;
}

/* Complex function with high register pressure and scheduling complexity */
static uint64_t complex_scheduling_function(volatile int outer_iterations) {
    /* High register pressure: 30+ local variables of mixed types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    volatile int arr_idx1 = 0, arr_idx2 = 0, arr_idx3 = 0;
    volatile int counter1 = 0, counter2 = 0, counter3 = 0;
    volatile int temp1, temp2, temp3, temp4, temp5;
    volatile float ftemp1, ftemp2, ftemp3;
    volatile double dtemp1, dtemp2;
    
    /* Local arrays for memory access patterns */
    int local_arr1[64];
    float local_arr2[64];
    double local_arr3[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        local_arr1[i] = i;
        local_arr2[i] = i * 0.5f;
        local_arr3[i] = i * 0.25;
    }
    
    uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        /* Mixed operation dependency chain */
        v1 = v2 + v3;
        f1 = f2 * f3;
        d1 = d2 - d3;
        
        /* Memory barrier to force scheduler to handle dependencies */
        asm volatile("" ::: "memory");
        
        /* Interleaved type conversions and operations */
        ftemp1 = (float)v1 + f1;
        temp1 = (int)f1 ^ v1;
        dtemp1 = (double)ftemp1 * d1;
        
        /* Store to memory with volatile index */
        local_arr1[arr_idx1 % 64] = temp1;
        local_arr2[arr_idx2 % 64] = ftemp1;
        local_arr3[arr_idx3 % 64] = dtemp1;
        
        /* Nested loops with variable bounds */
        volatile int middle_limit = (outer % 8) + 2;
        for (volatile int middle = 0; middle < middle_limit; middle++) {
            /* Inner loop with dependency on outer and middle */
            volatile int inner_limit = ((outer * middle) % 16) + 1;
            for (volatile int inner = 0; inner < inner_limit; inner++) {
                /* Complex mixed operations creating long dependency chains */
                v4 = v5 * v6 + inner;
                f4 = helper_float_ops(f5, f6, (float)inner);
                d4 = helper_mixed_ops(v4, f4, d5);
                
                /* Conditional execution paths */
                switch ((inner + middle) % 4) {
                    case 0:
                        /* FP math path */
                        f7 = f8 * f9 - f10;
                        d7 = d8 / d9 + d10;
                        temp2 = helper_int_ops(v7, v8, v9);
                        break;
                    case 1:
                        /* Integer bit manipulation path */
                        v7 = (v8 << 3) | (v9 >> 2);
                        v8 = ~v7 ^ v10;
                        temp2 = v7 & v8 | v9;
                        break;
                    case 2:
                        /* Memory intensive path */
                        temp2 = local_arr1[(inner + v7) % 64];
                        ftemp2 = local_arr2[(middle + v8) % 64];
                        dtemp2 = local_arr3[(outer + v9) % 64];
                        f7 = ftemp2 * 2.0f;
                        d7 = dtemp2 / 2.0;
                        break;
                    default:
                        /* Mixed operations path */
                        temp2 = helper_int_ops(v7, v8, v9);
                        f7 = helper_float_ops(f8, f9, f10);
                        d7 = helper_mixed_ops(temp2, f7, d8);
                        break;
                }
                
                /* More dependency chains across different operation types */
                v10 = v1 + temp2;
                f10 = f1 * (float)temp2;
                d10 = d1 + (double)temp2;
                
                /* Memory barrier to prevent reordering */
                asm volatile("" ::: "memory");
                
                /* Update array indices with complex expressions */
                arr_idx1 = (arr_idx1 + v10) % 64;
                arr_idx2 = (arr_idx2 + (int)f10) % 64;
                arr_idx3 = (arr_idx3 + (int)d10) % 64;
                
                /* Load from memory with volatile indices */
                temp3 = local_arr1[arr_idx1];
                ftemp3 = local_arr2[arr_idx2];
                dtemp1 = local_arr3[arr_idx3];
                
                /* Use loaded values in next operations */
                v9 = v10 ^ temp3;
                f9 = f10 + ftemp3;
                d9 = d10 * dtemp1;
                
                /* Update checksum with mixed operations */
                checksum ^= (uint64_t)v9;
                checksum += (uint64_t)((int)f9 * 1000);
                checksum ^= (uint64_t)((int)d9 * 1000000);
            }
            
            /* Function call with scheduling side effects */
            if (middle % 2 == 0) {
                temp4 = helper_int_ops(v2, v3, v4);
                f4 = helper_float_ops(f2, f3, f4);
            } else {
                d4 = helper_mixed_ops(v5, f5, d5);
            }
            
            /* Update volatile counters */
            counter1++;
            counter2 += middle;
            counter3 += outer;
        }
        
        /* Additional mixed operations between loop iterations */
        v5 = v6 * v7 - v8;
        f5 = f6 / f7 + f8;
        d5 = d6 * d7 - d8;
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        
        /* Complex address calculation and memory access */
        int complex_idx = (v5 * 31 + v6 * 17 + v7 * 13) % 64;
        local_arr1[complex_idx] = v5 + v6;
        local_arr2[complex_idx] = f5 + f6;
        local_arr3[complex_idx] = d5 + d6;
        
        /* Update checksum with array values */
        checksum ^= (uint64_t)local_arr1[complex_idx];
        checksum += (uint64_t)((int)local_arr2[complex_idx] * 100);
        checksum ^= (uint64_t)((int)local_arr3[complex_idx] * 10000);
    }
    
    /* Final accumulation using all variables */
    temp5 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    ftemp1 = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
    dtemp1 = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    
    checksum ^= (uint64_t)temp5;
    checksum += (uint64_t)((int)ftemp1 * 10);
    checksum ^= (uint64_t)((int)dtemp1 * 1000);
    checksum += (uint64_t)(counter1 + counter2 + counter3);
    
    return checksum;
}

int main(void) {
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: %llu\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
