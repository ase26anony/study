/* test_scheduler_context.c
 * Program designed to trigger GCC's instruction scheduler context allocation
 * and cleanup, specifically covering free_sched_context in haifa-sched.cc
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
int complex_int_computation(int a, int b, int c) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - a;
    v4 = v3 / b;
    v5 = v4 << 2;
    v6 = v5 | 0xFF;
    v7 = v6 & 0x0F;
    v8 = v7 ^ v1;
    v9 = v8 % 13;
    v10 = v9 + v2;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10 * 2;      /* WAW if we reuse v11 later */
    v12 = v11 + v3;
    v11 = v12 - v4;     /* WAR on v11, WAW on v11 */
    v13 = v11 * v5;
    v14 = v13 / v6;
    v15 = v14 | v7;
    v16 = v15 ^ v8;
    v17 = v16 & v9;
    v18 = v17 + v10;
    v19 = v18 - v11;
    v20 = v19 * v12;
    
    /* More complex dependency chain */
    v21 = (v13 + v14) * (v15 - v16);
    v22 = v21 / (v17 + 1);
    v23 = v22 | v18;
    v24 = v23 ^ v19;
    v25 = v24 & v20;
    v26 = v25 << 3;
    v27 = v26 >> 1;
    v28 = v27 + v21;
    v29 = v28 - v22;
    v30 = v29 * v23;
    
    /* Inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v24), "+r"(v25));
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* Branch with its own dependencies */
        v24 = v25 + v26;
        v25 = v24 * v27;
        v26 = v25 / v28;
    } else {
        /* Alternative branch with different dependencies */
        v24 = v26 - v27;
        v25 = v28 * v29;
        v26 = v30 / v24;
    }
    
    /* Final computation using many variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float floating_point_processing(float a, float b, int n) {
    float arr[32];
    float sum = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = a;
    arr[1] = b;
    for (int i = 2; i < 32; i++) {
        /* True data dependencies in loop */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Process array with mixed operations */
    for (int i = 0; i < n && i < 32; i++) {
        float temp1 = arr[i] * 2.0f;
        float temp2 = sinf(temp1);
        float temp3 = cosf(arr[i]);
        float temp4 = temp2 + temp3;
        float temp5 = sqrtf(fabsf(temp4));
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : "+f"(temp5));
        
        arr[i] = temp5;
        sum += arr[i];
    }
    
    /* Nested loop with dependencies */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i] += arr[i + j] * 0.1f;
        }
        sum += arr[i];
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many locals */
double mixed_operations(int x, float y, double z) {
    /* Many local variables of different types */
    int i1 = x, i2 = x*2, i3 = x*3, i4 = x*4, i5 = x*5;
    float f1 = y, f2 = y*2.0f, f3 = y*3.0f, f4 = y*4.0f, f5 = y*5.0f;
    double d1 = z, d2 = z*2.0, d3 = z*3.0, d4 = z*4.0, d5 = z*5.0;
    
    /* Complex control flow */
    if (x > 0) {
        /* Block A with dependencies */
        i1 = i2 + i3;
        f1 = f2 * f3;
        d1 = d2 - d3;
        
        i4 = i1 * i5;
        f4 = f1 / f5;
        d4 = d1 + d5;
        
        /* Inline assembly barrier */
        asm volatile("" : "+r"(i4), "+f"(f4), "+f"(d4));
    } else {
        /* Block B with different dependencies */
        i1 = i3 - i2;
        f1 = f3 / f2;
        d1 = d3 * d2;
        
        i4 = i5 % i1;
        f4 = f5 + f1;
        d4 = d5 - d1;
    }
    
    /* Switch statement creating multiple basic blocks */
    switch (x % 4) {
        case 0:
            i1 = i2 * i3 + i4;
            f1 = f2 - f3 * f4;
            d1 = d2 / d3 + d4;
            break;
        case 1:
            i1 = i3 / i2 | i4;
            f1 = f3 + f2 / f4;
            d1 = d3 * d2 - d4;
            break;
        case 2:
            i1 = i4 ^ i2 & i3;
            f1 = f4 * f2 + f3;
            d1 = d4 - d2 * d3;
            break;
        case 3:
            i1 = i2 << i3 >> i4;
            f1 = f2 / f3 - f4;
            d1 = d2 + d3 / d4;
            break;
    }
    
    /* Final mixed-type computation */
    return (double)i1 + (double)f1 + d1 + (double)i2 + (double)f2 + d2;
}

/* Function 4: Switch statement with different operation blocks */
long switch_based_computation(int mode, long base) {
    long result = base;
    
    switch (mode % 5) {
        case 0: {
            /* Integer arithmetic chain */
            long a = base * 2;
            long b = a + 0xABCD;
            long c = b ^ 0x1234;
            long d = c << 3;
            long e = d >> 1;
            long f = e | 0xFF;
            long g = f & 0x0F0F;
            long h = g % 17;
            result = a + b + c + d + e + f + g + h;
            break;
        }
        case 1: {
            /* Memory access pattern */
            long temp[8];
            for (int i = 0; i < 8; i++) {
                temp[i] = base * i;
            }
            for (int i = 1; i < 8; i++) {
                temp[i] += temp[i-1];
            }
            result = temp[0];
            for (int i = 1; i < 8; i++) {
                result ^= temp[i];
            }
            break;
        }
        case 2: {
            /* Floating point operations */
            double d1 = (double)base;
            double d2 = d1 * 1.234;
            double d3 = sin(d2);
            double d4 = cos(d1);
            double d5 = d3 * d4;
            double d6 = sqrt(fabs(d5));
            result = (long)(d6 * 1000.0);
            break;
        }
        case 3: {
            /* Bit manipulation chain */
            result = base;
            for (int i = 0; i < 16; i++) {
                result = ((result << 1) | (result >> 63)) ^ 0xDEADBEEF;
                asm volatile("" : "+r"(result));
            }
            break;
        }
        case 4: {
            /* Mixed operations */
            result = base;
            float f = (float)result;
            for (int i = 0; i < 10; i++) {
                result = result * 1103515245 + 12345;
                f = f * 1.1f + 0.5f;
                result ^= (long)f;
            }
            break;
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to create dynamic inputs */
    int dynamic1 = g_input1;
    int dynamic2 = g_input2;
    float dynamic3 = g_input3;
    float dynamic4 = g_input4;
    
    if (argc > 1) {
        dynamic1 += atoi(argv[1]);
    }
    if (argc > 2) {
        dynamic2 += atoi(argv[2]);
    }
    
    /* Call all complex functions to trigger scheduler */
    int result1 = complex_int_computation(dynamic1, dynamic2, dynamic1 ^ dynamic2);
    float result2 = floating_point_processing(dynamic3, dynamic4, 20);
    double result3 = mixed_operations(dynamic1, dynamic3, (double)dynamic2);
    long result4 = switch_based_computation(dynamic1, dynamic2);
    
    /* Aggregate results into volatile sink to prevent optimization */
    volatile long final_result = 0;
    final_result += result1;
    final_result += (long)result2;
    final_result += (long)result3;
    final_result += result4;
    
    /* Print to ensure code isn't dead */
    printf("Result: %ld\n", final_result);
    
    return 0;
}
