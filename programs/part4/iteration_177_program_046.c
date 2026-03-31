#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float(float a, float b, float c) {
    volatile float v1 = a * b + c;
    asm volatile("" ::: "memory");
    float v2 = v1 / (b + 1.0f);
    asm volatile("" ::: "memory");
    return v2 - a;
}

__attribute__((noinline))
int helper_int(int a, int b, int c) {
    volatile int v1 = (a ^ b) | c;
    asm volatile("" ::: "memory");
    int v2 = (v1 << 3) & 0xFF;
    asm volatile("" ::: "memory");
    return v2 * (b % 7);
}

__attribute__((noinline))
double helper_double(double a, double b, int c) {
    volatile double v1 = a / (b + 0.5);
    asm volatile("" ::: "memory");
    double v2 = v1 * c + a;
    asm volatile("" ::: "memory");
    return v2 - b;
}

/* Main complex function with high register pressure */
void complex_scheduling_test(void) {
    /* Many local variables to create register pressure */
    volatile int outer_limit = 1000;
    volatile int inner1_base = 50;
    volatile int inner2_base = 20;
    
    /* Integer variables */
    int i, j, k, m, n, p, q, r, s, t;
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    
    /* Floating point variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Memory/pointer variables */
    int arr1[64], arr2[64];
    float farr1[64], farr2[64];
    int *p1, *p2;
    float *fp1, *fp2;
    
    /* Initialize arrays */
    for (i = 0; i < 64; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    p1 = arr1;
    p2 = arr2;
    fp1 = farr1;
    fp2 = farr2;
    
    /* Initialize variables */
    a1 = a2 = a3 = a4 = a5 = a6 = a7 = a8 = a9 = a10 = 1;
    b1 = b2 = b3 = b4 = b5 = b6 = b7 = b8 = b9 = b10 = 2;
    f1 = f2 = f3 = f4 = f5 = 1.0f;
    f6 = f7 = f8 = f9 = f10 = 2.0f;
    d1 = d2 = d3 = d4 = d5 = 1.0;
    d6 = d7 = d8 = d9 = d10 = 2.0;
    
    volatile uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        /* First level nested loop with variable bound */
        int inner1_limit = inner1_base + (i % 10);
        for (j = 0; j < inner1_limit; j++) {
            /* Second level nested loop with volatile-dependent bound */
            volatile int inner2_mod = inner2_base + (j % 5);
            for (k = 0; k < inner2_mod; k++) {
                /* Complex mixed operation sequences with true dependencies */
                
                /* Integer arithmetic chain */
                a1 = a2 + a3;
                asm volatile("" ::: "memory");
                a2 = a1 * a4;
                asm volatile("" ::: "memory");
                a3 = a2 ^ a5;
                asm volatile("" ::: "memory");
                a4 = a3 | a6;
                asm volatile("" ::: "memory");
                a5 = a4 - a7;
                
                /* Floating point chain with integer input */
                f1 = (float)a1 * f2;
                asm volatile("" ::: "memory");
                f2 = f1 + f3;
                asm volatile("" ::: "memory");
                f3 = f2 / (f4 + 0.1f);
                
                /* Memory access chain */
                int idx = (a1 + j) & 63;
                b1 = arr1[idx] + arr2[idx];
                asm volatile("" ::: "memory");
                arr1[idx] = b1 ^ a2;
                asm volatile("" ::: "memory");
                arr2[idx] = arr1[idx] * k;
                
                /* Double precision chain */
                d1 = (double)f1 * d2;
                asm volatile("" ::: "memory");
                d2 = d1 + d3;
                asm volatile("" ::: "memory");
                d3 = d2 / (d4 + 0.01);
                
                /* Conditional execution paths */
                switch ((i + j + k) % 4) {
                    case 0:
                        /* FP math intensive path */
                        f4 = helper_float(f1, f2, f3);
                        asm volatile("" ::: "memory");
                        f5 = f4 * f6 - f7;
                        asm volatile("" ::: "memory");
                        d4 = helper_double(d1, d2, a1);
                        break;
                        
                    case 1:
                        /* Integer bit manipulation path */
                        b2 = helper_int(a1, a2, a3);
                        asm volatile("" ::: "memory");
                        b3 = (b2 << 2) ^ (b1 >> 1);
                        asm volatile("" ::: "memory");
                        b4 = b3 & 0xAAAAAAAA;
                        break;
                        
                    case 2:
                        /* Mixed type path */
                        f6 = (float)helper_int(b1, b2, b3) * 0.5f;
                        asm volatile("" ::: "memory");
                        d5 = helper_double(d3, d4, helper_int(a4, a5, a6));
                        asm volatile("" ::: "memory");
                        a6 = (int)(f6 * d5) & 0xFF;
                        break;
                        
                    case 3:
                        /* Memory intensive path */
                        for (m = 0; m < 4; m++) {
                            int idx2 = (idx + m) & 63;
                            farr1[idx2] = farr2[idx2] * f1;
                            asm volatile("" ::: "memory");
                            farr2[idx2] = farr1[idx2] + f2;
                            asm volatile("" ::: "memory");
                            arr1[idx2] = (int)farr1[idx2] ^ arr2[idx2];
                        }
                        break;
                }
                
                /* More interleaved operations */
                a7 = a6 + (int)f3;
                asm volatile("" ::: "memory");
                f7 = f6 * (float)a7;
                asm volatile("" ::: "memory");
                d6 = d5 + (double)f7;
                asm volatile("" ::: "memory");
                b5 = arr1[(a7 + k) & 63];
                asm volatile("" ::: "memory");
                
                /* Update checksum with various values */
                checksum ^= (uint64_t)a1;
                checksum ^= (uint64_t)(*(uint32_t*)&f1);
                checksum ^= (uint64_t)(*(uint64_t*)&d1);
                checksum ^= (uint64_t)b1;
            }
            
            /* Additional operations between inner loops */
            a8 = helper_int(a7, b5, j);
            asm volatile("" ::: "memory");
            f8 = helper_float(f7, (float)a8, f3);
            asm volatile("" ::: "memory");
            d8 = helper_double(d7, d6, a8);
        }
        
        /* Periodic complex sequence every 100 iterations */
        if ((i % 100) == 99) {
            /* Extra complex nested loop */
            for (n = 0; n < 10; n++) {
                p = helper_int(i, n, a8);
                asm volatile("" ::: "memory");
                q = p * arr1[n] - arr2[n];
                asm volatile("" ::: "memory");
                f9 = helper_float(f8, (float)q, farr1[n]);
                asm volatile("" ::: "memory");
                d9 = helper_double(d8, (double)f9, q);
                asm volatile("" ::: "memory");
                
                checksum ^= (uint64_t)q;
                checksum ^= (uint64_t)(*(uint32_t*)&f9);
            }
        }
    }
    
    /* Final accumulation */
    for (i = 0; i < 64; i++) {
        checksum ^= (uint64_t)arr1[i];
        checksum ^= (uint64_t)arr2[i];
        checksum ^= (uint64_t)(*(uint32_t*)&farr1[i]);
        checksum ^= (uint64_t)(*(uint32_t*)&farr2[i]);
    }
    
    /* Prevent dead code elimination */
    volatile uint64_t final_result = checksum;
    printf("Final checksum: %llu\n", (unsigned long long)final_result);
}

int main(void) {
    complex_scheduling_test();
    return 0;
}
