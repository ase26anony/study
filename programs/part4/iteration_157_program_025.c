/* test_scheduler_context.c
 * Complex program to trigger scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile inputs to prevent constant propagation */
volatile int g_input1 = 42;
volatile float g_input2 = 3.14159f;
volatile int g_input3 = 100;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_int_chain(int a, int b, int c, int d, int e) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long dependency chain with RAW dependencies */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 << 2;
    v6 = v5 ^ v1;
    v7 = v6 | v2;
    v8 = v7 & v3;
    v9 = v8 + v4;
    v10 = v9 * v5;
    
    /* Anti-dependencies (WAR) */
    v11 = v10 + 1;
    v10 = v11 - 2;  /* WAR: v10 written after v11 read */
    v12 = v10 * 3;
    v10 = v12 / 4;  /* Another WAR */
    
    /* Output dependencies (WAW) */
    v13 = v12 + v11;
    v13 = v13 * 2;  /* WAW: v13 written twice */
    
    /* More complex operations */
    v14 = (v13 << 3) | (v12 >> 2);
    v15 = v14 ^ v11;
    v16 = v15 + v10;
    v17 = v16 - v9;
    v18 = v17 * v8;
    v19 = v18 / v7;
    v20 = v19 | v6;
    
    /* Use inline assembly to create opaque dependencies */
    asm volatile("" : "+r"(v20));
    
    /* Second dependency chain */
    v21 = b + c;
    v22 = v21 * d;
    v23 = v22 - e;
    v24 = v23 / a;
    v25 = v24 << 1;
    
    /* Merge both chains */
    v26 = v20 + v25;
    v27 = v26 * v19;
    v28 = v27 - v18;
    v29 = v28 / v17;
    v30 = v29 | v16;
    
    /* Control flow to create basic block boundaries */
    if (v30 > 1000) {
        /* True path with more dependencies */
        v30 = v30 * 2;
        v29 = v29 + v28;
        v28 = v27 - v26;
    } else {
        /* False path with different operations */
        v30 = v30 / 2;
        v29 = v29 - v28;
        v28 = v27 + v26;
    }
    
    /* Final computation using most variables */
    return v30 + v29 + v28 + v27 + v26 + v25 + v24 + v23 + v22 + v21 +
           v20 + v19 + v18 + v17 + v16 + v15 + v14 + v13 + v12 + v11 +
           v10 + v9 + v8 + v7 + v6 + v5 + v4 + v3 + v2 + v1;
}

/* Function 2: Floating-point array processing with loops */
float floating_point_processing(float base, int iterations) {
    float arr[32];
    float result = 0.0f;
    int i, j;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1f;
    for (i = 2; i < 32; i++) {
        /* RAW dependency through array */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Complex loop with multiple dependencies */
    for (i = 0; i < iterations; i++) {
        float temp = 0.0f;
        
        /* Inner loop with floating-point operations */
        for (j = 0; j < 16; j++) {
            temp += arr[j] * sinf(arr[31-j] + (float)i);
        }
        
        /* Update array elements creating WAR dependencies */
        for (j = 1; j < 31; j++) {
            float old = arr[j];  /* Read */
            arr[j] = (arr[j-1] + arr[j+1]) * 0.5f;  /* Write - WAR */
            result += old;  /* Use old value */
        }
        
        result += temp;
        
        /* Inline assembly to prevent optimization */
        asm volatile("" : "+r"(i), "+r"(result));
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int mode, double x, double y) {
    /* Declare many variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Initialize with dependencies */
    i1 = (int)x;
    f1 = (float)y;
    d1 = x + y;
    
    i2 = i1 * 2;
    f2 = f1 * 1.5f;
    d2 = d1 * 2.0;
    
    i3 = i2 + i1;
    f3 = f2 + f1;
    d3 = d2 + d1;
    
    i4 = i3 - i2;
    f4 = f3 - f2;
    d4 = d3 - d2;
    
    i5 = i4 / 3;
    f5 = f4 / 2.0f;
    d5 = d4 / 3.0;
    
    /* Complex control flow */
    switch (mode % 4) {
        case 0:
            i6 = i5 * i4;
            f6 = f5 * f4;
            d6 = d5 * d4;
            break;
        case 1:
            i6 = i5 + i4;
            f6 = f5 + f4;
            d6 = d5 + d4;
            break;
        case 2:
            i6 = i5 - i4;
            f6 = f5 - f4;
            d6 = d5 - d4;
            break;
        case 3:
            i6 = i5 ^ i4;
            f6 = f5 / f4;
            d6 = d5 / d4;
            break;
    }
    
    /* More operations with type mixing */
    i7 = i6 + (int)f6;
    f7 = f6 + (float)i6;
    d7 = d6 + (double)i6;
    
    i8 = i7 << 2;
    f8 = f7 * 3.14159f;
    d8 = d7 * 3.14159265358979;
    
    /* Memory operations with address calculations */
    int arr[20];
    for (int k = 0; k < 20; k++) {
        arr[k] = i8 + k;
    }
    
    /* Process array with dependencies */
    for (int k = 1; k < 20; k++) {
        arr[k] = arr[k] + arr[k-1];
    }
    
    i9 = arr[19];
    f9 = f8 + (float)i9;
    d9 = d8 + (double)i9;
    
    /* Final computations using all variables */
    i10 = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9;
    f10 = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9;
    d10 = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9;
    
    return (double)i10 + (double)f10 + d10;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(int selector, long a, long b, long c) {
    long result = 0;
    
    switch (selector & 7) {
        case 0: {
            /* Block with arithmetic dependencies */
            long t1 = a + b;
            long t2 = t1 * c;
            long t3 = t2 - a;
            long t4 = t3 / b;
            long t5 = t4 << 3;
            long t6 = t5 ^ t1;
            result = t6 + t2 + t3 + t4 + t5;
            break;
        }
        case 1: {
            /* Block with bit operations */
            long t1 = a | b;
            long t2 = t1 & c;
            long t3 = t2 ^ a;
            long t4 = ~t3;
            long t5 = t4 << 2;
            long t6 = t5 >> 1;
            result = t1 + t2 + t3 + t4 + t5 + t6;
            break;
        }
        case 2: {
            /* Block with conditional operations */
            long t1 = (a > b) ? a : b;
            long t2 = (b > c) ? b : c;
            long t3 = (c > a) ? c : a;
            long t4 = t1 + t2;
            long t5 = t2 + t3;
            long t6 = t3 + t1;
            result = t4 * t5 / (t6 ? t6 : 1);
            break;
        }
        case 3: {
            /* Block with loop */
            long t1 = a;
            for (int i = 0; i < 10; i++) {
                t1 = t1 * b + c;
                asm volatile("" : "+r"(t1));
            }
            result = t1;
            break;
        }
        case 4: {
            /* Block with memory operations */
            long arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = a + i * b;
            }
            for (int i = 1; i < 8; i++) {
                arr[i] += arr[i-1];
            }
            result = arr[7];
            break;
        }
        case 5: {
            /* Block with mixed operations */
            long t1 = a * b;
            long t2 = b * c;
            long t3 = c * a;
            long t4 = (t1 << 1) | (t2 >> 1);
            long t5 = (t2 << 2) & (t3 >> 2);
            long t6 = (t3 << 3) ^ (t1 >> 3);
            result = t4 + t5 + t6;
            break;
        }
        case 6: {
            /* Block with nested control flow */
            long t1 = a;
            if (b > 0) {
                t1 = t1 * 2;
                if (c > 0) {
                    t1 = t1 + b;
                } else {
                    t1 = t1 - c;
                }
            } else {
                t1 = t1 / 2;
                if (c > 0) {
                    t1 = t1 * c;
                } else {
                    t1 = t1 / -c;
                }
            }
            result = t1;
            break;
        }
        case 7: {
            /* Block with many sequential dependencies */
            long t1 = a + 1;
            long t2 = t1 * b;
            long t3 = t2 - c;
            long t4 = t3 + a;
            long t5 = t4 * 2;
            long t6 = t5 / b;
            long t7 = t6 ^ c;
            long t8 = t7 | a;
            long t9 = t8 & b;
            long t10 = t9 << 1;
            result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
            break;
        }
    }
    
    return result;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent compile-time computation */
    volatile int input1 = g_input1;
    volatile float input2 = g_input2;
    volatile int input3 = g_input3;
    
    /* Read from command line if available to add variability */
    if (argc > 1) {
        input1 = atoi(argv[1]);
    }
    if (argc > 2) {
        input2 = atof(argv[2]);
    }
    if (argc > 3) {
        input3 = atoi(argv[3]);
    }
    
    long total_result = 0;
    
    /* Call all complex functions to trigger scheduler in different contexts */
    total_result += complex_int_chain(input1, input1+1, input1+2, input1+3, input1+4);
    
    float fp_result = floating_point_processing(input2, input3 % 10 + 5);
    total_result += (long)fp_result;
    
    double mixed_result = mixed_operations(input1, (double)input2, (double)input3);
    total_result += (long)mixed_result;
    
    long switch_result = switch_based_computation(input1, input3, input1, input3+input1);
    total_result += switch_result;
    
    /* Call functions multiple times with different parameters */
    for (int i = 0; i < 3; i++) {
        total_result += complex_int_chain(input1+i, input3-i, i, input1, input3);
        total_result += (long)floating_point_processing(input2+i*0.1f, 3);
        total_result += (long)mixed_operations((input1+i) % 8, (double)i, (double)(input3+i));
    }
    
    /* Use result to prevent dead code elimination */
    volatile long sink = total_result;
    
    /* Print minimal output */
    printf("Result: %ld\n", sink);
    
    return 0;
}
