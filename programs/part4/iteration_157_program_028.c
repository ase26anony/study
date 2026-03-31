/* test_scheduler_context.c
 * Designed to trigger GCC's scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile inputs to prevent constant propagation */
volatile int g_input1 = 42;
volatile int g_input2 = 17;
volatile float g_input3 = 3.14159f;
volatile float g_input4 = 2.71828f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_integer_ops(int a, int b, int c, int d) {
    /* Create register pressure with many variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / a;
    v5 = v4 << 2;
    v6 = v5 | b;
    v7 = v6 & c;
    v8 = v7 ^ d;
    v9 = v8 + v1;
    v10 = v9 - v2;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10;
    v12 = v11 * 3;
    v11 = v12 + 1;  /* WAR on v11 */
    v13 = v11;
    v11 = v13 - 2;  /* WAW on v11 */
    
    /* More complex dependency chain */
    v14 = v11 + v3;
    v15 = v14 * v4;
    v16 = v15 / v5;
    v17 = v16 | v6;
    v18 = v17 & v7;
    v19 = v18 ^ v8;
    v20 = v19 + v9;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 * 2;
    v22 = v21 - v10;
    v23 = v22 / 3;
    v24 = v23 << 1;
    v25 = v24 | 0xFF;
    
    /* Control flow to create multiple basic blocks */
    if (v25 > 1000) {
        v26 = v25 + 100;
        v27 = v26 * 2;
        v28 = v27 - 50;
    } else {
        v26 = v25 - 100;
        v27 = v26 / 2;
        v28 = v27 + 50;
    }
    
    /* Final computations using most variables */
    v29 = v28 + v20 + v15 + v10 + v5;
    v30 = v29 * 2;
    
    return v30;
}

/* Function 2: Floating-point array processing with loops */
float complex_float_ops(float a, float b, float c, float d) {
    /* Many local variables for register pressure */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    float arr[8];
    float result = 0.0f;
    
    /* Initialize array with dependencies */
    f1 = a + b;
    f2 = f1 * c;
    f3 = f2 - d;
    f4 = f3 / a;
    f5 = f4 + b;
    f6 = f5 * c;
    f7 = f6 - d;
    f8 = f7 / a;
    f9 = f8 + b;
    f10 = f9 * c;
    
    /* Array operations with loop-carried dependencies */
    arr[0] = f1;
    for (int i = 1; i < 8; i++) {
        arr[i] = arr[i-1] + f2 + (float)i;
        /* Inline assembly to prevent optimization */
        asm volatile("" : "+r"(i));
    }
    
    /* Mixed integer/float operations */
    f11 = arr[0] * arr[1];
    f12 = f11 + arr[2];
    f13 = f12 - arr[3];
    f14 = f13 * arr[4];
    f15 = f14 / arr[5];
    
    /* Conditional block */
    if (f15 > 100.0f) {
        f16 = f15 * 2.0f;
        f17 = f16 - 50.0f;
        f18 = sinf(f17);
    } else {
        f16 = f15 / 2.0f;
        f17 = f16 + 50.0f;
        f18 = cosf(f17);
    }
    
    f19 = f18 * arr[6];
    f20 = f19 + arr[7];
    
    /* Use all computed values */
    result = f1 + f3 + f5 + f7 + f9 + f11 + f13 + f15 + f17 + f19 + f20;
    for (int i = 0; i < 8; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many variables */
double mixed_operations(int a, float b, double c, int d) {
    /* Declare many variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Initial computations with dependencies */
    i1 = a * 2;
    f1 = (float)i1 + b;
    d1 = (double)f1 * c;
    
    i2 = i1 + d;
    f2 = f1 * 2.0f;
    d2 = d1 / 2.0;
    
    i3 = i2 - 5;
    f3 = f2 + 3.14f;
    d3 = d2 * 1.5;
    
    /* Complex control flow */
    switch (a % 4) {
        case 0:
            i4 = i3 * 3;
            f4 = f3 * 1.5f;
            d4 = d3 + 10.0;
            break;
        case 1:
            i4 = i3 / 2;
            f4 = f3 / 1.5f;
            d4 = d3 - 10.0;
            break;
        case 2:
            i4 = i3 << 1;
            f4 = f3 + f2;
            d4 = d3 * d2;
            break;
        default:
            i4 = i3 >> 1;
            f4 = f3 - f2;
            d4 = d3 / d2;
            break;
    }
    
    /* More operations using all variables */
    i5 = i4 + i3 + i2 + i1;
    f5 = f4 + f3 + f2 + f1;
    d5 = d4 + d3 + d2 + d1;
    
    i6 = i5 * 2;
    f6 = f5 * 1.1f;
    d6 = d5 * 1.01;
    
    /* Inline assembly to create barriers */
    asm volatile("" : "+r"(i6), "+r"(f6), "+r"(d6));
    
    i7 = i6 - 100;
    f7 = f6 - 10.0f;
    d7 = d6 - 1.0;
    
    i8 = i7 / 3;
    f8 = f7 / 2.0f;
    d8 = d7 / 1.5;
    
    i9 = i8 | 0xFF;
    f9 = f8 * f7;
    d9 = d8 * d7;
    
    i10 = i9 ^ 0xAA;
    f10 = f9 + f8;
    d10 = d9 - d8;
    
    /* Final combination */
    return (double)i10 + (double)f10 + d10;
}

/* Function 4: Switch statement with different operation blocks */
long switch_based_computation(int mode, long base) {
    long result = base;
    
    switch (mode) {
        case 0: {
            /* Block with many integer ops */
            long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
            a1 = base + 1;
            a2 = a1 * 2;
            a3 = a2 - 3;
            a4 = a3 / 4;
            a5 = a4 | 0xF0;
            a6 = a5 & 0x0F;
            a7 = a6 ^ 0xFF;
            a8 = a7 << 2;
            a9 = a8 >> 1;
            a10 = a9 + base;
            result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
            break;
        }
        case 1: {
            /* Block with mixed operations */
            float b1, b2, b3, b4, b5;
            long c1, c2, c3, c4, c5;
            b1 = (float)base * 1.5f;
            c1 = (long)b1 + 100;
            b2 = b1 + 2.5f;
            c2 = c1 * 2;
            b3 = b2 * 3.0f;
            c3 = c2 - 50;
            b4 = b3 / 2.0f;
            c4 = c3 | 0xAA;
            b5 = b4 - 1.0f;
            c5 = c4 & 0x55;
            result = c1 + c2 + c3 + c4 + c5 + (long)(b1 + b2 + b3 + b4 + b5);
            break;
        }
        case 2: {
            /* Block with loop and dependencies */
            long accum = 0;
            for (int i = 0; i < 10; i++) {
                long temp = base * i;
                accum += temp;
                /* Prevent loop unrolling from simplifying too much */
                asm volatile("" : "+r"(i));
            }
            result = accum * 2;
            break;
        }
        default: {
            /* Complex dependency chain */
            long d1 = base;
            long d2 = d1 * 3;
            long d3 = d2 + d1;
            long d4 = d3 - d2;
            long d5 = d4 * d3;
            long d6 = d5 / (d1 + 1);
            long d7 = d6 | d5;
            long d8 = d7 & d6;
            long d9 = d8 ^ d7;
            long d10 = d9 << 1;
            result = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
            break;
        }
    }
    
    return result;
}

/* Main function that ensures all code paths are executed */
int main(int argc, char *argv[]) {
    int result_int = 0;
    float result_float = 0.0f;
    double result_double = 0.0;
    long result_long = 0L;
    
    /* Use command line arguments or defaults to create dynamic inputs */
    int input1 = g_input1;
    int input2 = g_input2;
    float input3 = g_input3;
    float input4 = g_input4;
    
    if (argc > 1) input1 = atoi(argv[1]);
    if (argc > 2) input2 = atoi(argv[2]);
    if (argc > 3) input3 = atof(argv[3]);
    if (argc > 4) input4 = atof(argv[4]);
    
    /* Call all complex functions to trigger scheduler */
    result_int = complex_integer_ops(input1, input2, input1 + input2, input1 - input2);
    result_float = complex_float_ops(input3, input4, input3 * 2.0f, input4 / 2.0f);
    result_double = mixed_operations(input1, input3, (double)input2, input1 % 10);
    result_long = switch_based_computation(input1 % 4, (long)input1 * 1000L);
    
    /* Aggregate results to prevent dead code elimination */
    volatile int final_check = 0;
    final_check += result_int;
    final_check += (int)result_float;
    final_check += (int)result_double;
    final_check += (int)result_long;
    
    /* Print something to ensure execution */
    printf("Results: int=%d, float=%.2f, double=%.2f, long=%ld\n",
           result_int, result_float, result_double, result_long);
    printf("Final check: %d\n", final_check);
    
    return 0;
}
