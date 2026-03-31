/* test_scheduler_context.c
 * A program designed to trigger GCC's instruction scheduler context allocation
 * and cleanup, specifically covering the free_sched_context block in haifa-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile inputs to prevent constant propagation */
volatile int g_input1 = 7;
volatile int g_input2 = 13;
volatile float g_input3 = 3.14159f;
volatile float g_input4 = 2.71828f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_int_computation(int a, int b, int c) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - a;
    v4 = v3 / b;
    v5 = v4 << 2;
    v6 = v5 | v1;
    v7 = v6 & 0xFF;
    v8 = v7 ^ v2;
    v9 = v8 + v3;
    v10 = v9 - v4;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10;
    v10 = v11 + 1;  /* WAR on v10 */
    v12 = v10 * 2;
    v12 = v12 + 5;  /* WAW on v12 */
    
    /* More complex dependency chain */
    v13 = v12 >> 1;
    v14 = v13 * v5;
    v15 = v14 % 17;
    v16 = v15 + v8;
    v17 = v16 - v9;
    v18 = v17 * v10;
    v19 = v18 / (v11 + 1);
    v20 = v19 | v12;
    
    /* Control flow to create multiple basic blocks */
    if (v20 > 100) {
        v21 = v20 * 3;
        v22 = v21 + 7;
        v23 = v22 - v13;
    } else {
        v21 = v20 / 2;
        v22 = v21 * 5;
        v23 = v22 + v14;
    }
    
    /* Inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v23));
    
    v24 = v23 + v15;
    v25 = v24 * v16;
    v26 = v25 - v17;
    v27 = v26 / (v18 + 1);
    v28 = v27 | v19;
    v29 = v28 & 0xFFFF;
    v30 = v29 ^ v20;
    
    /* Use all variables to prevent dead store elimination */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float floating_point_processing(float base, int iterations) {
    /* Many local variables for register pressure */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    float arr[10];
    float result = 0.0f;
    
    /* Initialize array with dependencies */
    f1 = base;
    for (int i = 0; i < 10; i++) {
        arr[i] = f1 * i;
        f1 = arr[i] + 1.0f;  /* Loop-carried dependency */
    }
    
    /* Mixed integer/float operations */
    f2 = arr[0] * 2.0f;
    f3 = f2 / 1.5f;
    f4 = f3 + arr[1];
    f5 = f4 - arr[2];
    f6 = f5 * arr[3];
    f7 = f6 / arr[4];
    f8 = f7 + f2;
    f9 = f8 - f3;
    f10 = f9 * f4;
    
    /* Loop with conditional inside */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            f11 = f10 * i;
            f12 = sinf(f11);
            f13 = cosf(f12);
        } else if (i % 3 == 1) {
            f14 = f10 / (i + 1);
            f15 = sqrtf(f14);
            f16 = logf(f15 + 1.0f);
        } else {
            f17 = f10 + i;
            f18 = expf(f17);
            f19 = f18 * 0.5f;
        }
        
        /* Artificial dependency with inline assembly */
        asm volatile("" : "+r"(i));
        
        f20 = f11 + f14 + f17;
        result += f20;
    }
    
    return result + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 +
           f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18 + f19 + f20;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int a, float b, double c) {
    /* Declare many variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Complex initialization with dependencies */
    i1 = a * 2;
    f1 = b * 3.0f;
    d1 = c * 4.0;
    
    i2 = i1 + (int)f1;
    f2 = f1 + (float)i1;
    d2 = d1 + (double)a;
    
    i3 = i2 * 5;
    f3 = f2 / 2.0f;
    d3 = d2 * 1.5;
    
    /* Nested control flow */
    if (i3 > 50) {
        i4 = i3 / 2;
        f4 = f3 * 3.0f;
        d4 = d3 - 10.0;
        
        if (f4 > 20.0f) {
            i5 = i4 + 100;
            f5 = f4 / 1.5f;
            d5 = d4 * 2.0;
        } else {
            i5 = i4 - 50;
            f5 = f4 * 1.5f;
            d5 = d4 / 2.0;
        }
    } else {
        i4 = i3 * 3;
        f4 = f3 + 15.0f;
        d4 = d3 + 5.0;
        
        i5 = i4 % 17;
        f5 = f4 - 3.0f;
        d5 = d4 * 0.75;
    }
    
    /* More operations with dependencies */
    i6 = i5 ^ i4;
    f6 = f5 * f4;
    d6 = d5 + d4;
    
    i7 = i6 << 2;
    f7 = f6 / f3;
    d7 = d6 - d3;
    
    i8 = i7 >> 1;
    f8 = f7 + f2;
    d8 = d7 * d2;
    
    i9 = i8 | i5;
    f9 = f8 - f1;
    d9 = d8 / d1;
    
    i10 = i9 & 0xFF;
    f10 = f9 * 2.0f;
    d10 = d9 + 1.0;
    
    /* Use all variables */
    return (double)(i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10) +
           (double)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10) +
           (d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10);
}

/* Function 4: Switch statement with different operation blocks */
long switch_based_computation(int mode, long seed) {
    long result = seed;
    
    switch (mode % 5) {
        case 0: {
            /* Block with many integer operations */
            long a1 = result * 3;
            long a2 = a1 + 7;
            long a3 = a2 - 11;
            long a4 = a3 * 13;
            long a5 = a4 / 5;
            long a6 = a5 | 0xAAAA;
            long a7 = a6 & 0x5555;
            long a8 = a7 ^ 0x3333;
            long a9 = a8 << 3;
            long a10 = a9 >> 2;
            result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
            break;
        }
        case 1: {
            /* Block with mixed operations */
            double b1 = (double)result * 1.1;
            double b2 = b1 + 2.2;
            double b3 = b2 * 3.3;
            double b4 = b3 / 4.4;
            float b5 = (float)b4 + 5.5f;
            float b6 = b5 * 6.6f;
            float b7 = b6 - 7.7f;
            int b8 = (int)b7 * 8;
            int b9 = b8 + 9;
            int b10 = b9 % 10;
            result = (long)(b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10);
            break;
        }
        case 2: {
            /* Block with memory access pattern */
            int arr[20];
            for (int i = 0; i < 20; i++) {
                arr[i] = (int)result + i;
            }
            for (int i = 1; i < 19; i++) {
                arr[i] = arr[i-1] + arr[i+1];  /* Memory dependencies */
            }
            for (int i = 0; i < 20; i++) {
                result += arr[i];
            }
            break;
        }
        case 3: {
            /* Block with artificial dependencies via inline assembly */
            long c1 = result;
            asm volatile("" : "+r"(c1));
            long c2 = c1 * 2;
            asm volatile("" : "+r"(c2));
            long c3 = c2 + 3;
            asm volatile("" : "+r"(c3));
            long c4 = c3 - 4;
            asm volatile("" : "+r"(c4));
            long c5 = c4 * 5;
            result = c1 + c2 + c3 + c4 + c5;
            break;
        }
        case 4: {
            /* Block with complex control flow within */
            long d1 = result;
            for (int i = 0; i < 10; i++) {
                if (d1 % 2 == 0) {
                    d1 = d1 * 3 + 1;
                } else {
                    d1 = d1 / 2;
                }
                if (i % 3 == 0) {
                    d1 += i * 7;
                } else if (i % 3 == 1) {
                    d1 -= i * 5;
                } else {
                    d1 ^= i * 3;
                }
            }
            result = d1;
            break;
        }
    }
    
    return result;
}

/* Main function to ensure all code is executed */
int main(int argc, char *argv[]) {
    /* Use command line arguments or defaults to create dynamic inputs */
    int input1 = (argc > 1) ? atoi(argv[1]) : g_input1;
    int input2 = (argc > 2) ? atoi(argv[2]) : g_input2;
    float input3 = (argc > 3) ? atof(argv[3]) : g_input3;
    float input4 = (argc > 4) ? atof(argv[4]) : g_input4;
    
    /* Volatile sink to prevent optimization */
    volatile long total_result = 0;
    
    /* Call all complex functions */
    total_result += complex_int_computation(input1, input2, input1 + input2);
    total_result += (long)floating_point_processing(input3, 5);
    total_result += (long)mixed_operations(input1, input3, input4);
    total_result += switch_based_computation(input1, input2);
    
    /* Additional calls with different parameters */
    for (int i = 0; i < 3; i++) {
        total_result += complex_int_computation(input1 + i, input2 - i, i * 10);
        total_result += (long)floating_point_processing(input3 + i, 3);
    }
    
    /* Print result to ensure code isn't dead */
    printf("Result: %ld\n", total_result);
    
    return 0;
}
