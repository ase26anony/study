/* test_scheduler_context.c
 * Complex program to trigger GCC scheduler context allocation and cleanup
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
int complex_integer_chain(int a, int b, int c, int d, int e) {
    /* Create register pressure with many variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long dependency chain with RAW dependencies */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 << 2;
    v6 = v5 ^ 0xABCD;
    v7 = v6 & 0xFF00;
    v8 = v7 | 0x00FF;
    v9 = v8 >> 1;
    v10 = v9 % 256;
    
    /* Anti-dependencies (WAR) */
    v11 = v10 + 1;
    v10 = v11 * 2;  /* WAR: v10 written after v11 reads old v10 */
    
    /* Output dependencies (WAW) */
    v12 = v11 - 5;
    v12 = v12 + 10;  /* WAW: v12 written twice */
    
    /* More complex operations */
    v13 = (v12 << 3) | (v11 >> 2);
    v14 = v13 ^ v12;
    v15 = v14 * v13;
    v16 = v15 / (v14 + 1);
    v17 = v16 % 100;
    v18 = v17 & 0x0F0F;
    v19 = v18 | 0xF0F0;
    v20 = v19 ^ 0xAAAA;
    
    /* Control flow to create multiple basic blocks */
    if (v20 > 1000) {
        v21 = v20 * 2;
        v22 = v21 + 100;
        v23 = v22 - 50;
    } else {
        v21 = v20 / 2;
        v22 = v21 - 100;
        v23 = v22 + 50;
    }
    
    /* Inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v23));
    
    /* Continue dependency chain */
    v24 = v23 * 3;
    v25 = v24 + v22;
    v26 = v25 - v21;
    v27 = v26 / 4;
    v28 = v27 << 1;
    v29 = v28 ^ v27;
    v30 = v29 | v28;
    
    /* Final computation using many variables */
    return v1 + v5 + v10 + v15 + v20 + v25 + v30;
}

/* Function 2: Floating-point array processing with loops */
float floating_point_processing(float base, int iterations) {
    /* Many local variables for register pressure */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    float arr[20];
    float result = 0.0f;
    
    /* Initialize array with dependencies */
    f1 = base;
    for (int i = 0; i < 20; i++) {
        arr[i] = f1 + i * 0.1f;
        f1 = arr[i] * 1.1f;  /* RAW dependency */
    }
    
    /* Mixed integer/float operations */
    f2 = base * 2.0f;
    f3 = sinf(f2);
    f4 = cosf(f3);
    f5 = f4 * f3;
    f6 = f5 + f4;
    f7 = f6 - f3;
    f8 = f7 / (f2 + 1.0f);
    f9 = sqrtf(fabsf(f8));
    f10 = logf(f9 + 1.0f);
    
    /* Loop with conditional inside */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            f11 = arr[i % 20] * 1.5f;
            f12 = f11 + f10;
            f13 = f12 * 0.8f;
        } else if (i % 3 == 1) {
            f11 = arr[i % 20] / 1.5f;
            f12 = f11 - f10;
            f13 = f12 * 1.2f;
        } else {
            f11 = arr[i % 20];
            f12 = f11 * f10;
            f13 = f12 / 2.0f;
        }
        
        /* More operations in loop */
        f14 = f13 + (float)i;
        f15 = sinf(f14);
        f16 = cosf(f15);
        f17 = f16 * f15;
        f18 = f17 + f14;
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : "+r"(f18));
        
        result += f18;
    }
    
    /* Final chain of operations */
    f19 = result * 0.01f;
    f20 = expf(f19);
    
    return f20;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int mode, double x, double y) {
    /* Declare many variables to stress register allocation */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    double d21, d22, d23, d24, d25, d26, d27, d28, d29, d30;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Initial computations */
    d1 = x + y;
    d2 = x * y;
    d3 = x - y;
    d4 = x / (y + 1.0);
    
    /* Complex control flow */
    switch (mode % 4) {
        case 0:
            d5 = sin(d1) * cos(d2);
            d6 = d5 + tan(d3);
            d7 = d6 * log(fabs(d4) + 1.0);
            i1 = (int)d7;
            break;
        case 1:
            d5 = cos(d1) / sin(d2);
            d6 = d5 - atan(d3);
            d7 = d6 + exp(d4);
            i1 = (int)(d7 * 100.0);
            break;
        case 2:
            d5 = tan(d1) + atan(d2);
            d6 = d5 * sqrt(d3);
            d7 = d6 / (d4 + 1.0);
            i1 = (int)(d7 * 1000.0);
            break;
        default:
            d5 = d1 * d2 + d3 * d4;
            d6 = d5 / (d1 + d2 + d3 + d4);
            d7 = pow(d6, 2.0);
            i1 = (int)d7;
            break;
    }
    
    /* More operations with dependencies */
    i2 = i1 * 2;
    i3 = i2 + 100;
    i4 = i3 - 50;
    i5 = i4 / 3;
    i6 = i5 << 2;
    i7 = i6 >> 1;
    i8 = i7 ^ 0xFF;
    i9 = i8 & 0x0F;
    i10 = i9 | 0xF0;
    
    /* Convert back to double */
    d8 = (double)i10;
    d9 = d8 * d7;
    d10 = d9 + d6;
    d11 = d10 - d5;
    d12 = d11 / d4;
    d13 = d12 * d3;
    d14 = d13 + d2;
    d15 = d14 - d1;
    
    /* Memory access pattern with address calculation */
    double temp[10];
    for (int i = 0; i < 10; i++) {
        temp[i] = d15 + i * 0.5;
        if (i > 0) {
            temp[i] = temp[i] + temp[i-1];  /* RAW dependency */
        }
    }
    
    /* Final computations using array */
    d16 = 0.0;
    for (int i = 0; i < 10; i++) {
        d16 += temp[i] * (i + 1);
    }
    
    d17 = d16 / 10.0;
    d18 = sin(d17);
    d19 = cos(d18);
    d20 = d19 * d18;
    
    /* More variables to increase register pressure */
    d21 = d20 + d17;
    d22 = d21 * 2.0;
    d23 = d22 - d16;
    d24 = d23 / 3.0;
    d25 = sqrt(fabs(d24));
    d26 = log(d25 + 1.0);
    d27 = exp(d26);
    d28 = d27 * d26;
    d29 = d28 + d25;
    d30 = d29 - d24;
    
    return d30;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(long seed, int depth) {
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    long l11, l12, l13, l14, l15, l16, l17, l18, l19, l20;
    
    l1 = seed;
    
    /* Deeply nested control flow */
    for (int i = 0; i < depth; i++) {
        switch ((l1 + i) % 5) {
            case 0:
                l2 = l1 * 3;
                l3 = l2 + 12345;
                l4 = l3 ^ 0xAAAAAAAA;
                l5 = l4 << 3;
                l6 = l5 >> 1;
                l1 = l6 % 1000000;
                break;
            case 1:
                l2 = l1 / 2;
                l3 = l2 - 54321;
                l4 = l3 | 0x55555555;
                l5 = l4 << 2;
                l6 = l5 >> 2;
                l1 = l6 + 77777;
                break;
            case 2:
                l2 = l1 + 11111;
                l3 = l2 * 7;
                l4 = l3 & 0x0F0F0F0F;
                l5 = l4 ^ l3;
                l6 = l5 | l4;
                l1 = l6 - 33333;
                break;
            case 3:
                l2 = l1 - 99999;
                l3 = l2 * 11;
                l4 = l3 % 256;
                l5 = l4 << 4;
                l6 = l5 >> 4;
                l1 = l6 * 13;
                break;
            default:
                l2 = l1 ^ 0xFFFFFFFF;
                l3 = l2 + 88888;
                l4 = l3 / 17;
                l5 = l4 << 5;
                l6 = l5 >> 3;
                l1 = l6 & 0x00FF00FF;
                break;
        }
        
        /* Inline assembly between iterations */
        asm volatile("" : "+r"(l1));
    }
    
    /* Final computation chain */
    l7 = l1 * 19;
    l8 = l7 + 1111111;
    l9 = l8 - 222222;
    l10 = l9 / 23;
    l11 = l10 << 7;
    l12 = l11 >> 5;
    l13 = l12 ^ 0xCCCCCCCC;
    l14 = l13 | 0x33333333;
    l15 = l14 & 0x0F0F0F0F;
    l16 = l15 + 444444;
    l17 = l16 - 555555;
    l18 = l17 * 29;
    l19 = l18 % 1000000007;
    l20 = l19 ^ l18;
    
    return l20;
}

/* Main function to drive all computations */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int result1, result4;
    volatile float result2;
    volatile double result3;
    volatile long result5;
    
    /* Read inputs to make them dynamic */
    int input1 = g_input1;
    int input2 = g_input2;
    float input3 = g_input3;
    float input4 = g_input4;
    
    /* Add command line variability */
    if (argc > 1) {
        input1 += atoi(argv[1]);
    }
    if (argc > 2) {
        input2 += atoi(argv[2]);
    }
    
    /* Call all complex functions */
    result1 = complex_integer_chain(input1, input2, input1+1, input2-1, 7);
    result2 = floating_point_processing(input3, 50);
    result3 = mixed_operations(input1 % 4, (double)input3, (double)input4);
    result4 = complex_integer_chain(result1, input2, input1, result1 % 100, 13);
    result5 = switch_based_computation((long)result1 * 1000, 25);
    
    /* Use results to prevent dead code elimination */
    volatile int checksum = 0;
    checksum += result1;
    checksum += (int)result2;
    checksum += (int)result3;
    checksum += result4;
    checksum += (int)result5;
    
    /* Print something to ensure execution */
    printf("Scheduler test checksum: %d\n", checksum);
    
    return 0;
}
