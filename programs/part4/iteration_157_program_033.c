/* test_scheduler_context.c
 * 
 * This program creates complex basic blocks with various dependencies,
 * control flow, and register pressure to force GCC's instruction scheduler
 * to allocate and later free scheduling context during compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile variables to prevent constant propagation */
volatile int g_input1 = 42;
volatile int g_input2 = 17;
volatile float g_input3 = 3.14159f;
volatile float g_input4 = 2.71828f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int integer_computation(int a, int b, int c, int d, int e, int f) {
    /* Create many local variables for register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 | f;
    
    /* Anti-dependencies (WAR) */
    v6 = v5 + 1;
    v5 = v6 * 2;  /* v5 reused after v6 calculation */
    
    /* Output dependencies (WAW) */
    v7 = v6 << 2;
    v7 = v7 >> 1;  /* v7 written twice */
    
    /* More complex dependency chain */
    v8 = v7 ^ v5;
    v9 = v8 & v4;
    v10 = v9 | v3;
    v11 = v10 ^ v2;
    v12 = v11 + v1;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v12));
    
    v13 = v12 * 3;
    v14 = v13 / 5;
    v15 = v14 % 7;
    v16 = v15 << 3;
    v17 = v16 >> 2;
    v18 = v17 & 0xFF;
    v19 = v18 | 0x55;
    v20 = v19 ^ 0xAA;
    
    /* Another inline assembly barrier */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 + v19;
    v22 = v21 - v18;
    v23 = v22 * v17;
    v24 = v23 / v16;
    v25 = v24 % v15;
    v26 = v25 ^ v14;
    v27 = v26 & v13;
    v28 = v27 | v12;
    v29 = v28 + v11;
    v30 = v29 - v10;
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* Branch with more operations */
        v30 = v30 * 2;
        v29 = v29 / 2;
    } else {
        /* Alternative branch */
        v30 = v30 + 1000;
        v29 = v29 - 500;
    }
    
    /* Final computation using all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float a, float b, float c, float d) {
    /* Local array with many elements */
    float arr[32];
    float result = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = a;
    arr[1] = b;
    
    /* Loop with data dependencies across iterations */
    for (int i = 2; i < 32; i++) {
        /* True dependencies: arr[i] depends on arr[i-1] and arr[i-2] */
        arr[i] = arr[i-1] * c + arr[i-2] * d;
        
        /* Anti-dependency: reuse arr[i-1] */
        float temp = arr[i-1] * 0.5f;
        arr[i-1] = temp + 1.0f;
        
        /* Mix with integer operations */
        int idx = i * 2;
        arr[i] += (float)(idx % 5);
    }
    
    /* Process array with conditional inside loop */
    for (int i = 0; i < 32; i++) {
        if (i % 3 == 0) {
            arr[i] = arr[i] * arr[i];
        } else if (i % 3 == 1) {
            arr[i] = sqrtf(fabsf(arr[i]));
        } else {
            arr[i] = 1.0f / (arr[i] + 0.001f);
        }
        
        /* Accumulate with dependency chain */
        result += arr[i];
        
        /* Inline assembly to prevent reordering */
        if (i % 8 == 0) {
            asm volatile("" : "+r"(result));
        }
    }
    
    /* Final transformations */
    result = result * result;
    result = result - (a + b + c + d);
    result = fabsf(result);
    
    return result;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int x, float y, double z) {
    /* Declare many variables of different types */
    int i1 = x, i2 = x*2, i3 = x*3, i4 = x*4, i5 = x*5;
    int i6, i7, i8, i9, i10, i11, i12, i13, i14, i15;
    float f1 = y, f2 = y*2.0f, f3 = y*3.0f, f4 = y*4.0f, f5 = y*5.0f;
    double d1 = z, d2 = z*2.0, d3 = z*3.0, d4 = z*4.0, d5 = z*5.0;
    
    /* Complex control flow with switch */
    switch (x % 5) {
        case 0:
            i6 = i1 + i2;
            f1 = f2 * f3;
            d1 = d2 / d3;
            break;
        case 1:
            i6 = i2 - i3;
            f1 = f3 / f4;
            d1 = d3 * d4;
            break;
        case 2:
            i6 = i3 * i4;
            f1 = f4 + f5;
            d1 = d4 - d5;
            break;
        case 3:
            i6 = i4 / (i5 + 1);
            f1 = f5 - f2;
            d1 = d5 + d2;
            break;
        case 4:
            i6 = i5 % (i1 + 1);
            f1 = f2 * f4;
            d1 = d3 / d5;
            break;
    }
    
    /* More operations in each basic block */
    i7 = i6 << 2;
    i8 = i7 >> 1;
    i9 = i8 & 0xFF;
    i10 = i9 | 0x55;
    
    f2 = f1 * 1.5f;
    f3 = f2 / 2.0f;
    f4 = f3 + 3.14f;
    f5 = f4 - 2.71f;
    
    d2 = d1 * 1.618;
    d3 = d2 / 3.14159;
    d4 = d3 + 2.71828;
    d5 = d4 - 1.41421;
    
    /* Nested if-else with operations */
    if (i10 > 100) {
        if (f5 > 0.0f) {
            i11 = i10 * 2;
            f5 = f5 * 2.0f;
        } else {
            i11 = i10 / 2;
            f5 = f5 / 2.0f;
        }
        d5 = d5 * 2.0;
    } else {
        i11 = i10 + 100;
        f5 = f5 + 10.0f;
        d5 = d5 + 10.0;
    }
    
    /* More variables and operations */
    i12 = i11 ^ i9;
    i13 = i12 & i8;
    i14 = i13 | i7;
    i15 = i14 + i6;
    
    /* Final combination */
    double result = (double)i15 + (double)f5 + d5;
    
    /* Use inline assembly to ensure operations aren't eliminated */
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(long a, long b, long c) {
    long result = 0;
    
    /* Large switch to create many basic blocks */
    switch (a % 8) {
        case 0:
            result = b + c;
            result = result * 2;
            result = result - b;
            result = result / (c + 1);
            break;
        case 1:
            result = b - c;
            result = result << 3;
            result = result | 0xF0F0;
            result = result & 0xFFFF;
            break;
        case 2:
            result = b * c;
            result = result % 10007;
            result = result ^ 0xAAAA;
            result = result + 12345;
            break;
        case 3:
            result = b / (c + 1);
            result = result << 1;
            result = result >> 2;
            result = result | (b & c);
            break;
        case 4:
            result = (b << 4) | (c & 0xFF);
            result = result * result;
            result = result % 65537;
            result = result - 32768;
            break;
        case 5:
            result = b ^ c;
            result = ~result;
            result = result + a;
            result = result * 3;
            break;
        case 6:
            result = b & c;
            result = result | (a << 8);
            result = result / 256;
            result = result + 1024;
            break;
        case 7:
            result = b | c;
            result = result & 0xFFFFFF;
            result = result << 12;
            result = result >> 4;
            break;
    }
    
    /* Additional operations after switch */
    for (int i = 0; i < 4; i++) {
        if (result % 2 == 0) {
            result = result / 2;
        } else {
            result = result * 3 + 1;
        }
        
        /* Memory access pattern */
        long temp[4];
        temp[i % 4] = result;
        result = result + temp[(i + 1) % 4];
    }
    
    return result;
}

/* Main function to ensure all code is executed */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int input1 = g_input1;
    volatile int input2 = g_input2;
    volatile float input3 = g_input3;
    volatile float input4 = g_input4;
    
    /* Read from command line if available for more variability */
    if (argc > 1) {
        input1 = atoi(argv[1]);
    }
    if (argc > 2) {
        input2 = atoi(argv[2]);
    }
    if (argc > 3) {
        input3 = atof(argv[3]);
    }
    if (argc > 4) {
        input4 = atof(argv[4]);
    }
    
    /* Call all complex functions */
    int res1 = integer_computation(input1, input2, input1+1, input2-1, 
                                   input1*2, input2/2);
    
    float res2 = float_array_processing(input3, input4, 
                                        input3*0.5f, input4*1.5f);
    
    double res3 = mixed_operations(input1, input3, (double)input2);
    
    long res4 = switch_based_computation((long)input1, (long)input2, 
                                         (long)(input1 + input2));
    
    /* Aggregate results to prevent dead code elimination */
    volatile double final_result = 0.0;
    final_result += (double)res1;
    final_result += (double)res2;
    final_result += res3;
    final_result += (double)res4;
    
    /* Print checksum to ensure all computations are performed */
    printf("Result checksum: %f\n", final_result);
    
    return 0;
}
