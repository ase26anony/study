/* test_scheduler_context.c
 * Complex program to trigger GCC's scheduler context allocation and cleanup
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched test_scheduler_context.c -o test
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
int complex_int_ops(int a, int b, int c, int d, int e, int f) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 | f;
    v6 = v5 ^ v1;
    v7 = v6 & v2;
    v8 = v7 << 3;
    v9 = v8 >> 1;
    v10 = v9 + v3;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10 * 2;
    v12 = v11 - v4;  /* WAR: v4 read before potential write */
    v4 = v12 + 5;    /* WAW: v4 written again */
    v13 = v4 * v5;
    v14 = v13 / v6;
    v15 = v14 | v7;
    
    /* More complex dependency chain */
    v16 = v15 ^ v8;
    v17 = v16 & v9;
    v18 = v17 << v10;
    v19 = v18 >> v11;
    v20 = v19 + v12;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 * v13;
    v22 = v21 - v14;
    v23 = v22 / v15;
    v24 = v23 | v16;
    v25 = v24 ^ v17;
    
    /* Control flow to create basic block boundaries */
    if (v25 > 1000) {
        v26 = v25 / 2;
        v27 = v26 * 3;
    } else {
        v26 = v25 * 2;
        v27 = v26 / 3;
    }
    
    v28 = v27 + v18;
    v29 = v28 - v19;
    v30 = v29 * v20;
    
    /* Final computation using most variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float complex_float_ops(float a, float b, float c, int n) {
    float arr[32];
    float sum = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = a;
    arr[1] = b;
    for (int i = 2; i < 32; i++) {
        /* True data dependencies in loop */
        arr[i] = arr[i-1] * c + arr[i-2];
    }
    
    /* Process array with mixed operations */
    for (int i = 0; i < n && i < 32; i++) {
        float temp;
        if (i % 3 == 0) {
            temp = arr[i] * arr[i];
        } else if (i % 3 == 1) {
            temp = sqrtf(fabsf(arr[i]));
        } else {
            temp = 1.0f / (arr[i] + 1.0f);
        }
        
        /* Anti-dependency */
        arr[i] = temp + (float)i * 0.1f;
        sum += arr[i];
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : "+r"(sum));
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_ops_with_branches(int x, double y, int z) {
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Initial computations */
    i1 = x * 2;
    d1 = y * 3.14;
    i2 = z + i1;
    d2 = d1 / 2.71;
    
    /* Complex conditional with dependencies */
    if (x > 0) {
        i3 = i1 << 2;
        d3 = d1 * d2;
        i4 = i3 ^ i2;
        d4 = sin(d3);
    } else {
        i3 = i1 >> 2;
        d3 = d1 + d2;
        i4 = i3 | i2;
        d4 = cos(d3);
    }
    
    /* Switch statement for multiple basic blocks */
    switch (z % 4) {
        case 0:
            i5 = i3 * i4;
            d5 = d3 + d4;
            break;
        case 1:
            i5 = i3 + i4;
            d5 = d3 * d4;
            break;
        case 2:
            i5 = i3 - i4;
            d5 = d3 - d4;
            break;
        default:
            i5 = i3 / (i4 + 1);
            d5 = d3 / (d4 + 1.0);
            break;
    }
    
    /* More operations with output dependencies */
    i6 = i5 * 7;
    d6 = d5 * 2.5;
    i5 = i6 + 11;      /* WAW on i5 */
    d5 = d6 - 1.2;     /* WAW on d5 */
    
    i7 = i5 & 0xFF;
    d7 = fmod(d5, 3.0);
    i8 = i7 | 0x80;
    d8 = pow(d7, 2.0);
    
    /* Final computations using all variables */
    i9 = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8;
    d9 = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
    i10 = i9 * 2;
    d10 = d9 * 1.5;
    
    return (double)i10 + d10;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(int mode, long base) {
    long result = base;
    
    switch (mode % 5) {
        case 0: {
            /* Integer arithmetic chain */
            long a = result * 3;
            long b = a + 7;
            long c = b << 2;
            long d = c ^ 0xABCD;
            long e = d / 5;
            result = e - a + b - c + d - e;
            break;
        }
        case 1: {
            /* Bit manipulation chain */
            long a = result | 0xFF00;
            long b = a & 0x0FF0;
            long c = b ^ 0x3333;
            long d = c << 4;
            long e = d >> 2;
            result = a ^ b ^ c ^ d ^ e;
            break;
        }
        case 2: {
            /* Mixed operations with memory-like pattern */
            long arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = result + i;
                if (i > 0) {
                    arr[i] += arr[i-1];
                }
            }
            result = arr[0] + arr[2] + arr[4] + arr[6];
            break;
        }
        case 3: {
            /* Complex dependency web */
            long x = result;
            long y = x * 2;
            long z = y + x;
            long w = z ^ y;
            long v = w * x;
            long u = v / (y + 1);
            result = x + y + z + w + v + u;
            break;
        }
        default: {
            /* Simple but many operations */
            result = result + 1;
            result = result * 2;
            result = result - 3;
            result = result / 4;
            result = result | 0x1000;
            result = result & 0x0FFF;
            result = result ^ 0x0555;
            result = result << 1;
            result = result >> 2;
            break;
        }
    }
    
    /* Ensure result is used */
    asm volatile("" : "+r"(result));
    return result;
}

int main(int argc, char *argv[]) {
    int result_int = 0;
    float result_float = 0.0f;
    double result_double = 0.0;
    long result_long = 0L;
    
    /* Use command line arguments or defaults to prevent constant folding */
    int input1 = (argc > 1) ? atoi(argv[1]) : g_input1;
    int input2 = (argc > 2) ? atoi(argv[2]) : g_input2;
    float input3 = (argc > 3) ? atof(argv[3]) : g_input3;
    double input4 = (argc > 4) ? atof(argv[4]) : g_input4;
    
    /* Call all complex functions to trigger scheduler in different contexts */
    result_int = complex_int_ops(input1, input2, input1+input2, 
                                 input1-input2, input1*2, input2*3);
    
    result_float = complex_float_ops(input3, input3*2.0f, input3/3.0f, 20);
    
    result_double = mixed_ops_with_branches(input1, input4, input2);
    
    result_long = switch_based_computation(input1, input2);
    
    /* Aggregate results to prevent dead code elimination */
    volatile int final_check = 0;
    final_check += result_int;
    final_check += (int)result_float;
    final_check += (int)result_double;
    final_check += (int)result_long;
    
    /* Print to ensure all computations are used */
    printf("Results: int=%d, float=%.2f, double=%.2f, long=%ld, check=%d\n",
           result_int, result_float, result_double, result_long, final_check);
    
    return 0;
}
