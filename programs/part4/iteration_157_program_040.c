/* test_scheduler_context.c
 * Designed to trigger GCC's scheduler context allocation and cleanup
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
int integer_computation(int a, int b, int c) {
    /* Create register pressure with many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - a;
    v4 = v3 / b;
    v5 = v4 << 2;
    v6 = v5 | c;
    v7 = v6 & 0xFF;
    v8 = v7 ^ v1;
    v9 = v8 + v2;
    v10 = v9 - v3;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10 * 2;      /* WAR: v10 read before potential write */
    v10 = v11 + 1;      /* WAW: v10 written again */
    v12 = v11 - v10;
    v13 = v12 << 1;
    v14 = v13 >> 2;
    v15 = v14 | v11;
    v16 = v15 & v12;
    v17 = v16 ^ v13;
    v18 = v17 + v14;
    v19 = v18 - v15;
    v20 = v19 * v16;
    
    /* More operations to increase block size */
    v21 = v20 / (v17 + 1);
    v22 = v21 << (v18 & 3);
    v23 = v22 | v19;
    v24 = v23 & 0xFFFF;
    v25 = v24 ^ v20;
    v26 = v25 + v21;
    v27 = v26 - v22;
    v28 = v27 * v23;
    v29 = v28 / (v24 + 1);
    v30 = v29 | v25;
    
    /* Inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v30) : : "memory");
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* Branch with different dependency chain */
        v30 = v30 * 2;
        v29 = v29 + v28;
        v28 = v28 - v27;
    } else {
        /* Alternative branch */
        v30 = v30 / 2;
        v29 = v29 - v28;
        v28 = v28 + v27;
    }
    
    /* Final computation using many variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float a, float b, int size) {
    float arr[32];
    float sum = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = a;
    arr[1] = b;
    for (int i = 2; i < size && i < 32; i++) {
        /* Data dependencies across loop iterations */
        arr[i] = arr[i-1] * arr[i-2] + (float)i;
    }
    
    /* Process array with mixed operations */
    for (int i = 0; i < size && i < 32; i++) {
        float temp;
        
        /* Complex floating-point operations */
        temp = arr[i] * 2.0f;
        temp = temp / (arr[i] + 1.0f);
        temp = sqrtf(fabsf(temp));
        temp = sinf(temp) + cosf(temp);
        
        /* Memory access with address calculation */
        if (i > 0) {
            temp = temp * arr[i-1];
        }
        
        sum += temp;
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : "+r"(i), "+m"(arr[i]) : : "memory");
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int x, float y, double z) {
    /* Many local variables of different types */
    int i1 = x, i2 = x*2, i3 = x*3, i4 = x*4, i5 = x*5;
    float f1 = y, f2 = y*2.0f, f3 = y*3.0f, f4 = y*4.0f, f5 = y*5.0f;
    double d1 = z, d2 = z*2.0, d3 = z*3.0, d4 = z*4.0, d5 = z*5.0;
    
    /* Complex control flow */
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            /* Block A: Integer operations */
            i1 = i1 + i2;
            i2 = i2 * i3;
            i3 = i3 - i4;
            i4 = i4 ^ i5;
            i5 = i5 | i1;
            
            /* Type conversions create additional dependencies */
            f1 = (float)i1 + f2;
            d1 = (double)f1 * d2;
        } else if (i % 3 == 1) {
            /* Block B: Floating operations */
            f2 = f2 * f3;
            f3 = f3 / f4;
            f4 = f4 - f5;
            f5 = f5 + f1;
            
            d2 = d2 * d3;
            d3 = d3 / d4;
            d4 = d4 - d5;
            d5 = d5 + d1;
        } else {
            /* Block C: Mixed operations */
            i1 = i1 ^ (int)f1;
            f2 = f2 + (float)i2;
            d3 = d3 * (double)i3;
            
            /* Memory operations */
            volatile int mem_var = i4;
            i4 = mem_var + 1;
        }
        
        /* Loop-carried dependencies */
        i1 = i1 + i;
        f2 = f2 + (float)i;
        d3 = d3 + (double)i;
    }
    
    /* Switch statement for additional control flow */
    switch (x % 4) {
        case 0:
            return (double)i1 + d1;
        case 1:
            return (double)f2 + d2;
        case 2:
            return (double)i3 * d3;
        case 3:
            return (double)f4 / d4;
        default:
            return d5;
    }
}

/* Function 4: Switch statement with different operation blocks */
long switch_based_computation(int mode, long base) {
    long result = base;
    
    switch (mode % 5) {
        case 0: {
            /* Integer arithmetic chain */
            long a = result * 3;
            long b = a + result;
            long c = b - a;
            long d = c * b;
            long e = d / (a + 1);
            result = e ^ d;
            break;
        }
        case 1: {
            /* Bit manipulation chain */
            result = result << 3;
            result = result | 0x0F0F0F0F;
            result = result & 0xFFFFFFFF;
            result = result ^ 0x12345678;
            result = result >> 2;
            break;
        }
        case 2: {
            /* Memory-intensive pattern */
            long arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = result + i;
                if (i > 0) {
                    arr[i] = arr[i] * arr[i-1];
                }
            }
            result = arr[7];
            break;
        }
        case 3: {
            /* Complex dependency chain */
            long x = result;
            for (int i = 0; i < 6; i++) {
                x = x * (i + 1);
                x = x + result;
                x = x ^ (x >> 8);
                asm volatile("" : "+r"(x) : : "memory");
            }
            result = x;
            break;
        }
        case 4: {
            /* Mixed operations with many temporaries */
            long t1 = result + 100;
            long t2 = t1 * 2;
            long t3 = t2 - 50;
            long t4 = t3 / 3;
            long t5 = t4 | t1;
            long t6 = t5 & t2;
            long t7 = t6 ^ t3;
            long t8 = t7 + t4;
            long t9 = t8 - t5;
            long t10 = t9 * t6;
            result = t10;
            break;
        }
    }
    
    return result;
}

/* Main function to ensure all code paths are executed */
int main(int argc, char *argv[]) {
    int result1, result2;
    float result3;
    double result4;
    long result5;
    
    /* Use volatile and command line to prevent optimization */
    volatile int input1 = g_input1;
    volatile int input2 = g_input2;
    volatile float input3 = g_input3;
    volatile float input4 = g_input4;
    
    if (argc > 1) {
        input1 = atoi(argv[1]);
    }
    if (argc > 2) {
        input2 = atoi(argv[2]);
    }
    
    /* Call all functions to trigger scheduler in different contexts */
    result1 = integer_computation(input1, input2, input1 + input2);
    result3 = float_array_processing(input3, input4, 20);
    result4 = mixed_operations(input1, input3, (double)input2);
    result5 = switch_based_computation(input1, input2);
    
    /* Second call with different parameters */
    result2 = integer_computation(input2, input1, input1 - input2);
    
    /* Combine results to prevent dead code elimination */
    volatile long final_result = 0;
    final_result += result1;
    final_result += (long)result3;
    final_result += (long)result4;
    final_result += result5;
    final_result += result2;
    
    /* Print to ensure code isn't optimized away */
    printf("Result: %ld\n", final_result);
    
    return 0;
}
