#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_ops(float a, float b, float c, float d, float e) {
    volatile float v1 = a * b + c;
    float v2 = d / e + v1;
    float v3 = v2 * v2 - v1;
    asm volatile("" ::: "memory");
    return v3 * 0.5f + a - b;
}

__attribute__((noinline))
static int helper_int_ops(int a, int b, int c, int d, int e) {
    volatile int v1 = a ^ b;
    int v2 = (c * d) | v1;
    int v3 = v2 + (e << 3);
    asm volatile("" ::: "memory");
    return (v3 & 0xFF) + (a % 7);
}

__attribute__((noinline))
static double helper_mixed_ops(int a, float b, double c, int* d) {
    volatile double v1 = (double)a * (double)b + c;
    *d += (int)v1;
    double v2 = v1 / (c + 1.0);
    asm volatile("" ::: "memory");
    return v2 * 2.0 - (double)(*d % 256);
}

/* Complex function with high register pressure and mixed operations */
static uint64_t complex_scheduling_function(volatile int outer_limit) {
    /* High register pressure: many live variables of different types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile int arr[32];
    volatile int* ptr_arr[16];
    volatile uint64_t checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr[i] = i * 3 + 1;
    }
    for (int i = 0; i < 16; i++) {
        ptr_arr[i] = (int*)&arr[i * 2];
    }
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Nested loops with variable bounds */
        int inner_limit = (outer % 8) + 3;
        for (int inner1 = 0; inner1 < inner_limit; inner1++) {
            /* Mixed operation dependency chains */
            int temp1 = v1 + v2 * v3;
            float temp2 = f1 * f2 + (float)temp1;
            d1 = (double)temp2 * d2;
            
            /* Memory access pattern */
            arr[inner1] = (int)d1 + temp1;
            temp1 = arr[(inner1 + 1) % 32] ^ arr[(inner1 + 2) % 32];
            
            /* Inline assembly barrier */
            asm volatile("" ::: "memory");
            
            /* More mixed operations */
            f3 = f4 / f5 + (float)temp1;
            v4 = (int)f3 * v5 - v6;
            d3 = d4 * d5 + (double)v4;
            
            /* Conditional execution paths */
            switch (inner1 % 4) {
                case 0:
                    /* FP math branch */
                    f6 = helper_float_ops(f1, f2, f3, f4, f5);
                    v7 = (int)(f6 * 100.0f);
                    d1 = d2 * d3 - (double)v7;
                    break;
                case 1:
                    /* Integer bit manipulation branch */
                    v8 = helper_int_ops(v1, v2, v3, v4, v5);
                    v8 = (v8 << 3) | (v8 >> 5);
                    arr[v8 % 32] ^= 0xAA55AA55;
                    break;
                case 2:
                    /* Mixed operations branch */
                    d5 = helper_mixed_ops(v9, f7, d4, &v10);
                    f8 = (float)d5 * f9 - f10;
                    v9 = (int)f8 + arr[v10 % 32];
                    break;
                case 3:
                    /* Memory intensive branch */
                    for (int j = 0; j < 4; j++) {
                        ptr_arr[j][0] += ptr_arr[j + 1][0];
                        ptr_arr[j][1] ^= ptr_arr[j + 2][1];
                    }
                    asm volatile("" ::: "memory");
                    break;
            }
            
            /* Another nested loop with data-dependent bounds */
            int inner2_limit = (v1 % 5) + 2;
            for (int inner2 = 0; inner2 < inner2_limit; inner2++) {
                /* Complex dependency chain across iterations */
                v1 = v2 + v3 * inner2;
                f1 = f2 * (float)v1 - f3;
                d2 = (double)f1 / (d3 + 1.0);
                arr[(inner1 + inner2) % 32] = (int)d2;
                
                /* Call helper with live variables */
                if (inner2 % 2 == 0) {
                    f4 = helper_float_ops(f1, f2, f3, f4, f5);
                } else {
                    v5 = helper_int_ops(v1, v2, v3, v4, v5);
                }
                
                /* Memory barrier */
                asm volatile("" ::: "memory");
                
                /* More operations to increase pressure */
                v6 = v7 ^ v8 + inner2;
                f5 = f6 * f7 - (float)v6;
                d3 = d4 + d5 * (double)inner2;
            }
            
            /* Update checksum with various values */
            checksum ^= (uint64_t)v1;
            checksum ^= (uint64_t)(*(uint32_t*)&f1);
            checksum ^= (uint64_t)(*(uint64_t*)&d1);
            checksum ^= (uint64_t)arr[inner1 % 32];
        }
        
        /* Rotate values to create varying patterns */
        int tmp_v = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = tmp_v;
        
        float tmp_f = f1;
        f1 = f2; f2 = f3; f3 = f4; f4 = f5; f5 = f6;
        f6 = f7; f7 = f8; f8 = f9; f9 = f10; f10 = tmp_f;
        
        double tmp_d = d1;
        d1 = d2; d2 = d3; d3 = d4; d4 = d5; d5 = tmp_d;
    }
    
    return checksum;
}

int main(void) {
    volatile int iterations = 1000;
    volatile uint64_t final_checksum = 0;
    
    printf("Starting complex scheduling stress test...\n");
    
    /* Run the complex function multiple times */
    for (int run = 0; run < 3; run++) {
        uint64_t result = complex_scheduling_function(iterations);
        final_checksum ^= result;
        printf("Run %d checksum: 0x%016llx\n", run, (unsigned long long)result);
    }
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)final_checksum);
    
    /* Additional volatile operations to prevent dead code elimination */
    volatile int dummy = (int)final_checksum;
    asm volatile("" : "+r" (dummy));
    
    return 0;
}
