/* test_scheduler_context.c
 * Complex program to trigger scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile inputs to prevent constant propagation */
volatile int g_input1 = 42;
volatile int g_input2 = 73;
volatile float g_input3 = 3.14159f;
volatile float g_input4 = 2.71828f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_int_chain(int a, int b, int c, int d, int e, int f) {
    /* Create many local variables for register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initial computations with dependencies */
    v1 = a + b;
    v2 = c * d;
    v3 = v1 - v2;
    
    /* Inline assembly to create artificial dependency */
    asm volatile("" : "+r"(v3));
    
    v4 = v3 >> 2;
    v5 = v4 | e;
    v6 = v5 & f;
    v7 = v6 ^ a;
    
    /* More arithmetic chain */
    v8 = v7 * 17;
    v9 = v8 / 3;
    v10 = v9 + b;
    v11 = v10 - c;
    v12 = v11 * d;
    v13 = v12 % 7;
    v14 = v13 << 1;
    v15 = v14 | e;
    v16 = v15 ^ f;
    
    /* Another assembly barrier */
    asm volatile("" : "+r"(v16));
    
    /* Continue the chain */
    v17 = v16 + a;
    v18 = v17 * b;
    v19 = v18 - c;
    v20 = v19 / d;
    v21 = v20 | e;
    v22 = v21 ^ f;
    v23 = v22 + v1;
    v24 = v23 * v2;
    v25 = v24 - v3;
    v26 = v25 / v4;
    v27 = v26 | v5;
    v28 = v27 ^ v6;
    v29 = v28 + v7;
    v30 = v29 * v8;
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* Branch with more computations */
        v30 = v30 / 2;
        v29 = v29 + 100;
        v28 = v28 * 3;
    } else {
        /* Alternative branch */
        v30 = v30 * 2;
        v29 = v29 - 50;
        v28 = v28 / 2;
    }
    
    /* Final computation using all variables */
    return v30 + v29 + v28 + v27 + v26 + v25 + v24 + v23 + v22 + v21 +
           v20 + v19 + v18 + v17 + v16 + v15 + v14 + v13 + v12 + v11 +
           v10 + v9 + v8 + v7 + v6 + v5 + v4 + v3 + v2 + v1;
}

/* Function 2: Floating-point array processing with loops */
float floating_point_processing(float base, int iterations) {
    /* Many local float variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    float arr[20];
    float result = 0.0f;
    
    /* Initialize array with dependencies */
    f1 = base;
    for (int i = 0; i < 20; i++) {
        arr[i] = f1 * i;
        f1 = f1 + 0.5f;
    }
    
    /* Complex floating-point computations */
    f2 = arr[0] * arr[1];
    f3 = arr[2] / arr[3];
    f4 = f2 + f3;
    
    /* Assembly barrier */
    asm volatile("" : "+r"(f4));
    
    f5 = arr[4] - arr[5];
    f6 = arr[6] * arr[7];
    f7 = f5 / f6;
    f8 = f4 + f7;
    f9 = arr[8] * arr[9];
    f10 = arr[10] / arr[11];
    f11 = f9 - f10;
    f12 = f8 * f11;
    f13 = arr[12] + arr[13];
    f14 = arr[14] - arr[15];
    f15 = f13 / f14;
    f16 = f12 + f15;
    f17 = arr[16] * arr[17];
    f18 = arr[18] / arr[19];
    f19 = f17 - f18;
    f20 = f16 * f19;
    
    /* Loop with conditional inside */
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            /* Even iteration operations */
            f20 = f20 * 1.1f;
            f19 = f19 + 0.1f;
            result += f20;
        } else {
            /* Odd iteration operations */
            f20 = f20 / 1.1f;
            f19 = f19 - 0.1f;
            result -= f19;
        }
        
        /* More computations in loop */
        f18 = f17 * 0.9f;
        f17 = f16 + f15;
        f16 = f15 - f14;
        f15 = f14 / f13;
    }
    
    return result + f20 + f19 + f18 + f17 + f16 + f15 + f14 + f13 + f12 + f11 +
           f10 + f9 + f8 + f7 + f6 + f5 + f4 + f3 + f2 + f1;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int mode, double x, double y) {
    /* Declare many variables of different types */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5;
    
    /* Initial mixed computations */
    d1 = x + y;
    i1 = (int)(x * 100);
    f1 = (float)(y / 2.0);
    
    d2 = d1 * 2.0;
    i2 = i1 >> 2;
    f2 = f1 + 1.5f;
    
    /* Complex control flow */
    switch (mode % 4) {
        case 0:
            d3 = d2 * d1;
            i3 = i2 | i1;
            f3 = f2 * f1;
            d4 = d3 / 3.14159;
            break;
        case 1:
            d3 = d2 / d1;
            i3 = i2 & i1;
            f3 = f2 / f1;
            d4 = d3 * 2.71828;
            break;
        case 2:
            d3 = d2 + d1;
            i3 = i2 ^ i1;
            f3 = f2 + f1;
            d4 = d3 - 1.41421;
            break;
        case 3:
            d3 = d2 - d1;
            i3 = i2 + i1;
            f3 = f2 - f1;
            d4 = d3 + 1.73205;
            break;
    }
    
    /* More computations after switch */
    d5 = d4 * d3;
    i4 = i3 * 7;
    f4 = f3 * 2.0f;
    
    /* Assembly barrier */
    asm volatile("" : "+r"(d5), "+r"(i4));
    
    d6 = d5 / 4.0;
    i5 = i4 % 13;
    f5 = f4 / 3.0f;
    
    d7 = d6 + d5;
    i6 = i5 | i4;
    
    d8 = d7 * d6;
    i7 = i6 & i5;
    
    d9 = d8 - d7;
    i8 = i7 ^ i6;
    
    d10 = d9 / d8;
    i9 = i8 + i7;
    i10 = i9 * 2;
    
    /* Final result mixing all types */
    return d10 + (double)i10 + (double)f5;
}

/* Function 4: Memory-intensive with pointer chasing */
int memory_access_pattern(int *arr, int size) {
    int sum = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    int i, j, k;
    
    /* Nested loops with memory dependencies */
    for (i = 1; i < size - 1; i++) {
        /* True data dependency through memory */
        arr[i] = arr[i-1] + arr[i] * 2;
        
        /* Anti-dependency */
        tmp1 = arr[i];
        arr[i] = arr[i+1] - 3;
        tmp2 = tmp1;
        
        /* Output dependency */
        tmp3 = arr[i];
        arr[i] = tmp2 + tmp3;
        
        /* More computations */
        for (j = 0; j < 3; j++) {
            tmp4 = arr[i] * j;
            sum += tmp4;
            
            /* Small inner loop */
            for (k = 0; k < 2; k++) {
                tmp5 = tmp4 >> k;
                sum -= tmp5;
            }
        }
        
        /* Conditional inside outer loop */
        if (i % 5 == 0) {
            arr[i] = arr[i] * 2;
            sum += 100;
        } else if (i % 3 == 0) {
            arr[i] = arr[i] / 2;
            sum -= 50;
        }
        
        /* Assembly barrier every 10 iterations */
        if (i % 10 == 0) {
            asm volatile("" : "+r"(sum));
        }
    }
    
    return sum;
}

/* Main function to drive all computations */
int main(int argc, char *argv[]) {
    int result1, result4;
    float result2;
    double result3;
    int array[100];
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 100; i++) {
        array[i] = i * g_input1 + g_input2;
    }
    
    /* Call all complex functions with dynamic inputs */
    result1 = complex_int_chain(g_input1, g_input2, 
                               g_input1 + 1, g_input2 - 1,
                               g_input1 * 2, g_input2 / 2);
    
    result2 = floating_point_processing(g_input3, 25);
    
    result3 = mixed_operations(result1 % 4, 
                              (double)g_input3, 
                              (double)g_input4);
    
    result4 = memory_access_pattern(array, 100);
    
    /* Aggregate results into volatile sink to prevent elimination */
    volatile int final_result = 0;
    final_result += result1;
    final_result += (int)result2;
    final_result += (int)result3;
    final_result += result4;
    
    /* Print to ensure code isn't dead */
    printf("Final checksum: %d\n", final_result);
    
    return 0;
}
