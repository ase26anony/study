/* test_scheduler_context.c
 * Complex program to trigger scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile inputs to prevent constant propagation */
volatile int g_input1 = 42;
volatile int g_input2 = 17;
volatile float g_input3 = 3.14159f;
volatile float g_input4 = 2.71828f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_int_operations(int a, int b, int c) {
    /* Create register pressure with many variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - a;
    v4 = v3 / (b + 1);
    v5 = v4 << 2;
    v6 = v5 ^ v3;
    v7 = v6 | v2;
    v8 = v7 & 0xFF;
    v9 = v8 + v1;
    v10 = v9 - v4;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10;
    v10 = v11 + 1;  /* WAR via v10 */
    v12 = v11 * 2;
    v11 = v12 - 3;  /* WAW on v11 */
    
    /* More complex chains */
    v13 = (v11 << 3) | (v12 >> 1);
    v14 = v13 % (v10 + 1);
    v15 = v14 ^ v13;
    v16 = v15 + v14 - v13;
    v17 = v16 * v15 / (v14 + 1);
    v18 = v17 & v16 | v15;
    v19 = v18 ^ v17;
    v20 = v19 + v18 - v17;
    
    /* Control flow to create basic blocks */
    if (v20 > 1000) {
        v21 = v20 / 2;
        v22 = v21 * 3;
        v23 = v22 + 7;
    } else {
        v21 = v20 * 2;
        v22 = v21 / 3;
        v23 = v22 - 7;
    }
    
    /* Artificial dependency via inline asm */
    asm volatile("" : "+r"(v23));
    
    /* More operations in this basic block */
    v24 = v23 + v22;
    v25 = v24 * v21;
    v26 = v25 - v20;
    v27 = v26 / v19;
    v28 = v27 | v18;
    v29 = v28 ^ v17;
    v30 = v29 & 0xFFFF;
    
    return v30;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float base, int iterations) {
    float arr[32];
    float result = base;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1f;
    for (int i = 2; i < 32; i++) {
        /* True dependencies across loop iterations */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Complex loop with register pressure */
    for (int iter = 0; iter < iterations; iter++) {
        float temp[8];
        
        /* Multiple floating point operations */
        temp[0] = arr[iter % 32] * 1.5f;
        temp[1] = temp[0] + arr[(iter+1) % 32];
        temp[2] = temp[1] - arr[(iter+2) % 32];
        temp[3] = temp[2] * temp[1];
        temp[4] = temp[3] / (temp[0] + 1.0f);
        temp[5] = temp[4] + temp[3] - temp[2];
        temp[6] = temp[5] * 0.9f;
        temp[7] = temp[6] / 1.1f;
        
        /* Conditional inside loop */
        if (temp[7] > 0.0f) {
            result += temp[7] * 0.5f;
            arr[iter % 16] = result;
        } else {
            result -= temp[7] * 0.3f;
            arr[iter % 16] = -result;
        }
        
        /* Inline asm to prevent reordering */
        asm volatile("" : "+r"(iter), "+m"(arr[iter % 16]));
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many locals */
double mixed_operations(int a, float b, double c) {
    /* Many local variables for register pressure */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Initial computations */
    i1 = a * 2;
    f1 = b * 3.14f;
    d1 = c * 2.71828;
    
    /* Cross-type dependencies */
    i2 = i1 + (int)f1;
    f2 = f1 + (float)i2;
    d2 = d1 + (double)f2;
    
    i3 = i2 ^ (int)d2;
    f3 = f2 * (float)i3;
    d3 = d2 / d1;
    
    /* Complex control flow */
    if (i3 > 100) {
        i4 = i3 / 5;
        f4 = f3 * 2.0f;
        d4 = d3 + 1.0;
        
        i5 = i4 << 1;
        f5 = f4 / 1.5f;
        d5 = d4 * d3;
    } else if (i3 < -100) {
        i4 = i3 * 3;
        f4 = f3 / 3.0f;
        d4 = d3 - 1.0;
        
        i5 = i4 >> 1;
        f5 = f5 + 2.0f;  /* WAW on f5 */
        d5 = d4 / d3;
    } else {
        i4 = i3 + 50;
        f4 = f3 - 1.0f;
        d4 = d3 * 2.0;
        
        i5 = i4 | 0xFF;
        f5 = f4 * f3;
        d5 = d4 + d3;
    }
    
    /* More operations in basic block */
    i6 = i5 + i4;
    f6 = f5 + f4;
    d6 = d5 + d4;
    
    i7 = i6 * i5;
    f7 = f6 * f5;
    d7 = d6 * d5;
    
    i8 = i7 - i6;
    f8 = f7 - f6;
    d8 = d7 - d6;
    
    i9 = i8 / (i7 + 1);
    f9 = f8 / (f7 + 1.0f);
    d9 = d8 / (d7 + 1.0);
    
    i10 = i9 ^ i8;
    f10 = f9 * 0.5f;
    d10 = d9 * 0.5;
    
    /* Final result mixing all types */
    return d10 + (double)f10 + (double)i10;
}

/* Function 4: Switch statement with different operation blocks */
long switch_based_computation(int mode, long seed) {
    long result = seed;
    
    switch (mode % 4) {
        case 0: {
            /* Block with integer operations */
            long a = seed * 3;
            long b = a + 17;
            long c = b ^ 0xABCDEF;
            long d = c << 3;
            long e = d >> 1;
            long f = e | 0xFF;
            long g = f & 0xFFFF;
            long h = g % 1001;
            result = h * 2;
            
            /* Inline asm for dependency */
            asm volatile("" : "+r"(result));
            break;
        }
        case 1: {
            /* Block with mixed operations */
            double x = (double)seed * 1.234;
            float y = (float)x * 2.345f;
            int z = (int)y + 567;
            long w = (long)z * 8910;
            result = w ^ (long)x;
            break;
        }
        case 2: {
            /* Block with memory access pattern */
            long arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = seed + i;
            }
            
            /* Chain of dependencies */
            arr[0] = arr[0] * 2;
            arr[1] = arr[0] + arr[1];
            arr[2] = arr[1] - arr[2];
            arr[3] = arr[2] * arr[3];
            arr[4] = arr[3] / (arr[4] + 1);
            arr[5] = arr[4] | arr[5];
            arr[6] = arr[5] ^ arr[6];
            arr[7] = arr[6] & arr[7];
            
            result = arr[7];
            break;
        }
        case 3: {
            /* Complex block with many operations */
            long t1 = seed + 1;
            long t2 = t1 * 3;
            long t3 = t2 - 5;
            long t4 = t3 / 2;
            long t5 = t4 | 0xF0F0;
            long t6 = t5 ^ 0x0F0F;
            long t7 = t6 << 4;
            long t8 = t7 >> 2;
            long t9 = t8 + t7;
            long t10 = t9 - t6;
            long t11 = t10 * t5;
            long t12 = t11 / (t4 + 1);
            result = t12;
            break;
        }
    }
    
    return result;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int result_int = 0;
    volatile float result_float = 0.0f;
    volatile double result_double = 0.0;
    volatile long result_long = 0L;
    
    /* Get dynamic inputs */
    int base1 = g_input1;
    int base2 = g_input2;
    float base3 = g_input3;
    float base4 = g_input4;
    
    /* Add some randomness */
    if (argc > 1) {
        base1 += atoi(argv[1]);
    }
    
    /* Call all complex functions */
    result_int = complex_int_operations(base1, base2, base1 ^ base2);
    result_float = float_array_processing(base3 + base4, 8);
    result_double = mixed_operations(base1, base3, (double)base2);
    result_long = switch_based_computation(base1, base2);
    
    /* Aggregate results to prevent dead code elimination */
    volatile long final_result = 0;
    final_result += (long)result_int;
    final_result += (long)result_float;
    final_result += (long)result_double;
    final_result += result_long;
    
    /* Print to ensure code isn't optimized away */
    printf("Result checksum: %ld\n", final_result);
    
    return 0;
}
