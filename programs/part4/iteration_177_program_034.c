#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_ops(float a, float b, float c) {
    volatile float v1 = a * b + c;
    volatile float v2 = b / (a + 1.0f);
    asm volatile("" ::: "memory");
    return v1 - v2;
}

__attribute__((noinline))
int helper_int_ops(int a, int b, int c) {
    volatile int v1 = (a ^ b) | c;
    volatile int v2 = (a & b) << 3;
    asm volatile("" ::: "memory");
    return v1 + v2;
}

__attribute__((noinline))
double helper_mixed_ops(int a, float b, double c) {
    volatile double d1 = (double)a * (double)b + c;
    volatile double d2 = c / (double)(a + 1);
    asm volatile("" ::: "memory");
    return d1 - d2;
}

/* Main stress function with high register pressure */
__attribute__((noinline))
uint64_t stress_scheduler(void) {
    /* Many local variables to create register pressure */
    volatile int outer_limit = 1000;
    volatile int inner1_limit = 50;
    volatile int inner2_limit = 20;
    
    /* Integer variables */
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    int i11 = 11, i12 = 12, i13 = 13, i14 = 14, i15 = 15;
    int i16 = 16, i17 = 17, i18 = 18, i19 = 19, i20 = 20;
    
    /* Floating point variables */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    /* Double precision variables */
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    /* Array for memory operations */
    volatile int mem_buffer[256];
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i;
    }
    
    /* Checksum accumulator */
    uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* First inner loop with data-dependent trip count */
        int inner1_max = inner1_limit + (outer % 10);
        for (volatile int inner1 = 0; inner1 < inner1_max; inner1++) {
            /* Mixed operation dependency chain */
            i1 = i2 + i3;
            f1 = (float)i1 * f2;
            asm volatile("" ::: "memory");
            
            /* Memory access pattern */
            mem_buffer[(i1 + inner1) & 0xFF] = i1;
            i2 = mem_buffer[(inner1 + outer) & 0xFF];
            
            /* Second inner loop with more complex bounds */
            int inner2_max = inner2_limit + (inner1 % 5);
            for (volatile int inner2 = 0; inner2 < inner2_max; inner2++) {
                /* Complex floating point operations */
                f3 = helper_float_ops(f1, f2, f3);
                d1 = helper_mixed_ops(i2, f3, d1);
                
                /* Integer operations with barriers */
                i3 = helper_int_ops(i1, i2, i3);
                asm volatile("" ::: "memory");
                
                /* Conditional execution paths */
                switch ((inner1 + inner2) % 4) {
                    case 0:
                        /* FP-intensive path */
                        f4 = f1 * f2 - f3;
                        f5 = f4 / (f2 + 1.0f);
                        d2 = (double)f5 * d1;
                        i4 = (int)d2;
                        break;
                    case 1:
                        /* Integer-intensive path */
                        i5 = (i1 ^ i2) | i3;
                        i6 = (i4 & i5) << 2;
                        i7 = i6 * i5 - i4;
                        f6 = (float)i7 / 3.14159f;
                        break;
                    case 2:
                        /* Memory-intensive path */
                        for (int k = 0; k < 8; k++) {
                            mem_buffer[(k + inner2) & 0xFF] = 
                                mem_buffer[(k + inner1) & 0xFF] + k;
                        }
                        i8 = mem_buffer[inner2 & 0xFF];
                        f7 = (float)i8 * 0.5f;
                        break;
                    case 3:
                        /* Mixed operations with function calls */
                        i9 = helper_int_ops(i8, i7, i6);
                        f8 = helper_float_ops(f7, f6, f5);
                        d3 = helper_mixed_ops(i9, f8, d2);
                        i10 = (int)(d3 * 100.0);
                        break;
                }
                
                /* More dependency chains */
                i11 = i10 + i9;
                f9 = f8 * f7 + f6;
                asm volatile("" ::: "memory");
                
                /* Update checksum with various values */
                checksum ^= (uint64_t)i11;
                checksum += (uint64_t)(f9 * 1000.0f);
                checksum ^= (uint64_t)(d3 * 10000.0);
            }
            
            /* Additional operations between loops */
            i12 = i11 * i10 - i9;
            f10 = helper_float_ops(f9, f8, f7);
            d4 = d3 * d2 / d1;
            
            /* Memory store with barrier */
            mem_buffer[(outer + inner1) & 0xFF] = i12;
            asm volatile("" ::: "memory");
            
            /* Update more variables */
            i13 = mem_buffer[(inner1 * 2) & 0xFF];
            i14 = i13 + i12;
        }
        
        /* Periodic complex operations in outer loop */
        if (outer % 100 == 0) {
            /* Heavy computation block */
            for (int j = 0; j < 100; j++) {
                i15 = i14 + i13 + j;
                f1 = f10 * 1.1f + (float)j;
                d5 = d4 * 1.01 - (double)j;
                
                /* Nested conditional */
                if (j % 3 == 0) {
                    i16 = helper_int_ops(i15, i14, i13);
                    f2 = helper_float_ops(f1, f10, f9);
                } else if (j % 3 == 1) {
                    i17 = i16 << (j % 8);
                    f3 = f2 / (float)(i17 + 1);
                } else {
                    i18 = i17 | i16;
                    f4 = f3 * f2 - f1;
                }
                
                asm volatile("" ::: "memory");
                checksum += (uint64_t)i18;
            }
        }
        
        /* Update variables for next iteration */
        i19 = i18 + i17 + i16;
        i20 = i19 * 2 - i15;
        f5 = f4 + f3 + f2 + f1;
        d1 = d5 * 0.99;
    }
    
    /* Final complex computation */
    for (int final_iter = 0; final_iter < 100; final_iter++) {
        i1 = i20 ^ i19;
        f6 = helper_float_ops(f5, f4, f3);
        i2 = helper_int_ops(i1, i20, i19);
        d2 = helper_mixed_ops(i2, f6, d1);
        
        /* Memory access pattern */
        mem_buffer[final_iter & 0xFF] = i1 + i2;
        i3 = mem_buffer[(final_iter * 7) & 0xFF];
        
        checksum ^= (uint64_t)(i3 * final_iter);
        checksum += (uint64_t)(d2 * 1000.0);
    }
    
    return checksum;
}

int main(void) {
    printf("Starting scheduler stress test...\n");
    
    uint64_t result = stress_scheduler();
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
