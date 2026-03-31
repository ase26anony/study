/* Test program to trigger scheduler context allocation and cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Integer-heavy computation with many serial dependencies */
int func1_intensive(int a, int b, int c, int d, int e) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long chain of true data dependencies */
    v1 = a + b;
    asm volatile("" : "+r"(v1));  /* Prevent optimization */
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / (e + 1);
    v5 = v4 << 2;
    v6 = v5 ^ v1;
    v7 = v6 & 0xFF;
    v8 = v7 | v2;
    v9 = v8 + v3;
    v10 = v9 - v4;
    
    /* Anti-dependencies and output dependencies */
    v11 = v10;
    v10 = v11 + v5;  /* Anti-dependency: v10 read then written */
    v12 = v10;
    v10 = v12 * 2;   /* Another anti-dependency */
    v13 = v10;
    v10 = v13 / 3;   /* Output dependency: v10 written multiple times */
    
    /* More operations using many variables */
    v14 = v1 + v2 + v3;
    v15 = v4 * v5 * v6;
    v16 = v7 - v8 - v9;
    v17 = v10 ^ v11 ^ v12;
    v18 = v13 & v14 & v15;
    v19 = v16 | v17 | v18;
    v20 = v19 << 1;
    
    /* Control flow to create basic blocks */
    if (v20 > 1000) {
        v21 = v20 / 2;
        v22 = v21 + v1;
        v23 = v22 * v2;
    } else {
        v21 = v20 * 2;
        v22 = v21 - v1;
        v23 = v22 / (v2 + 1);
    }
    
    /* Use remaining variables */
    v24 = v23 + v3 + v4;
    v25 = v24 - v5 - v6;
    v26 = v25 * v7 * v8;
    v27 = v26 / (v9 + 1);
    v28 = v27 ^ v10 ^ v11;
    v29 = v28 & v12 & v13;
    v30 = v29 | v14 | v15;
    
    /* Final computation using all variables */
    return v30 + v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23 + v24 + v25;
}

/* Function 2: Floating-point array processing with loops */
float func2_fp_loop(float base, int iterations) {
    float arr[32];
    float sum = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1f;
    for (int i = 2; i < 32; i++) {
        /* True dependencies in loop */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
        asm volatile("" : "+r"(arr[i]));  /* Prevent optimization */
    }
    
    /* Process array with mixed operations */
    for (int i = 0; i < iterations; i++) {
        int idx = i % 32;
        
        /* Control flow inside loop */
        if (idx % 3 == 0) {
            arr[idx] = arr[idx] * 2.0f - 1.0f;
        } else if (idx % 3 == 1) {
            arr[idx] = arr[idx] / 1.5f + 0.5f;
        } else {
            arr[idx] = (arr[idx] + arr[(idx+1)%32]) * 0.75f;
        }
        
        /* Accumulate with dependency */
        sum += arr[idx];
        asm volatile("" : "+r"(sum));
    }
    
    /* Final reduction */
    float result = sum;
    for (int i = 0; i < 16; i++) {
        result = result + arr[i] - arr[31-i];
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many locals */
double func3_mixed_ops(int mode, double x, double y) {
    /* Many local variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5;
    
    /* Initial computations with type mixing */
    i1 = (int)x;
    d1 = (double)i1 * y;
    f1 = (float)d1;
    
    i2 = i1 + 10;
    d2 = d1 / 2.0;
    f2 = f1 * 1.5f;
    
    /* Complex control flow */
    switch (mode % 4) {
        case 0:
            i3 = i1 * i2;
            d3 = d1 + d2;
            f3 = f1 - f2;
            break;
        case 1:
            i3 = i1 / (i2 + 1);
            d3 = d1 - d2;
            f3 = f1 + f2;
            break;
        case 2:
            i3 = i1 ^ i2;
            d3 = d1 * d2;
            f3 = f1 / f2;
            break;
        default:
            i3 = i1 & i2;
            d3 = d1 / (d2 + 0.001);
            f3 = f1 * f2;
            break;
    }
    
    /* More operations creating dependencies */
    i4 = i3 << 2;
    d4 = d3 * 3.14159;
    f4 = f3 + 2.0f;
    
    i5 = i4 >> 1;
    d5 = d4 / 2.71828;
    f5 = f4 - 1.0f;
    
    i6 = i5 | 0xFF;
    d6 = d5 + d4;
    
    i7 = i6 & 0x0F;
    d7 = d6 - d5;
    
    i8 = i7 * i6;
    d8 = d7 * d6;
    
    i9 = i8 / (i7 + 1);
    d9 = d8 / (d7 + 0.001);
    
    i10 = i9 + i8 + i7;
    d10 = d9 + d8 + d7;
    
    /* Use all variables in final computation */
    double result = d10 + (double)i10 + (double)f5;
    
    /* Additional control flow */
    if (result > 100.0) {
        for (int i = 0; i < 5; i++) {
            result = result * 0.9;
            asm volatile("" : "+r"(result));
        }
    } else {
        for (int i = 0; i < 3; i++) {
            result = result + 10.0;
            asm volatile("" : "+r"(result));
        }
    }
    
    return result;
}

/* Function 4: Switch statement with different operation blocks */
long func4_switch_blocks(int selector, long val1, long val2) {
    long result = 0;
    
    switch (selector % 6) {
        case 0: {
            /* Block with arithmetic dependencies */
            long a = val1 + val2;
            long b = a * 3;
            long c = b - val1;
            long d = c / 2;
            long e = d ^ val2;
            result = e << 1;
            asm volatile("" : "+r"(result));
            break;
        }
        case 1: {
            /* Block with bit operations */
            long a = val1 & 0xAAAAAAAA;
            long b = val2 | 0x55555555;
            long c = a ^ b;
            long d = ~c;
            long e = d << 4;
            long f = e >> 2;
            result = f + a + b;
            asm volatile("" : "+r"(result));
            break;
        }
        case 2: {
            /* Block with mixed operations */
            long a = val1 * val2;
            long b = a + 12345;
            long c = b - 6789;
            long d = c & 0xFF;
            long e = d | 0x80;
            result = e * 2;
            asm volatile("" : "+r"(result));
            break;
        }
        case 3: {
            /* Block with sequential dependencies */
            long t = val1;
            for (int i = 0; i < 8; i++) {
                t = t * 3 + i;
                asm volatile("" : "+r"(t));
            }
            result = t + val2;
            break;
        }
        case 4: {
            /* Block with conditional operations */
            long a = (val1 > val2) ? val1 : val2;
            long b = (a % 2 == 0) ? a * 2 : a / 2;
            long c = b + (val1 & val2);
            result = c * c;
            asm volatile("" : "+r"(result));
            break;
        }
        default: {
            /* Default block with many operations */
            long a = val1 + 1;
            long b = val2 - 1;
            long c = a * b;
            long d = c / (a + 1);
            long e = d | a;
            long f = e & b;
            long g = f ^ c;
            result = g + d + e + f;
            asm volatile("" : "+r"(result));
            break;
        }
    }
    
    return result;
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int input1, input2, input3, input4, input5;
    volatile float finput;
    volatile double dinput1, dinput2;
    volatile long linput1, linput2;
    
    /* Initialize with non-constant values */
    if (argc > 1) {
        input1 = atoi(argv[1]);
        input2 = atoi(argv[argc > 2 ? 2 : 1]);
        input3 = atoi(argv[argc > 3 ? 3 : 1]);
        input4 = atoi(argv[argc > 4 ? 4 : 1]);
        input5 = atoi(argv[argc > 5 ? 5 : 1]);
        finput = (float)atof(argv[argc > 6 ? 6 : "1.5"]);
        dinput1 = atof(argv[argc > 7 ? 7 : "2.718"]);
        dinput2 = atof(argv[argc > 8 ? 8 : "3.141"]);
        linput1 = atol(argv[argc > 9 ? 9 : "123456"]);
        linput2 = atol(argv[argc > 10 ? 10 : "654321"]);
    } else {
        /* Use time-based values if no arguments */
        srand(time(NULL));
        input1 = rand() % 100 + 1;
        input2 = rand() % 100 + 1;
        input3 = rand() % 100 + 1;
        input4 = rand() % 100 + 1;
        input5 = rand() % 100 + 1;
        finput = (float)(rand() % 100) / 10.0f + 1.0f;
        dinput1 = (double)(rand() % 100) / 10.0 + 1.0;
        dinput2 = (double)(rand() % 100) / 10.0 + 1.0;
        linput1 = rand() % 10000 + 1000;
        linput2 = rand() % 10000 + 1000;
    }
    
    /* Call all functions to trigger scheduler */
    int result1 = func1_intensive(input1, input2, input3, input4, input5);
    float result2 = func2_fp_loop(finput, input1 + 10);
    double result3 = func3_mixed_ops(input2, dinput1, dinput2);
    long result4 = func4_switch_blocks(input3, linput1, linput2);
    
    /* Aggregate results to prevent dead code elimination */
    volatile long final_result = 0;
    final_result += result1;
    final_result += (long)result2;
    final_result += (long)result3;
    final_result += result4;
    
    /* Print to ensure code isn't optimized away */
    printf("Results: %d, %.2f, %.2f, %ld\n", 
           result1, result2, result3, result4);
    printf("Final aggregate: %ld\n", final_result);
    
    return 0;
}
