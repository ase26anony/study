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
int complex_int_operations(int a, int b, int c) {
    /* Create many local variables to increase register pressure */
    int v1 = a + b;
    int v2 = b * c;
    int v3 = v1 - v2;
    int v4 = v2 / (a + 1);
    int v5 = v3 ^ v4;
    int v6 = v4 | v5;
    int v7 = v5 & v6;
    int v8 = v6 << 2;
    int v9 = v7 >> 1;
    int v10 = v8 % (v9 + 1);
    int v11 = v9 * v10;
    int v12 = v10 + v11;
    int v13 = v11 - v12;
    int v14 = v12 * v13;
    int v15 = v13 / (v14 + 1);
    int v16 = v14 ^ v15;
    int v17 = v15 | v16;
    int v18 = v16 & v17;
    int v19 = v17 << 3;
    int v20 = v18 >> 2;
    
    /* Create artificial dependencies with inline assembly */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3));
    
    /* More operations with dependencies */
    int v21 = v19 + v20;
    int v22 = v20 * v21;
    int v23 = v21 - v22;
    int v24 = v22 / (v23 + 1);
    
    /* Control flow to create multiple basic blocks */
    if (v24 > 1000) {
        v24 = v24 * 2;
        v23 = v23 + v24;
        asm volatile("" : "+r"(v23), "+r"(v24));
    } else {
        v24 = v24 / 2;
        v23 = v23 - v24;
        asm volatile("" : "+r"(v23), "+r"(v24));
    }
    
    /* Final computation with all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24;
}

/* Function 2: Floating-point array processing with loops */
float complex_float_operations(float a, float b, int iterations) {
    /* Many local float variables */
    float f1 = a;
    float f2 = b;
    float f3 = f1 * f2;
    float f4 = f2 / f1;
    float f5 = f3 + f4;
    float f6 = f4 - f3;
    float f7 = f5 * f6;
    float f8 = f6 / f5;
    float f9 = f7 + f8;
    float f10 = f8 - f7;
    
    /* Array with dependencies */
    float arr[20];
    arr[0] = f1;
    arr[1] = f2;
    
    /* Loop with data dependencies */
    for (int i = 2; i < 20; i++) {
        arr[i] = arr[i-1] + arr[i-2] * (i % 3);
        /* Inline asm to prevent optimization */
        asm volatile("" : "+r"(i));
    }
    
    /* Nested loop with conditional */
    float sum = 0.0f;
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 5; j++) {
            if ((i + j) % 2 == 0) {
                sum += arr[i % 20] * arr[j % 20];
                f9 = f9 + sum * 0.5f;
            } else {
                sum -= arr[i % 20] / arr[(j + 1) % 20];
                f10 = f10 - sum * 0.25f;
            }
        }
        /* Artificial dependency chain */
        asm volatile("" : "+r"(sum));
    }
    
    return f9 + f10 + sum;
}

/* Function 3: Mixed operations with control flow and many variables */
double mixed_operations(int a, float b, double c) {
    /* Mixed type variables */
    int i1 = a;
    float f1 = b;
    double d1 = c;
    int i2 = i1 * 2;
    float f2 = f1 * 3.14f;
    double d2 = d1 / 2.71828;
    
    /* Complex dependency chain */
    for (int counter = 0; counter < 10; counter++) {
        i2 = i2 + counter;
        f2 = f2 * (1.0f + (float)counter / 100.0f);
        d2 = d2 + (double)i2 * 0.01;
        
        /* Conditional with different operation sets */
        if (counter % 3 == 0) {
            i2 = i2 ^ (i1 << 2);
            f2 = f2 + f1 * 2.0f;
            asm volatile("" : "+r"(i2), "+r"(f2));
        } else if (counter % 3 == 1) {
            i2 = i2 | (i1 >> 1);
            f2 = f2 - f1 * 0.5f;
            d2 = d2 * 1.1;
            asm volatile("" : "+r"(i2), "+r"(f2), "+r"(d2));
        } else {
            i2 = i2 & (~i1);
            f2 = f2 / (f1 + 1.0f);
            d2 = d2 / 1.05;
            asm volatile("" : "+r"(i2), "+r"(f2), "+r"(d2));
        }
    }
    
    /* More variables to increase register pressure */
    int i3 = i2 * 3;
    int i4 = i3 / 2;
    int i5 = i4 + i2;
    int i6 = i5 - i3;
    int i7 = i6 * i4;
    int i8 = i7 % (i5 + 1);
    
    float f3 = f2 * 2.0f;
    float f4 = f3 / 1.5f;
    float f5 = f4 + f2;
    float f6 = f5 - f3;
    
    double d3 = d2 * 1.5;
    double d4 = d3 / 1.2;
    double d5 = d4 + d2;
    double d6 = d5 - d3;
    
    /* Final computation using all variables */
    return (double)(i3 + i4 + i5 + i6 + i7 + i8) +
           (double)(f3 + f4 + f5 + f6) +
           (d3 + d4 + d5 + d6);
}

/* Function 4: Switch statement with different operation blocks */
long switch_based_computation(int mode, long base) {
    long result = base;
    
    switch (mode % 4) {
        case 0: {
            /* Integer arithmetic block */
            int temp[10];
            temp[0] = (int)(base & 0xFF);
            for (int i = 1; i < 10; i++) {
                temp[i] = temp[i-1] * 3 + i;
                result += temp[i];
            }
            result = result << 2;
            asm volatile("" : "+r"(result));
            break;
        }
        case 1: {
            /* Bit manipulation block */
            result = result ^ 0xAAAAAAAA;
            result = result | 0x55555555;
            result = result & 0xFFFFFFFF;
            result = result << 4;
            result = result >> 2;
            for (int i = 0; i < 8; i++) {
                result = result ^ (1 << (i * 4));
                asm volatile("" : "+r"(result));
            }
            break;
        }
        case 2: {
            /* Mixed operations block */
            float f = (float)(result % 1000);
            for (int i = 0; i < 5; i++) {
                f = f * 1.5f + (float)i;
                result += (long)f;
            }
            result = result * 3 - 7;
            asm volatile("" : "+r"(result), "+r"(f));
            break;
        }
        case 3: {
            /* Memory access pattern */
            long values[8];
            values[0] = result;
            for (int i = 1; i < 8; i++) {
                values[i] = values[i-1] * 2 + i;
            }
            for (int i = 7; i > 0; i--) {
                result += values[i] - values[i-1];
            }
            result = result % 1000000;
            asm volatile("" : "+r"(result));
            break;
        }
    }
    
    return result;
}

/* Main function that calls all complex functions */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int result_int = 0;
    volatile float result_float = 0.0f;
    volatile double result_double = 0.0;
    volatile long result_long = 0L;
    
    /* Get dynamic inputs */
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
    
    /* Call all complex functions multiple times */
    for (int i = 0; i < 3; i++) {
        result_int += complex_int_operations(input1 + i, input2 - i, i * 10);
        result_float += complex_float_operations(input3 + i, input4 - i, 5 + i);
        result_double += mixed_operations(input1 * (i + 1), input3 + i, input4 * (i + 1));
        result_long += switch_based_computation(input1 + input2 + i, result_long + i * 1000);
        
        /* Create cross-dependencies between iterations */
        asm volatile("" : "+r"(result_int), "+r"(result_float), 
                          "+r"(result_double), "+r"(result_long));
    }
    
    /* Aggregate results to prevent dead code elimination */
    volatile long final_result = (long)result_int + (long)result_float + 
                                 (long)result_double + result_long;
    
    /* Print checksum */
    printf("Result checksum: %ld\n", final_result);
    
    return 0;
}
