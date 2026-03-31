#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_op(float a, float b, float c) {
    volatile float barrier = a + b;
    asm volatile("" ::: "memory");
    return (barrier * c) - (a / (b + 1.0f));
}

__attribute__((noinline))
int helper_int_op(int a, int b, int c) {
    volatile int barrier = a ^ b;
    asm volatile("" ::: "memory");
    return (barrier * c) + ((a & b) | (c << 3));
}

__attribute__((noinline))
double helper_mixed_op(int a, float b, double c) {
    volatile double barrier = (double)a + (double)b;
    asm volatile("" ::: "memory");
    return barrier * c + (double)(a % 7) / (c + 1.0);
}

/* Complex function with high register pressure and mixed operations */
__attribute__((noinline))
uint64_t complex_scheduling_function(volatile int outer_iterations) {
    /* High register pressure: many live variables of different types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    double d6 = 6.66, d7 = 7.77, d8 = 8.88, d9 = 9.99, d10 = 10.1010;
    int i11 = 11, i12 = 12, i13 = 13, i14 = 14, i15 = 15;
    float f11 = 11.11f, f12 = 12.12f, f13 = 13.13f, f14 = 14.14f, f15 = 15.15f;
    double d11 = 11.111, d12 = 12.122, d13 = 13.133, d14 = 14.144, d15 = 15.155;
    
    /* Arrays for memory operations */
    int arr_int[32];
    float arr_float[32];
    double arr_double[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_int[i] = i * 3;
        arr_float[i] = i * 1.5f;
        arr_double[i] = i * 2.5;
    }
    
    uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        /* Nested loops with variable bounds */
        volatile int middle_limit = (outer % 8) + 3;
        
        for (int middle = 0; middle < middle_limit; middle++) {
            /* Inner loop with complex dependency chain */
            volatile int inner_limit = (middle * 7 + outer) % 16 + 2;
            
            for (int inner = 0; inner < inner_limit; inner++) {
                /* Mixed operation dependency chains */
                
                /* Chain 1: int -> float -> memory store */
                int temp1 = i6 + i7 - i8 * (inner + 1);
                asm volatile("" ::: "memory");
                float temp2 = f6 * f7 + (float)temp1 / 3.0f;
                asm volatile("" ::: "memory");
                arr_float[inner % 32] = temp2 + f8;
                
                /* Chain 2: float -> int -> memory load -> double */
                float temp3 = f9 * f10 - f11;
                asm volatile("" ::: "memory");
                int temp4 = (int)temp3 + i9 * i10;
                asm volatile("" ::: "memory");
                double temp5 = arr_double[temp4 % 32] + d6;
                asm volatile("" ::: "memory");
                d7 = temp5 * d8 - d9;
                
                /* Chain 3: memory load -> int -> float -> memory store */
                int temp6 = arr_int[(inner + middle) % 32];
                asm volatile("" ::: "memory");
                float temp7 = (float)temp6 * 1.7f + f12;
                asm volatile("" ::: "memory");
                arr_float[(inner + 5) % 32] = temp7;
                
                /* Conditional execution paths */
                switch ((inner + outer + middle) % 5) {
                    case 0:
                        /* FP math intensive path */
                        f13 = helper_float_op(f13, f14, f15);
                        d10 = d11 * d12 + d13 / (d14 + 1.0);
                        arr_double[inner % 32] = d10;
                        break;
                    case 1:
                        /* Integer bit manipulation path */
                        i11 = helper_int_op(i11, i12, i13);
                        i14 = (i11 ^ i12) & (i13 | i14);
                        arr_int[(inner + 3) % 32] = i14;
                        break;
                    case 2:
                        /* Mixed operations path */
                        d15 = helper_mixed_op(i15, f15, d15);
                        f1 = f2 * f3 - f4 / (f5 + 1.0f);
                        i1 = i2 + i3 * i4 - i5;
                        break;
                    case 3:
                        /* Memory intensive path */
                        for (int k = 0; k < 4; k++) {
                            arr_int[(inner + k) % 32] += arr_float[(inner + k + 1) % 32];
                            arr_float[(inner + k) % 32] *= 1.01f;
                            arr_double[(inner + k) % 32] = arr_double[(inner + k + 2) % 32] * 0.99;
                        }
                        break;
                    case 4:
                        /* Dependency chain across all types */
                        i6 = arr_int[inner % 32] + 1;
                        f7 = (float)i6 * 0.5f + arr_float[(inner + 1) % 32];
                        d8 = (double)f7 + arr_double[(inner + 2) % 32];
                        i9 = (int)d8 ^ arr_int[(inner + 3) % 32];
                        arr_int[(inner + 4) % 32] = i9;
                        break;
                }
                
                /* More mixed operations to keep variables live */
                v1 = v2 + v3 * (inner % 7);
                f2 = f3 * 1.1f - f4 / (v1 + 1.0f);
                d3 = d4 + d5 * (double)(middle % 5);
                v4 = v5 ^ (int)(f2 * 10.0f);
                
                /* Memory barrier to prevent reordering */
                asm volatile("" ::: "memory");
            }
            
            /* Call helper functions with live variables */
            if (middle % 3 == 0) {
                f5 = helper_float_op(f5, f6, f7);
            } else if (middle % 3 == 1) {
                i10 = helper_int_op(i10, i11, i12);
            } else {
                d13 = helper_mixed_op(i13, f13, d13);
            }
        }
        
        /* Update checksum with all variables */
        checksum ^= (uint64_t)v1;
        checksum ^= (uint64_t)(*(uint32_t*)&f2);
        checksum ^= (uint64_t)(*(uint64_t*)&d3);
        checksum ^= (uint64_t)i6;
        checksum ^= (uint64_t)(*(uint32_t*)&f7);
        checksum ^= (uint64_t)(*(uint64_t*)&d8);
        
        /* Access arrays to keep them live */
        for (int j = 0; j < 4; j++) {
            checksum += arr_int[(outer + j) % 32];
            checksum += (uint64_t)(*(uint32_t*)&arr_float[(outer + j + 1) % 32]);
            checksum += (uint64_t)(*(uint64_t*)&arr_double[(outer + j + 2) % 32]);
        }
    }
    
    return checksum;
}

int main() {
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Checksum result: %llu\n", (unsigned long long)result);
    
    return 0;
}
