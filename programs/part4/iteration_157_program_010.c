/* test_scheduler_context.c
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched test_scheduler_context.c -o test_scheduler
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function 1: Integer-heavy computation with many serial dependencies */
int func1_intensive(int a, int b, int c, int d, int e) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long chain of true data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 | a;
    v6 = v5 & b;
    v7 = v6 ^ c;
    v8 = v7 << 2;
    v9 = v8 >> 1;
    v10 = v9 + v1;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10 * 3;
    v10 = v11 - 5;      /* WAR: v10 read before this write */
    v12 = v10 + v2;
    v13 = v12 * v3;
    v12 = v13 / 4;      /* WAW: v12 written twice */
    
    /* More operations with mixed dependencies */
    v14 = v12 + v4;
    v15 = v14 - v5;
    v16 = v15 * v6;
    v17 = v16 / v7;
    v18 = v17 | v8;
    v19 = v18 & v9;
    v20 = v19 ^ v10;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 + v11;
    v22 = v21 * v12;
    v23 = v22 - v13;
    v24 = v23 / v14;
    v25 = v24 | v15;
    
    /* Control flow to create multiple basic blocks */
    if (v25 > 1000) {
        v26 = v25 * 2;
        v27 = v26 - 50;
        v28 = v27 / 3;
    } else {
        v26 = v25 / 2;
        v27 = v26 + 50;
        v28 = v27 * 3;
    }
    
    v29 = v28 + v16;
    v30 = v29 * v17;
    
    /* Final result uses many variables to prevent optimization */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float func2_fp_loop(float base, int iterations) {
    float arr[32];
    float sum = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1f;
    for (int i = 2; i < 32; i++) {
        /* True dependencies between array elements */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Complex loop with conditional inside */
    for (int i = 0; i < iterations; i++) {
        float temp = arr[i % 32];
        
        /* Mixed floating-point operations */
        temp = temp * 1.5f;
        temp = temp / (i + 1.0f);
        temp = sqrtf(fabsf(temp));
        
        /* Anti-dependency */
        arr[i % 32] = temp + 0.1f;
        
        /* Output dependency */
        sum = sum + temp;
        
        /* Inline assembly to prevent reordering */
        if (i % 8 == 0) {
            asm volatile("" : "+r"(sum));
        }
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many locals */
double func3_mixed_control(int mode, double x, double y) {
    /* Many local variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5;
    
    /* Initial computations with dependencies */
    i1 = (int)x;
    i2 = (int)y;
    d1 = x * y;
    d2 = x / (y + 1.0);
    
    /* Complex control flow */
    switch (mode % 4) {
        case 0:
            i3 = i1 + i2;
            i4 = i1 * i2;
            d3 = d1 + d2;
            d4 = d1 - d2;
            f1 = (float)d3;
            f2 = (float)d4;
            break;
        case 1:
            i3 = i1 - i2;
            i4 = i1 / (i2 + 1);
            d3 = d1 * d2;
            d4 = d1 / d2;
            f1 = (float)d3 * 2.0f;
            f2 = (float)d4 / 2.0f;
            break;
        case 2:
            i3 = i1 | i2;
            i4 = i1 & i2;
            d3 = fmod(d1, d2);
            d4 = pow(d1, d2);
            f1 = sinf((float)d3);
            f2 = cosf((float)d4);
            break;
        default:
            i3 = i1 ^ i2;
            i4 = ~i1;
            d3 = fmax(d1, d2);
            d4 = fmin(d1, d2);
            f1 = (float)d3 + 1.0f;
            f2 = (float)d4 - 1.0f;
            break;
    }
    
    /* More operations using all variables */
    i5 = i3 + i4;
    i6 = i3 * i4;
    d5 = d3 + d4;
    d6 = d3 * d4;
    f3 = f1 + f2;
    f4 = f1 * f2;
    
    /* Artificial dependency chain with inline assembly */
    asm volatile("" : "+r"(i5), "+r"(d5), "+r"(f3));
    
    i7 = i5 + i6;
    i8 = i5 - i6;
    d7 = d5 + d6;
    d8 = d5 - d6;
    f5 = f3 + f4;
    
    i9 = i7 * i8;
    i10 = i7 / (i8 + 1);
    d9 = d7 * d8;
    d10 = d7 / (d8 + 0.001);
    
    /* Use all variables in final computation */
    return (double)(i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10) +
           d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           (double)(f1 + f2 + f3 + f4 + f5);
}

/* Function 4: Switch statement with different operation blocks */
long func4_switch_blocks(int selector, long a, long b, long c) {
    long result = 0;
    
    switch (selector % 6) {
        case 0: {
            /* Block with many sequential dependencies */
            long t1 = a + b;
            long t2 = t1 * c;
            long t3 = t2 - a;
            long t4 = t3 / b;
            long t5 = t4 | c;
            long t6 = t5 & a;
            long t7 = t6 ^ b;
            long t8 = t7 << 3;
            long t9 = t8 >> 1;
            long t10 = t9 + t1;
            result = t2 + t4 + t6 + t8 + t10;
            break;
        }
        case 1: {
            /* Different dependency pattern */
            long t1 = a * b;
            long t2 = c - a;
            long t3 = t1 + t2;
            long t4 = t3 * 7;
            long t5 = t4 / 3;
            long t6 = t5 | t1;
            long t7 = t6 & t2;
            long t8 = t7 ^ t3;
            long t9 = t8 << 2;
            long t10 = t9 >> 2;
            result = t1 - t3 + t5 - t7 + t9;
            break;
        }
        case 2: {
            /* Memory access pattern */
            long arr[10];
            for (int i = 0; i < 10; i++) {
                arr[i] = a + i * b;
            }
            for (int i = 1; i < 10; i++) {
                arr[i] = arr[i] + arr[i-1];
            }
            result = arr[5] + arr[9];
            break;
        }
        case 3: {
            /* Mixed operations with anti-dependencies */
            long t1 = a;
            long t2 = b;
            long t3 = c;
            t1 = t1 + t2;  /* WAR on t1 */
            t2 = t2 * t3;  /* WAR on t2 */
            t3 = t3 - t1;  /* WAR on t3 */
            t1 = t1 | t2;  /* WAW on t1 */
            t2 = t2 & t3;  /* WAW on t2 */
            t3 = t3 ^ t1;  /* WAW on t3 */
            result = t1 + t2 + t3;
            break;
        }
        case 4: {
            /* Chain of dependencies with inline assembly */
            long t1 = a + b;
            asm volatile("" : "+r"(t1));
            long t2 = t1 * c;
            asm volatile("" : "+r"(t2));
            long t3 = t2 - a;
            asm volatile("" : "+r"(t3));
            long t4 = t3 / b;
            result = t1 + t2 + t3 + t4;
            break;
        }
        default: {
            /* Complex computation using all inputs */
            long t1 = (a << 2) | (b >> 3);
            long t2 = (b << 3) & (c >> 2);
            long t3 = (c << 4) ^ (a >> 1);
            long t4 = t1 + t2;
            long t5 = t2 + t3;
            long t6 = t3 + t1;
            long t7 = t4 * t5;
            long t8 = t5 * t6;
            long t9 = t6 * t4;
            result = t7 + t8 + t9;
            break;
        }
    }
    
    return result;
}

/* Main function to ensure all code is executed */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int input1, input2, input3;
    volatile float input4;
    volatile double input5, input6;
    volatile long input7, input8, input9;
    
    /* Read from command line or use defaults */
    if (argc > 1) {
        input1 = atoi(argv[1]);
        input2 = atoi(argv[argc > 2 ? 2 : 1]);
        input3 = atoi(argv[argc > 3 ? 3 : 1]);
        input4 = atof(argv[argc > 4 ? 4 : "1.5"]);
        input5 = atof(argv[argc > 5 ? 5 : "2.5"]);
        input6 = atof(argv[argc > 6 ? 6 : "3.5"]);
        input7 = atol(argv[argc > 7 ? 7 : "100"]);
        input8 = atol(argv[argc > 8 ? 8 : "200"]);
        input9 = atol(argv[argc > 9 ? 9 : "300"]);
    } else {
        /* Default values if no arguments provided */
        input1 = 42;
        input2 = 17;
        input3 = 89;
        input4 = 1.5f;
        input5 = 2.5;
        input6 = 3.5;
        input7 = 100;
        input8 = 200;
        input9 = 300;
    }
    
    /* Call all functions with non-constant inputs */
    int result1 = func1_intensive(input1, input2, input3, input1 + input2, input2 + input3);
    float result2 = func2_fp_loop(input4, 50);
    double result3 = func3_mixed_control(input1, input5, input6);
    long result4 = func4_switch_blocks(input2, input7, input8, input9);
    
    /* Aggregate results to prevent dead code elimination */
    volatile double final_result = 0.0;
    final_result += result1;
    final_result += result2;
    final_result += result3;
    final_result += result4;
    
    /* Print checksum to ensure all computations are used */
    printf("Result checksum: %f\n", final_result);
    
    return 0;
}
