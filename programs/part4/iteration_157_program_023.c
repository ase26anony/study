/* test_scheduler_context.c
 * 
 * This program creates complex basic blocks with various dependencies,
 * control flow, and register pressure to force GCC's instruction scheduler
 * to allocate and later free scheduling context during compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile inputs to prevent constant propagation */
volatile int g_input1 = 42;
volatile int g_input2 = 17;
volatile float g_input3 = 3.14159f;
volatile double g_input4 = 2.71828;

/* Function 1: Integer-heavy computation with many serial dependencies */
int integer_heavy_computation(int a, int b, int c) {
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
    v6 = v5 ^ v3;
    v7 = v6 | v2;
    v8 = v7 & 0xFF;
    v9 = v8 + v1;
    v10 = v9 - v4;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10;          /* WAW if v11 was used before */
    v12 = v11 + v2;     /* RAW on v11, WAR on v2 */
    v2 = v12 * 3;       /* WAR on v2, WAW on v2 */
    v13 = v2 >> 1;      /* RAW on v2 */
    v14 = v13 & v11;    /* RAW on v13 and v11 */
    v15 = v14 | v10;    /* RAW on v14 and v10 */
    
    /* More complex chains */
    v16 = (v15 + v13) * (v12 - v11);
    v17 = v16 % 256;
    v18 = v17 ^ v15;
    v19 = v18 + v14;
    v20 = v19 - v13;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 * 2;
    v22 = v21 / 3;
    v23 = v22 + v19;
    v24 = v23 - v18;
    v25 = v24 ^ v17;
    v26 = v25 | v16;
    v27 = v26 & 0xFFFF;
    v28 = v27 << 4;
    v29 = v28 >> 2;
    v30 = v29 + v15;
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* Branch with its own dependencies */
        v30 = v30 * 2 + v1;
        v29 = v29 / 2 - v2;
    } else {
        /* Alternative branch */
        v30 = v30 / 2 + v3;
        v29 = v29 * 2 - v4;
    }
    
    /* Final computation using most variables */
    return v30 + v29 + v28 + v27 + v26 + v25 + v24 + v23 + v22 + v21 +
           v20 + v19 + v18 + v17 + v16 + v15 + v14 + v13 + v12 + v11 +
           v10 + v9 + v8 + v7 + v6 + v5 + v4 + v3 + v2 + v1;
}

/* Function 2: Floating-point array processing with loops */
float floating_point_processing(float a, float b, int n) {
    /* Local array to create memory dependencies */
    float arr[32];
    float sum = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = a;
    arr[1] = b;
    
    /* Loop with data dependencies across iterations */
    for (int i = 2; i < 32 && i < n; i++) {
        /* RAW: arr[i] depends on arr[i-1] and arr[i-2] */
        arr[i] = arr[i-1] * 1.5f + arr[i-2] * 0.5f;
        
        /* Anti-dependency: sum is read then written */
        sum = sum + arr[i];
        
        /* Output dependency: arr[i-1] modified */
        arr[i-1] = arr[i-1] * 0.9f;
    }
    
    /* Complex floating-point operations */
    float t1 = sum * a;
    float t2 = t1 / b;
    float t3 = t2 + sum;
    float t4 = t3 - a;
    float t5 = t4 * b;
    float t6 = t5 / t3;
    float t7 = t6 + t4;
    float t8 = t7 - t2;
    float t9 = t8 * t1;
    float t10 = t9 / t5;
    
    /* Mix with integer operations */
    int it1 = (int)t10;
    int it2 = it1 * 3;
    float t11 = t10 + (float)it2;
    
    /* Inline assembly barrier */
    asm volatile("" : "+r"(it2), "+f"(t11));
    
    return t11 + sum;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int a, float b, double c) {
    /* Declare many variables of different types */
    int i1 = a, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1 = b, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1 = c, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Complex control flow with dependencies */
    if (a > 0) {
        /* Block A: Integer-heavy path */
        i2 = i1 * 2;
        i3 = i2 + 100;
        i4 = i3 / 3;
        i5 = i4 << 1;
        f2 = f1 * 2.0f;
        d2 = d1 + 1.0;
        
        /* Cross-type dependencies */
        i6 = i5 + (int)f2;
        f3 = f2 + (float)i4;
        d3 = d2 * (double)i3;
    } else {
        /* Block B: Float-heavy path */
        f2 = f1 / 2.0f;
        f3 = f2 * 3.14159f;
        f4 = f3 - f1;
        i2 = i1 - 50;
        i3 = i2 * 3;
        d2 = d1 * 2.71828;
        
        i4 = i3 + (int)f4;
        f5 = f4 + (float)i2;
        d3 = d2 / (double)f3;
    }
    
    /* Common code with many operations */
    i7 = i4 ^ i3;
    i8 = i7 | i2;
    i9 = i8 & 0xFF;
    i10 = i9 << 2;
    
    f6 = f3 * f2;
    f7 = f6 / f1;
    f8 = f7 + f4;
    f9 = f8 - f3;
    f10 = f9 * 2.0f;
    
    d4 = d3 + d2;
    d5 = d4 * d1;
    d6 = d5 / 3.0;
    d7 = d6 - d3;
    d8 = d7 * 2.71828;
    d9 = d8 + d4;
    d10 = d9 / 1.41421;
    
    /* Another control flow */
    switch (a % 4) {
        case 0:
            i10 = i10 + (int)d10;
            f10 = f10 * (float)i9;
            break;
        case 1:
            i10 = i10 - (int)f10;
            d10 = d10 * (double)i8;
            break;
        case 2:
            f10 = f10 + (float)d10;
            i10 = i10 ^ (int)f9;
            break;
        default:
            d10 = d10 - (double)f10;
            i10 = i10 | (int)d9;
            break;
    }
    
    /* Final mix */
    return (double)i10 + (double)f10 + d10;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(int mode, long base) {
    long result = base;
    
    switch (mode % 5) {
        case 0: {
            /* Integer arithmetic chain */
            long a = result * 3;
            long b = a + 1000;
            long c = b / 7;
            long d = c << 3;
            long e = d ^ 0xABCD;
            long f = e | 0x1234;
            long g = f & 0xFF00;
            long h = g + a;
            long i = h - c;
            long j = i * 2;
            result = j;
            break;
        }
        case 1: {
            /* Memory access pattern */
            long temp[8];
            for (int k = 0; k < 8; k++) {
                temp[k] = result + k * 100;
            }
            for (int k = 1; k < 8; k++) {
                temp[k] = temp[k] + temp[k-1];
            }
            result = temp[7];
            break;
        }
        case 2: {
            /* Mixed operations */
            double d = (double)result;
            d = d * 1.234;
            d = d + 567.89;
            d = d / 2.345;
            float f = (float)d;
            f = f * 3.14159f;
            int i = (int)f;
            i = i ^ 0xF0F0;
            i = i << 4;
            result = (long)i + (long)d;
            break;
        }
        case 3: {
            /* Complex dependency chain */
            long x = result;
            for (int k = 0; k < 10; k++) {
                x = (x * 1103515245 + 12345) & 0x7FFFFFFF;
                x = x ^ (x >> 16);
                x = x + k;
            }
            result = x;
            break;
        }
        default: {
            /* Simple but many operations */
            result = result + 1;
            result = result * 2;
            result = result - 3;
            result = result / 4;
            result = result << 5;
            result = result >> 2;
            result = result | 0xFF;
            result = result & 0xFFFF;
            result = result ^ 0xAAAA;
            result = result + 0x5555;
            break;
        }
    }
    
    return result;
}

/* Main function that ensures all code is executed */
int main(int argc, char *argv[]) {
    /* Use command line arguments or stdin to get dynamic values */
    int input1, input2;
    float input3;
    double input4;
    
    if (argc >= 3) {
        input1 = atoi(argv[1]);
        input2 = atoi(argv[2]);
        input3 = atof(argv[3]);
        input4 = atof(argv[4]);
    } else {
        /* Use volatile globals to prevent optimization */
        input1 = g_input1;
        input2 = g_input2;
        input3 = g_input3;
        input4 = g_input4;
    }
    
    /* Call all complex functions */
    int res1 = integer_heavy_computation(input1, input2, input1 + input2);
    float res2 = floating_point_processing(input3, input3 * 2.0f, 20);
    double res3 = mixed_operations(input1, input3, input4);
    long res4 = switch_based_computation(input1, input2);
    
    /* Aggregate results to a volatile sink */
    volatile double final_result = 0.0;
    final_result += (double)res1;
    final_result += (double)res2;
    final_result += res3;
    final_result += (double)res4;
    
    /* Print checksum to ensure all computations are used */
    printf("Result checksum: %f\n", final_result);
    
    return 0;
}
