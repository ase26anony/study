/* test_scheduler_context.c
 * A program designed to trigger GCC's instruction scheduler context allocation
 * and cleanup, specifically covering the free_sched_context block.
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function 1: Integer-heavy computation with many serial dependencies */
int integer_computation(int seed) {
    /* Create many local variables to increase register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    
    /* Initial values with dependencies */
    a1 = seed * 2;
    a2 = a1 + seed;
    a3 = a2 * a1;
    a4 = a3 - seed;
    a5 = a4 / (seed + 1);
    a6 = a5 << 2;
    a7 = a6 ^ seed;
    a8 = a7 | 0xFF;
    a9 = a8 & 0x0F;
    a10 = a9 % 7;
    
    /* Anti-dependencies and output dependencies */
    b1 = a10;
    b2 = b1 + a9;
    b3 = b2 * a8;
    b4 = b3 - a7;
    b5 = b4 / a6;
    b6 = b5 << 3;
    b7 = b6 ^ a5;
    b8 = b7 | 0xAA;
    b9 = b8 & 0x55;
    b10 = b9 % 11;
    
    /* More complex dependencies with control flow */
    if (b10 > 100) {
        c1 = b10 * 2;
        c2 = c1 + b9;
        c3 = c2 * b8;
        c4 = c3 - b7;
        c5 = c4 / b6;
    } else {
        c1 = b10 / 2;
        c2 = c1 - b9;
        c3 = c2 + b8;
        c4 = c3 * b7;
        c5 = c4 % b6;
    }
    
    /* Final computation with all variables */
    c6 = c5 + a10 + b10;
    c7 = c6 * c1;
    c8 = c7 - c2;
    c9 = c8 / c3;
    c10 = c9 % c4;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(c10));
    
    return c10 + a1 + b1 + c1;
}

/* Function 2: Floating-point array processing with loops */
float float_computation(float seed) {
    float arr[20];
    float result = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = seed;
    arr[1] = seed * 2.0f;
    
    for (int i = 2; i < 20; i++) {
        /* True data dependencies */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
        
        /* Anti-dependencies */
        float temp = arr[i];
        arr[i-2] = temp * 0.3f;
        
        /* Output dependencies */
        arr[i-1] = arr[i] * arr[i-2];
        
        /* Inline assembly to prevent optimization */
        asm volatile("" : "+r"(arr[i]));
    }
    
    /* Process array with mixed operations */
    for (int i = 0; i < 19; i++) {
        if (i % 3 == 0) {
            result += arr[i] * arr[i+1];
        } else if (i % 3 == 1) {
            result -= arr[i] / (arr[i+1] + 1.0f);
        } else {
            result *= 1.0f + fabsf(arr[i] - arr[i+1]);
        }
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_computation(double seed) {
    /* Many local variables to stress register allocation */
    double v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    double w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
    double x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    
    /* Complex initialization chain */
    v1 = seed;
    v2 = v1 * 1.5;
    v3 = v2 + v1;
    v4 = v3 * v2;
    v5 = v4 - v3;
    v6 = v5 / (v4 + 1.0);
    v7 = sin(v6);
    v8 = cos(v7);
    v9 = v8 * v7;
    v10 = v9 - v8;
    
    /* Branch with different computation patterns */
    if (v10 > 0.0) {
        w1 = v10 * 2.0;
        w2 = w1 + v9;
        w3 = w2 * v8;
        w4 = w3 - v7;
        w5 = w4 / v6;
    } else {
        w1 = v10 / 2.0;
        w2 = w1 - v9;
        w3 = w2 + v8;
        w4 = w3 * v7;
        w5 = w4 * v6;
    }
    
    /* Nested loops with dependencies */
    for (int i = 0; i < 5; i++) {
        x1 = w5;
        for (int j = 0; j < 3; j++) {
            x2 = x1 * (i + 1);
            x3 = x2 + (j + 1);
            x4 = x3 * x2;
            x5 = x4 - x3;
            
            /* Inline assembly to create dependencies */
            asm volatile("" : "+r"(x5));
            
            x1 = x5;
        }
        w5 = x1;
    }
    
    /* Final computation using all variables */
    w6 = w5 + v10 + x5;
    w7 = w6 * w1;
    w8 = w7 - w2;
    w9 = w8 / w3;
    w10 = fmod(w9, w4 + 1.0);
    
    return w10 + v1 + x1;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_computation(long seed) {
    long result = 0;
    
    switch (seed % 5) {
        case 0: {
            /* Block with many integer operations */
            long a = seed * 3;
            long b = a << 4;
            long c = b ^ 0xABCD;
            long d = c | 0x1234;
            long e = d & 0xFF00;
            long f = e + a;
            long g = f - b;
            long h = g * c;
            long i = h / (d + 1);
            long j = i % (e + 1);
            result = j + f + g;
            break;
        }
        case 1: {
            /* Block with floating point conversions */
            double x = (double)seed;
            double y = x * 1.234;
            double z = y + 5.678;
            double w = z * x;
            double v = w / y;
            result = (long)(v * 1000.0);
            break;
        }
        case 2: {
            /* Block with memory access pattern */
            long arr[8];
            for (int k = 0; k < 8; k++) {
                arr[k] = seed * (k + 1);
            }
            for (int k = 1; k < 8; k++) {
                arr[k] = arr[k] + arr[k-1];
            }
            result = arr[7] - arr[0];
            break;
        }
        case 3: {
            /* Block with bit manipulation */
            result = seed;
            for (int k = 0; k < 16; k++) {
                result = ((result << 1) | (result >> 31)) ^ (0x5A5A5A5A << (k % 4));
                asm volatile("" : "+r"(result));
            }
            break;
        }
        case 4: {
            /* Block with mixed operations */
            result = seed;
            for (int k = 0; k < 10; k++) {
                if (k % 2 == 0) {
                    result = result * 3 + k;
                } else {
                    result = result / 2 - k;
                }
                float temp = (float)result;
                temp = temp * 0.5f;
                result = (long)temp;
            }
            break;
        }
    }
    
    return result;
}

/* Main function to ensure all code is executed */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int int_seed;
    volatile float float_seed;
    volatile double double_seed;
    volatile long long_seed;
    
    /* Initialize with non-constant values */
    if (argc > 1) {
        int_seed = atoi(argv[1]);
    } else {
        int_seed = 42;  /* Default seed */
    }
    
    float_seed = (float)int_seed * 0.5f;
    double_seed = (double)int_seed * 0.25;
    long_seed = (long)int_seed * 3L;
    
    /* Call all functions to trigger scheduler */
    int result1 = integer_computation(int_seed);
    float result2 = float_computation(float_seed);
    double result3 = mixed_computation(double_seed);
    long result4 = switch_computation(long_seed);
    
    /* Aggregate results to prevent dead code elimination */
    volatile long final_result = 0;
    final_result += result1;
    final_result += (long)result2;
    final_result += (long)result3;
    final_result += result4;
    
    /* Print to ensure code isn't optimized away */
    printf("Final checksum: %ld\n", final_result);
    
    return 0;
}
