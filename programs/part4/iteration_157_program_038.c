/* test_scheduler_context.c
 * A program designed to trigger GCC's scheduler context allocation and cleanup
 * to cover the free_sched_context block in haifa-sched.cc
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
int integer_heavy_computation(int a, int b, int c, int d) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Start with input values */
    v1 = a + b;
    v2 = b * c;
    
    /* Create true data dependencies (RAW) */
    v3 = v1 + v2;
    v4 = v3 - d;
    v5 = v4 * v1;
    v6 = v5 / (v2 + 1);
    v7 = v6 ^ v3;
    v8 = v7 | v4;
    v9 = v8 & v5;
    v10 = v9 << 2;
    
    /* Anti-dependencies (WAR) */
    v1 = v10 + v6;  /* v1 reused after v10 calculation */
    v2 = v1 - v7;   /* v2 reused after v1 calculation */
    
    /* Output dependencies (WAW) */
    v11 = v2 * 3;
    v11 = v11 + v8;  /* v11 written twice */
    
    /* More complex dependency chain */
    v12 = v11 >> 1;
    v13 = v12 + v9;
    v14 = v13 * v10;
    v15 = v14 - v11;
    v16 = v15 / (v12 + 1);
    v17 = v16 ^ v13;
    v18 = v17 | v14;
    v19 = v18 & v15;
    v20 = v19 << 3;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    /* Second chain with mixed operations */
    v21 = v20 + v16;
    v22 = v21 * v17;
    v23 = v22 - v18;
    v24 = v23 / (v19 + 1);
    v25 = v24 ^ v20;
    v26 = v25 | v21;
    v27 = v26 & v22;
    v28 = v27 << 1;
    v29 = v28 + v23;
    v30 = v29 * v24;
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* Block A with more dependencies */
        v1 = v30 - 500;
        v2 = v1 * 2;
        v3 = v2 + v29;
        v4 = v3 / 3;
        v5 = v4 ^ v28;
    } else {
        /* Block B with different dependencies */
        v1 = v30 + 500;
        v2 = v1 / 2;
        v3 = v2 - v29;
        v4 = v3 * 3;
        v5 = v4 | v28;
    }
    
    /* Another inline assembly barrier */
    asm volatile("" : "+r"(v5));
    
    /* Final computation using many variables */
    return v5 + v6 + v7 + v8 + v9 + v10 + v20 + v25 + v30;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float a, float b, int n) {
    /* Many local float variables for register pressure */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    
    /* Initialize with inputs */
    f1 = a;
    f2 = b;
    
    /* Create dependency chains */
    f3 = f1 + f2;
    f4 = f1 * f2;
    f5 = f3 - f4;
    f6 = f5 / f3;
    f7 = f6 * f4;
    f8 = f7 + f5;
    f9 = f8 - f6;
    f10 = f9 * f7;
    
    /* Loop with conditional inside */
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            /* Even iteration operations */
            f11 = f10 + i;
            f12 = f11 * f9;
            f13 = sinf(f12);
            f14 = cosf(f11);
            f15 = f13 + f14;
            sum += f15;
            
            /* More operations to create larger basic block */
            f16 = f15 * f8;
            f17 = f16 / f7;
            f18 = f17 - f6;
            f19 = f18 * f5;
            f20 = f19 + f4;
            f10 = f20;  /* Update for next iteration */
        } else {
            /* Odd iteration operations */
            f11 = f10 - i;
            f12 = f11 / f9;
            f13 = expf(f12);
            f14 = logf(fabsf(f11) + 1.0f);
            f15 = f13 * f14;
            sum += f15;
            
            /* Different operations for odd iterations */
            f16 = f15 + f8;
            f17 = f16 * f7;
            f18 = f17 - f6;
            f19 = f18 / f5;
            f20 = f19 * f4;
            f10 = f20;  /* Update for next iteration */
        }
        
        /* Inline assembly to prevent loop unrolling */
        if (i % 4 == 0) {
            asm volatile("" : "+r"(sum));
        }
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int a, float b, double c) {
    /* Declare many variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Initialize from inputs */
    i1 = a;
    f1 = b;
    d1 = c;
    
    /* Integer operations */
    i2 = i1 * 2;
    i3 = i2 + 100;
    i4 = i3 - i1;
    i5 = i4 / 3;
    i6 = i5 << 2;
    i7 = i6 >> 1;
    i8 = i7 ^ i5;
    i9 = i8 | i4;
    i10 = i9 & i3;
    
    /* Float operations with dependencies on integers */
    f2 = (float)i10 + f1;
    f3 = f2 * 1.5f;
    f4 = f3 - f1;
    f5 = f4 / 2.0f;
    f6 = f5 * f2;
    f7 = f6 + f3;
    f8 = f7 - f4;
    f9 = f8 * f5;
    f10 = f9 / f6;
    
    /* Double operations with mixed dependencies */
    d2 = (double)i9 + d1;
    d3 = d2 * 2.5;
    d4 = d3 + (double)f10;
    d5 = d4 - d1;
    d6 = d5 / 1.7;
    d7 = d6 * d2;
    d8 = d7 + d3;
    d9 = d8 - d4;
    d10 = d9 * d5;
    
    /* Complex control flow */
    if (i10 > 50) {
        if (f10 > 10.0f) {
            d1 = d10 * 2.0;
            f1 = f10 * 3.0f;
            i1 = i10 << 1;
        } else {
            d1 = d10 / 2.0;
            f1 = f10 / 3.0f;
            i1 = i10 >> 1;
        }
    } else {
        if (d10 > 100.0) {
            d1 = d10 + 50.0;
            f1 = f10 + 5.0f;
            i1 = i10 + 25;
        } else {
            d1 = d10 - 50.0;
            f1 = f10 - 5.0f;
            i1 = i10 - 25;
        }
    }
    
    /* More operations after control flow */
    i2 = i1 * 3;
    f2 = f1 * 1.1f;
    d2 = d1 * 1.2;
    
    /* Final computation using all variable types */
    return (double)i2 + (double)f2 + d2;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(int mode, long base) {
    long result = base;
    
    switch (mode % 5) {
        case 0: {
            /* Case 0: Arithmetic operations */
            long a = result + 100;
            long b = a * 2;
            long c = b - 50;
            long d = c / 3;
            long e = d << 2;
            long f = e >> 1;
            long g = f ^ 0xFF;
            long h = g | 0xAA;
            long i = h & 0x55;
            long j = i + a;
            result = j * 2;
            break;
        }
        case 1: {
            /* Case 1: Different arithmetic pattern */
            long a = result - 100;
            long b = a / 2;
            long c = b + 200;
            long d = c * 3;
            long e = d << 1;
            long f = e >> 2;
            long g = f | 0xCC;
            long h = g ^ 0x33;
            long i = h & 0xF0;
            long j = i - b;
            result = j / 2;
            break;
        }
        case 2: {
            /* Case 2: More complex operations */
            long a = result * 3;
            long b = a + 150;
            long c = b - 75;
            long d = c << 3;
            long e = d >> 2;
            long f = e ^ d;
            long g = f | c;
            long h = g & b;
            long i = h + a;
            long j = i * 2;
            long k = j - 100;
            long l = k / 5;
            result = l << 1;
            break;
        }
        case 3: {
            /* Case 3: Nested operations */
            long a = result / 4;
            long b = a + 300;
            long c = b * 2;
            long d = c - 100;
            long e = d << 4;
            long f = e >> 3;
            long g = f ^ e;
            long h = g | d;
            long i = h & c;
            long j = i + b;
            long k = j - a;
            long l = k * 3;
            long m = l / 2;
            result = m + 50;
            break;
        }
        case 4: {
            /* Case 4: Maximum operations */
            long a = result + 500;
            long b = a - 250;
            long c = b * 4;
            long d = c / 2;
            long e = d << 2;
            long f = e >> 1;
            long g = f ^ 0xFF00;
            long h = g | 0xAA00;
            long i = h & 0x5500;
            long j = i + a;
            long k = j - b;
            long l = k * c;
            long m = l / d;
            long n = m << 1;
            long o = n >> 2;
            result = o + 1000;
            break;
        }
    }
    
    /* Inline assembly barrier after switch */
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int result_int = 0;
    volatile float result_float = 0.0f;
    volatile double result_double = 0.0;
    volatile long result_long = 0L;
    
    /* Read inputs or use defaults */
    int input1 = g_input1;
    int input2 = g_input2;
    float input3 = g_input3;
    float input4 = g_input4;
    
    /* If command line arguments provided, use them */
    if (argc > 1) {
        input1 = atoi(argv[1]);
    }
    if (argc > 2) {
        input2 = atoi(argv[2]);
    }
    
    /* Call all functions to trigger scheduler in different contexts */
    result_int = integer_heavy_computation(input1, input2, input1 + 1, input2 - 1);
    
    result_float = float_array_processing(input3, input4, 20);
    
    result_double = mixed_operations(input1, input3, (double)input2);
    
    result_long = switch_based_computation(input1, input2);
    
    /* Aggregate results to prevent dead code elimination */
    volatile long final_result = 0;
    final_result = (long)result_int + (long)result_float + (long)result_double + result_long;
    
    /* Print something to ensure execution */
    printf("Result checksum: %ld\n", final_result);
    
    return 0;
}
