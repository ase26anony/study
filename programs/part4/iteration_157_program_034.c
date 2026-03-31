/* test_scheduler_context.c
 * Complex program to trigger GCC's scheduler context allocation and cleanup
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile inputs to prevent constant propagation */
volatile int g_input1 = 7;
volatile int g_input2 = 13;
volatile float g_input3 = 3.14159f;
volatile double g_input4 = 2.71828;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_int_operations(int a, int b, int c) {
    /* Create register pressure with many variables */
    int v1 = a + b;
    int v2 = b * c;
    int v3 = v1 ^ v2;
    int v4 = v3 << 2;
    int v5 = v4 - a;
    int v6 = v5 * b;
    int v7 = v6 / (c + 1);
    int v8 = v7 | v4;
    int v9 = v8 & 0xFF;
    int v10 = v9 + v5;
    int v11 = v10 * 3;
    int v12 = v11 - v2;
    int v13 = v12 >> 1;
    int v14 = v13 ^ v8;
    int v15 = v14 + v7;
    int v16 = v15 * 2;
    int v17 = v16 % 17;
    int v18 = v17 + v10;
    int v19 = v18 * v9;
    int v20 = v19 - v13;
    
    /* Artificial dependency chain with inline asm */
    asm volatile("" : "+r"(v20));
    
    int v21 = v20 + v6;
    int v22 = v21 * v3;
    int v23 = v22 / (v4 + 1);
    int v24 = v23 | v15;
    int v25 = v24 & 0xFFFF;
    
    /* Control flow to create basic blocks */
    if (v25 > 1000) {
        v25 = v25 * 2;
        v25 = v25 - 500;
    } else {
        v25 = v25 + 1000;
        v25 = v25 / 2;
    }
    
    return v25;
}

/* Function 2: Floating-point array processing with loops */
float complex_float_operations(float base, int iterations) {
    /* Many local variables for register pressure */
    float f1 = base;
    float f2 = base * 1.1f;
    float f3 = base * 0.9f;
    float f4 = base + 1.0f;
    float f5 = base - 0.5f;
    float f6, f7, f8, f9, f10;
    
    /* Loop with dependencies */
    for (int i = 0; i < iterations; i++) {
        f6 = f1 * f2;
        f7 = f3 + f4;
        f8 = f5 - f6;
        f9 = f7 * f8;
        f10 = f9 / (f1 + 1.0f);
        
        /* Update chain */
        f1 = f10 * 0.99f;
        f2 = f1 + f3;
        f3 = f2 * 0.95f;
        f4 = f3 - f5;
        f5 = f4 / 1.05f;
        
        /* Inline asm to prevent optimization */
        asm volatile("" : "+r"(i));
    }
    
    /* Conditional block */
    float result;
    if (f10 > 0) {
        result = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
        result = result / 10.0f;
    } else {
        result = f1 * f2 * f3 * f4 * f5;
        result = sqrtf(fabsf(result));
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many variables */
double mixed_operations(int mode, double x, double y) {
    /* Declare many variables to stress register allocation */
    double d1 = x;
    double d2 = y;
    double d3 = x + y;
    double d4 = x * y;
    double d5 = x - y;
    double d6 = x / (y + 1.0);
    double d7, d8, d9, d10, d11, d12, d13, d14, d15;
    
    int i1 = (int)x;
    int i2 = (int)y;
    int i3 = i1 + i2;
    int i4 = i1 * i2;
    int i5 = i1 - i2;
    int i6 = i1 ^ i2;
    int i7 = i1 | i2;
    int i8 = i1 & i2;
    
    /* Complex control flow */
    switch (mode % 4) {
        case 0:
            d7 = sin(d1) * cos(d2);
            d8 = exp(d3) * log(fabs(d4) + 1.0);
            d9 = d7 + d8;
            i3 = i3 * 2;
            break;
        case 1:
            d7 = tan(d1) * atan(d2);
            d8 = pow(d3, 1.5);
            d9 = d7 - d8;
            i3 = i3 + 100;
            break;
        case 2:
            d7 = d1 * d2 * d3;
            d8 = d4 + d5 + d6;
            d9 = d7 / d8;
            i3 = i3 >> 1;
            break;
        case 3:
            d7 = sqrt(d1 * d1 + d2 * d2);
            d8 = d3 * d4 * d5;
            d9 = d7 + d8;
            i3 = i3 ^ 0x55;
            break;
    }
    
    /* More operations with dependencies */
    d10 = d9 * i3;
    d11 = d10 + d1;
    d12 = d11 - d2;
    d13 = d12 * d3;
    d14 = d13 / (d4 + 1.0);
    d15 = d14 + d5 + d6 + d7 + d8 + d9;
    
    /* Inline asm to create barriers */
    asm volatile("" : "+r"(i3), "+r"(d15));
    
    return d15 * i3;
}

/* Function 4: Memory access pattern with address calculations */
void memory_intensive(int* arr, int size) {
    /* Many local variables */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int prod1 = 1, prod2 = 1;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    
    /* Loop with memory dependencies */
    for (int i = 2; i < size - 2; i++) {
        /* Read-after-write and write-after-read dependencies */
        tmp1 = arr[i-2];
        tmp2 = arr[i-1];
        tmp3 = arr[i];
        tmp4 = arr[i+1];
        tmp5 = arr[i+2];
        
        /* Computation chain */
        tmp6 = tmp1 + tmp2;
        tmp7 = tmp3 * tmp4;
        tmp8 = tmp5 - tmp6;
        
        /* Update with anti-dependencies */
        arr[i-1] = tmp6 + tmp7;  /* Write-after-read of tmp6, tmp7 */
        arr[i] = tmp8 * 2;       /* Write-after-read of tmp8 */
        
        /* Accumulate results */
        sum1 += tmp6;
        sum2 += tmp7;
        sum3 += tmp8;
        sum4 += arr[i-1] + arr[i];
        
        prod1 *= (tmp6 + 1);
        prod2 *= (tmp7 + 1);
        
        /* Prevent loop unrolling optimization */
        asm volatile("" : "+r"(i));
    }
    
    /* Use results to prevent dead code elimination */
    arr[0] = sum1 + sum2 + sum3 + sum4;
    arr[1] = prod1 + prod2;
}

/* Main function to drive execution */
int main(int argc, char** argv) {
    /* Dynamic inputs to prevent constant folding */
    int input1 = g_input1;
    int input2 = g_input2;
    float input3 = g_input3;
    double input4 = g_input4;
    
    /* Read from command line if available */
    if (argc > 1) input1 = atoi(argv[1]);
    if (argc > 2) input2 = atoi(argv[2]);
    
    printf("Starting scheduler context test...\n");
    
    /* Call all complex functions */
    int result1 = complex_int_operations(input1, input2, input1 + input2);
    printf("Result1: %d\n", result1);
    
    float result2 = complex_float_operations(input3, 50);
    printf("Result2: %f\n", result2);
    
    double result3 = mixed_operations(input1, input4, input4 * 2);
    printf("Result3: %f\n", result3);
    
    /* Memory intensive operation */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * input1 + input2;
    }
    memory_intensive(arr, 100);
    printf("Array result: %d, %d\n", arr[0], arr[1]);
    
    /* Aggregate results into volatile sink */
    volatile int final_check = result1 + (int)result2 + (int)result3 + arr[0] + arr[1];
    printf("Final check: %d\n", final_check);
    
    return 0;
}
