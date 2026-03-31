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
volatile double g_input4 = 2.71828;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_integer_ops(int a, int b, int c) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long dependency chain with RAW dependencies */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - a;
    v4 = v3 / (b + 1);
    v5 = v4 << 2;
    v6 = v5 ^ v3;
    v7 = v6 | v2;
    v8 = v7 & 0xFF;
    v9 = v8 * v1;
    v10 = v9 - v4;
    
    /* Anti-dependencies (WAR) */
    v11 = v10 + 1;  /* Read v10 */
    v10 = v11 * 2;  /* Write v10 - anti-dependency with previous read */
    
    /* Output dependencies (WAW) */
    v12 = v11 + v9;
    v12 = v12 * 3;  /* Second write to v12 - output dependency */
    
    /* More operations with mixed dependencies */
    v13 = v12 >> 1;
    v14 = v13 + v8;
    v15 = v14 * v7;
    v16 = v15 - v6;
    v17 = v16 / (v5 + 1);
    v18 = v17 ^ v4;
    v19 = v18 | v3;
    v20 = v19 & 0x7F;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 * 2;
    v22 = v21 + v19;
    v23 = v22 - v18;
    v24 = v23 / (v17 + 1);
    v25 = v24 << 1;
    v26 = v25 ^ v16;
    v27 = v26 | v15;
    v28 = v27 & 0x3F;
    v29 = v28 * v14;
    v30 = v29 - v13;
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* Branch with more operations */
        v30 = v30 * 2;
        v29 = v29 + v28;
    } else {
        /* Alternative branch with different operations */
        v30 = v30 / 2;
        v29 = v29 - v28;
    }
    
    /* Another artificial dependency barrier */
    asm volatile("" : "+r"(v29), "+r"(v30));
    
    /* Final computation using most variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float complex_float_ops(float a, float b, int n) {
    float arr[32];
    float sum = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = a;
    arr[1] = b;
    for (int i = 2; i < 32; i++) {
        /* RAW dependency through array */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Loop with conditional inside - creates complex control flow */
    for (int i = 0; i < n; i++) {
        if (i % 4 == 0) {
            /* Block A: FP operations */
            arr[i % 32] = arr[i % 32] * 1.1f + sinf(arr[(i+1) % 32]);
        } else if (i % 4 == 1) {
            /* Block B: Different FP operations */
            arr[i % 32] = arr[i % 32] / 1.1f - cosf(arr[(i+2) % 32]);
        } else if (i % 4 == 2) {
            /* Block C: More operations */
            arr[i % 32] = sqrtf(fabsf(arr[i % 32])) + arr[(i+3) % 32];
        } else {
            /* Block D: Mixed operations */
            arr[i % 32] = powf(arr[i % 32], 1.5f) * expf(arr[(i+4) % 32]);
        }
        
        /* Anti-dependency in loop */
        float temp = arr[i % 32];
        arr[i % 32] = temp * 0.9f;  /* WAR: read temp, then write arr[i%32] */
        
        sum += arr[i % 32];
    }
    
    /* Dependency chain after loop */
    float result = sum;
    for (int i = 0; i < 8; i++) {
        result = result * 1.01f - 0.5f;
        result = sinf(result) + cosf(result);
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many locals */
double mixed_operations(int a, float b, double c) {
    /* Many local variables of different types */
    int i1 = a, i2 = a * 2, i3 = a + 1, i4 = a - 1, i5 = a * 3;
    int i6, i7, i8, i9, i10, i11, i12, i13, i14, i15;
    float f1 = b, f2 = b * 2.0f, f3 = b + 1.0f, f4 = b - 1.0f;
    float f5, f6, f7, f8, f9, f10;
    double d1 = c, d2 = c * 2.0, d3 = c + 1.0, d4 = c - 1.0;
    double d5, d6, d7, d8, d9, d10;
    
    /* Complex control flow with switch */
    switch (a % 5) {
        case 0:
            /* Integer operations block */
            i6 = i1 + i2;
            i7 = i3 * i4;
            i8 = i5 - i6;
            i9 = i7 / (i8 + 1);
            i10 = i9 << (i1 % 4);
            f5 = f1 * f2;
            d5 = d1 + d2;
            break;
        case 1:
            /* Float operations block */
            f5 = f1 + f2;
            f6 = f3 * f4;
            f7 = f5 - f6;
            f8 = f7 / (f1 + 0.001f);
            i6 = (int)(f8 * 100);
            d5 = d3 - d4;
            break;
        case 2:
            /* Double operations block */
            d5 = d1 * d2;
            d6 = d3 + d4;
            d7 = d5 - d6;
            d8 = d7 / (d1 + 0.0001);
            f5 = (float)d8;
            i6 = i2 * i3;
            break;
        case 3:
            /* Mixed type conversions */
            i6 = (int)(b * 100);
            f5 = (float)a;
            d5 = (double)(a + b);
            i7 = i6 + (int)d5;
            f6 = f5 + (float)i7;
            d6 = d5 + (double)f6;
            break;
        default:
            /* All types operations */
            i6 = i1 * i2 + i3 - i4;
            f5 = f1 / f2 * f3 - f4;
            d5 = d1 * d2 + d3 / d4;
            break;
    }
    
    /* More operations after switch */
    i11 = i6 * 2;
    f6 = f5 * 1.5f;
    d6 = d5 * 1.25;
    
    /* Nested if-else for more basic blocks */
    if (i11 > 100) {
        if (f6 > 50.0f) {
            i12 = i11 + (int)f6;
            d7 = d6 + (double)i12;
        } else {
            i12 = i11 - (int)f6;
            d7 = d6 - (double)i12;
        }
        f7 = f6 * 2.0f;
    } else {
        if (d6 > 100.0) {
            i12 = i11 * 2;
            d7 = d6 / 2.0;
        } else {
            i12 = i11 / 2;
            d7 = d6 * 2.0;
        }
        f7 = f6 / 2.0f;
    }
    
    /* Final dependency chain */
    i13 = i12 + a;
    f8 = f7 + b;
    d8 = d7 + c;
    
    i14 = i13 * 3;
    f9 = f8 * 1.1f;
    d9 = d8 * 1.01;
    
    i15 = i14 - i13;
    f10 = f9 - f8;
    d10 = d9 - d8;
    
    /* Artificial dependency barrier */
    asm volatile("" : "+r"(i15), "+r"(f10), "+r"(d10));
    
    return (double)i15 + (double)f10 + d10;
}

/* Function 4: Switch statement with complex blocks per case */
long switch_complex(int selector, long base) {
    long result = base;
    
    switch (selector % 6) {
        case 0: {
            /* Block with many integer operations */
            long a = result, b = result * 2, c = result + 1;
            long d = a + b, e = b - c, f = c * a;
            long g = d / (e + 1), h = e << 2, i = f ^ 0xABCD;
            long j = g | h, k = i & 0xFF, l = j - k;
            long m = l * 3, n = m / 2, o = n + result;
            result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o;
            break;
        }
        case 1: {
            /* Block with memory access pattern */
            long temp[8];
            for (int idx = 0; idx < 8; idx++) {
                temp[idx] = result + idx;
            }
            for (int idx = 1; idx < 8; idx++) {
                temp[idx] = temp[idx-1] * 2 + temp[idx];
            }
            result = 0;
            for (int idx = 0; idx < 8; idx++) {
                result += temp[idx];
            }
            break;
        }
        case 2: {
            /* Block with mixed operations and control flow */
            long x = result;
            for (int iter = 0; iter < 10; iter++) {
                if (x % 2 == 0) {
                    x = x * 3 + 1;
                } else {
                    x = x / 2;
                }
                x = x ^ (x << 13);
                x = x ^ (x >> 17);
                x = x ^ (x << 5);
            }
            result = x;
            break;
        }
        case 3: {
            /* Block with dependency chain */
            long chain[10];
            chain[0] = result;
            for (int i = 1; i < 10; i++) {
                chain[i] = chain[i-1] * 6364136223846793005ULL + 1442695040888963407ULL;
            }
            result = chain[9];
            break;
        }
        case 4: {
            /* Block with many temporary variables */
            long t1 = result, t2 = t1 + 1, t3 = t2 * 2, t4 = t3 - 1;
            long t5 = t4 / 2, t6 = t5 << 3, t7 = t6 >> 1, t8 = t7 ^ t1;
            long t9 = t8 | t2, t10 = t9 & t3, t11 = t10 + t4, t12 = t11 - t5;
            long t13 = t12 * t6, t14 = t13 / t7, t15 = t14 ^ t8, t16 = t15 | t9;
            long t17 = t16 & t10, t18 = t17 + t11, t19 = t18 - t12, t20 = t19 * t13;
            result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                     t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
            break;
        }
        default: {
            /* Default block with arithmetic series */
            long sum = 0;
            for (int i = 0; i < 20; i++) {
                long term = result + i;
                term = term * term - term;
                sum += term;
            }
            result = sum;
            break;
        }
    }
    
    return result;
}

/* Main function that calls all complex functions */
int main(int argc, char *argv[]) {
    /* Use volatile and command line inputs to prevent optimization */
    volatile int input1 = g_input1;
    volatile int input2 = g_input2;
    volatile float input3 = g_input3;
    volatile double input4 = g_input4;
    
    if (argc > 1) {
        input1 = atoi(argv[1]);
        if (argc > 2) input2 = atoi(argv[2]);
    }
    
    /* Call all complex functions to trigger scheduler */
    int result1 = complex_integer_ops(input1, input2, input1 + input2);
    float result2 = complex_float_ops(input3, input3 * 2.0f, 50);
    double result3 = mixed_operations(input1, input3, input4);
    long result4 = switch_complex(input1, input2);
    
    /* Aggregate results to prevent dead code elimination */
    volatile double final_result = 0.0;
    final_result += (double)result1;
    final_result += (double)result2;
    final_result += result3;
    final_result += (double)result4;
    
    /* Print checksum to ensure all computations are used */
    printf("Result checksum: %f\n", final_result);
    
    return 0;
}
