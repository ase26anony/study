/* test_scheduler_context.c
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched test_scheduler_context.c -o test
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
int func1_intensive(int a, int b, int c, int d, int e) {
    /* Create register pressure with many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long dependency chain with RAW dependencies */
    v1 = a + b;
    asm volatile("" : "+r"(v1));  /* Artificial dependency */
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 | v1;
    v6 = v5 ^ v2;
    v7 = v6 & v3;
    v8 = v7 << 2;
    v9 = v8 >> 1;
    v10 = v9 + v4;
    
    /* Anti-dependencies (WAR) */
    v11 = v10 * 2;
    v10 = v11 - 5;  /* Anti-dependency: v10 written after v11 reads it */
    
    /* Output dependencies (WAW) */
    v12 = v11 + 7;
    v12 = v12 * 3;  /* Output dependency: v12 written twice */
    
    /* More operations to create scheduling complexity */
    v13 = v12 % 13;
    v14 = v13 + v5;
    v15 = v14 - v6;
    v16 = v15 * v7;
    v17 = v16 / v8;
    v18 = v17 | v9;
    v19 = v18 ^ v10;
    v20 = v19 & v11;
    
    /* Use remaining variables to prevent dead store elimination */
    v21 = v20 + v12;
    v22 = v21 - v13;
    v23 = v22 * v14;
    v24 = v23 / v15;
    v25 = v24 | v16;
    v26 = v25 ^ v17;
    v27 = v26 & v18;
    v28 = v27 << v19;
    v29 = v28 >> v20;
    v30 = v29 + v21;
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* True path with more dependencies */
        v22 = v30 * 2;
        v23 = v22 - 500;
        v24 = v23 / 3;
        return v24 + v22;
    } else {
        /* False path with different operations */
        v25 = v30 * 3;
        v26 = v25 + 250;
        v27 = v26 / 2;
        return v27 - v25;
    }
}

/* Function 2: Floating-point array processing with loops */
float func2_float_loop(float base, int iterations) {
    float arr[32];
    float sum = 0.0f;
    int i, j;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1f;
    for (i = 2; i < 32; i++) {
        /* RAW dependency through array */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
        asm volatile("" : "+r"(i));  /* Prevent loop unrolling */
    }
    
    /* Nested loop with mixed operations */
    for (i = 0; i < iterations; i++) {
        float temp = 0.0f;
        for (j = 0; j < 16; j++) {
            /* Complex floating-point operations */
            temp += arr[j] * (float)i + arr[31-j] / (float)(j+1);
            
            /* Conditional inside loop */
            if (temp > 100.0f) {
                temp = temp * 0.9f;
                arr[j] = arr[j] - 0.1f;
            } else {
                temp = temp * 1.1f;
                arr[j] = arr[j] + 0.1f;
            }
        }
        sum += temp;
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many locals */
double func3_mixed_ops(int x, float y, double z) {
    /* Many local variables for register pressure */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Initial computations with type mixing */
    i1 = x * 2;
    f1 = y * 3.14f;
    d1 = z * 2.71828;
    
    /* Cross-type dependencies */
    i2 = i1 + (int)f1;
    f2 = f1 + (float)i1;
    d2 = d1 + (double)i1;
    
    /* More operations creating complex dependency graph */
    i3 = i2 * 3;
    f3 = f2 / 2.0f;
    d3 = d2 * 1.5;
    
    i4 = i3 - (int)d3;
    f4 = f3 * (float)i3;
    d4 = d3 / (double)f3;
    
    i5 = i4 ^ 0xFF;
    f5 = f4 + 10.0f;
    d5 = d4 - 5.0;
    
    /* Control flow with switch statement */
    switch (i5 % 4) {
        case 0:
            i6 = i5 * 2;
            f6 = f5 * 2.0f;
            d6 = d5 * 2.0;
            break;
        case 1:
            i6 = i5 / 2;
            f6 = f5 / 2.0f;
            d6 = d5 / 2.0;
            break;
        case 2:
            i6 = i5 + 100;
            f6 = f5 + 100.0f;
            d6 = d5 + 100.0;
            break;
        default:
            i6 = i5 - 100;
            f6 = f5 - 100.0f;
            d6 = d5 - 100.0;
            break;
    }
    
    /* Use remaining variables */
    i7 = i6 | 0x0F;
    f7 = f6 * 1.1f;
    d7 = d6 * 1.1;
    
    i8 = i7 << 2;
    f8 = f7 / 1.1f;
    d8 = d7 / 1.1;
    
    i9 = i8 >> 1;
    f9 = f8 + f7;
    d9 = d8 + d7;
    
    i10 = i9 % 17;
    f10 = f9 - f8;
    d10 = d9 - d8;
    
    /* Final computation using all types */
    return (double)i10 + (double)f10 + d10;
}

/* Function 4: Complex switch with different operation blocks */
long func4_switch_complex(int mode, long val) {
    long result = val;
    
    switch (mode % 5) {
        case 0: {
            /* Block with many integer operations */
            long a = result * 2;
            long b = a + 100;
            long c = b - 50;
            long d = c * 3;
            long e = d / 2;
            long f = e | 0xFF;
            long g = f ^ 0xAA;
            long h = g & 0x55;
            result = h << 2;
            break;
        }
        case 1: {
            /* Block with bit manipulation */
            result = ((result << 5) | (result >> 59)) ^ 0xDEADBEEF;
            result = (result & 0xAAAAAAAA) | ((result & 0x55555555) << 1);
            result = ~result + 1;
            break;
        }
        case 2: {
            /* Block with conditional operations */
            for (int i = 0; i < 8; i++) {
                if (result & (1L << i)) {
                    result = result * 3 + i;
                } else {
                    result = result / 2 - i;
                }
            }
            break;
        }
        case 3: {
            /* Block with memory-like access pattern */
            long temp[8];
            for (int i = 0; i < 8; i++) {
                temp[i] = result + i * 7;
            }
            for (int i = 7; i >= 0; i--) {
                result = result ^ temp[i];
                if (i > 0) {
                    result = result + temp[i-1];
                }
            }
            break;
        }
        default: {
            /* Default block with mixed operations */
            result = (result * 7) % 131071;
            result = result * result;
            result = (result >> 16) | (result << 48);
            break;
        }
    }
    
    return result;
}

/* Main function that calls all test functions */
int main(int argc, char *argv[]) {
    int result_int = 0;
    float result_float = 0.0f;
    double result_double = 0.0;
    long result_long = 0L;
    
    /* Use command line arguments or defaults to create dynamic inputs */
    int base_int = (argc > 1) ? atoi(argv[1]) : g_input1;
    int iter_count = (argc > 2) ? atoi(argv[2]) : 50;
    float base_float = (argc > 3) ? atof(argv[3]) : g_input3;
    
    /* Call all functions to trigger scheduler in different contexts */
    result_int = func1_intensive(base_int, g_input2, base_int+1, g_input2*2, iter_count);
    
    result_float = func2_float_loop(base_float, iter_count % 20 + 5);
    
    result_double = func3_mixed_ops(base_int, base_float, (double)base_int / (base_float + 1.0f));
    
    result_long = func4_switch_complex(base_int, (long)result_int * 1000L);
    
    /* Aggregate results into a volatile sink to prevent optimization */
    volatile int final_check = 0;
    final_check = (int)result_int + (int)result_float + (int)result_double + (int)result_long;
    
    /* Print minimal output (can be redirected to /dev/null) */
    if (argc > 4) {
        printf("Results: %d %.2f %.2f %ld\n", 
               result_int, result_float, result_double, result_long);
    }
    
    return final_check % 256;
}
