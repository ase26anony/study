#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_op(float a, float b, float c) {
    volatile float barrier = a * b;
    asm volatile("" ::: "memory");
    return (barrier + c) * 0.5f;
}

__attribute__((noinline))
int helper_int_op(int a, int b, int c) {
    volatile int barrier = (a ^ b) & c;
    asm volatile("" ::: "memory");
    return barrier + (a >> 3) - (b << 2);
}

__attribute__((noinline))
double helper_mem_op(double *arr, int idx1, int idx2) {
    volatile double temp = arr[idx1] * arr[idx2];
    asm volatile("" ::: "memory");
    arr[idx1] = temp * 0.333;
    return temp;
}

/* Complex function with high register pressure and mixed operations */
__attribute__((noinline))
uint64_t complex_scheduling_function(volatile int outer_iterations) {
    /* High register pressure: 30+ local variables */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    double d6 = 6.66, d7 = 7.77, d8 = 8.88, d9 = 9.99, d10 = 10.101;
    int i11 = 11, i12 = 12, i13 = 13, i14 = 14, i15 = 15;
    float f11 = 11.11f, f12 = 12.12f, f13 = 13.13f;
    double d11 = 11.111, d12 = 12.121, d13 = 13.131;
    
    /* Array for memory operations */
    double mem_array[32];
    for (int i = 0; i < 32; i++) {
        mem_array[i] = i * 1.2345;
    }
    
    uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        /* Nested loops with variable bounds */
        volatile int mid_limit = (outer % 8) + 3;
        for (int mid = 0; mid < mid_limit; mid++) {
            /* Inner loop with volatile-dependent bound */
            volatile int inner_limit = (v1 + mid) % 7 + 2;
            for (int inner = 0; inner < inner_limit; inner++) {
                
                /* Mixed operation dependency chains */
                /* int -> float -> memory store -> int load chain */
                i6 = v1 * v2 + inner;
                f6 = (float)i6 * f1;
                mem_array[inner % 32] = f6 * d1;
                v3 = (int)mem_array[(inner + 1) % 32];
                
                /* Another dependency chain */
                i7 = helper_int_op(v2, v3, v4);
                f7 = helper_float_op(f2, f3, f4);
                d7 = helper_mem_op(mem_array, i7 % 32, inner % 32);
                
                /* Memory barrier to force scheduler grouping */
                asm volatile("" ::: "memory");
                
                /* Conditional execution paths with different operation mixes */
                switch ((inner + outer) % 5) {
                    case 0: /* FP math intensive */
                        f8 = f6 * f7 - f1;
                        f9 = sqrtf(fabsf(f8));
                        d8 = (double)f9 * d2 / d3;
                        i8 = (int)(d8 * 1000.0);
                        break;
                    case 1: /* Integer bit manipulation */
                        i9 = (i6 ^ i7) | (i8 << 3);
                        i10 = ~i9 & 0xFFFF;
                        i11 = (i10 >> 4) + (i9 & 0xFF);
                        i12 = i11 * 137; /* Prime multiplier */
                        break;
                    case 2: /* Memory intensive */
                        for (int m = 0; m < 4; m++) {
                            mem_array[(inner + m) % 32] = 
                                mem_array[(inner + m + 1) % 32] * 1.01;
                        }
                        d9 = mem_array[inner % 32] + mem_array[(inner + 3) % 32];
                        break;
                    case 3: /* Mixed type conversions */
                        f10 = (float)i9 * 0.25f;
                        d10 = (double)f10 * 2.0;
                        i13 = (int)d10;
                        f11 = (float)i13 / 3.0f;
                        break;
                    case 4: /* Function call intensive */
                        i14 = helper_int_op(i10, i11, i12);
                        f12 = helper_float_op(f10, f11, f3);
                        d11 = helper_mem_op(mem_array, i14 % 32, mid % 32);
                        break;
                }
                
                /* More interleaved operations */
                v4 = v3 * v5 - i6;
                f4 = f3 * f5 + (float)v4;
                d4 = d3 * d5 * (double)f4;
                
                /* Another memory barrier */
                asm volatile("" ::: "memory");
                
                /* Update checksum with various values */
                checksum ^= (uint64_t)i6;
                checksum ^= (uint64_t)(*(uint32_t*)&f7);
                checksum ^= (uint64_t)(*(uint64_t*)&d8);
                checksum += (uint64_t)v4;
            }
            
            /* Additional operations between inner loops */
            v5 = helper_int_op(v4, v3, v2);
            f5 = helper_float_op(f4, f3, f2);
            d5 = helper_mem_op(mem_array, v5 % 32, mid % 32);
        }
        
        /* Update volatile variables to prevent optimization */
        v1 = (v1 + 1) % 13;
        v2 = (v2 + 2) % 17;
        f1 = f1 * 1.1f;
        if (f1 > 100.0f) f1 = 1.1f;
        d1 = d1 * 1.01;
        if (d1 > 200.0) d1 = 1.11;
    }
    
    /* Final accumulation */
    checksum ^= (uint64_t)v1;
    checksum ^= (uint64_t)v2;
    checksum ^= (uint64_t)(*(uint32_t*)&f1);
    checksum ^= (uint64_t)(*(uint64_t*)&d1);
    
    return checksum;
}

int main() {
    /* Volatile to prevent constant propagation */
    volatile int iterations = 1000;
    
    printf("Starting complex scheduling stress test...\n");
    
    uint64_t result = complex_scheduling_function(iterations);
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
