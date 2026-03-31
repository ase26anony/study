/* test_scheduler_context.c
 * 
 * This program creates complex basic blocks with various dependencies,
 * control flow, and register pressure to force GCC's instruction scheduler
 * to allocate and later free scheduling context data structures.
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched test_scheduler_context.c -o test
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
    v6 = v5 ^ v1;
    v7 = v6 << 2;
    v8 = v7 >> 1;
    v9 = v8 & 0xFF;
    v10 = v9 + v2;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10 * 3;      /* WAR: v10 read before potential write */
    v10 = v11 + 5;      /* WAW: v10 written again */
    v12 = v10 - v11;    /* Complex dependency chain */
    
    /* More operations with mixed dependencies */
    v13 = v12 * v3;
    v14 = v13 / v4;
    v15 = v14 | v5;
    v16 = v15 ^ v6;
    v17 = v16 << v7;
    v18 = v17 >> v8;
    v19 = v18 & v9;
    v20 = v19 + v10;
    
    /* Control flow to create multiple basic blocks */
    if (v20 > 1000) {
        v21 = v20 * 2;
        v22 = v21 - 50;
        v23 = v22 / 3;
    } else {
        v21 = v20 / 2;
        v22 = v21 + 50;
        v23 = v22 * 3;
    }
    
    /* More operations in the merged block */
    v24 = v23 + v11;
    v25 = v24 - v12;
    v26 = v25 * v13;
    v27 = v26 / v14;
    v28 = v27 | v15;
    v29 = v28 ^ v16;
    v30 = v29 & v17;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v30));
    
    /* Final computation using most variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float a, float b, float c, float d) {
    float arr[32];
    float result = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = a;
    arr[1] = b;
    for (int i = 2; i < 32; i++) {
        /* True data dependencies across loop iterations */
        arr[i] = arr[i-1] * c + arr[i-2] * d;
        
        /* Anti-dependency: reading arr[i-1] before writing arr[i-2] */
        if (i % 3 == 0) {
            arr[i-2] = arr[i-1] * 0.5f;  /* WAR */
        }
        
        /* Output dependency */
        if (i % 5 == 0) {
            arr[i-1] = arr[i] * 2.0f;    /* WAW */
        }
    }
    
    /* Process array with mixed operations */
    for (int i = 0; i < 32; i++) {
        float temp = arr[i];
        
        /* Chain of floating-point operations */
        temp = temp * temp;
        temp = sqrtf(fabsf(temp));
        temp = sinf(temp) + cosf(temp);
        temp = expf(temp * 0.1f);
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : "+f"(temp));
        
        result += temp;
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int x, float y, double z) {
    /* Many local variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Initial computations with type conversions */
    i1 = x * 2;
    f1 = y * 3.14f;
    d1 = z * 2.71828;
    
    /* Complex control flow */
    if (x > 0) {
        i2 = i1 + x;
        f2 = f1 * y;
        d2 = d1 / z;
        
        /* Nested if for more basic blocks */
        if (y > 0) {
            i3 = i2 * 3;
            f3 = f2 + y;
            d3 = d2 * z;
        } else {
            i3 = i2 / 3;
            f3 = f2 - y;
            d3 = d2 / z;
        }
    } else {
        i2 = i1 - x;
        f2 = f1 / y;
        d2 = d1 * z;
        
        i3 = i2 % 7;
        f3 = f2 * 2.0f;
        d3 = d2 + 1.0;
    }
    
    /* More operations in the merged block */
    i4 = i3 << 2;
    f4 = f3 * f3;
    d4 = d3 * d3;
    
    i5 = i4 >> 1;
    f5 = sqrtf(f4);
    d5 = sqrt(d4);
    
    /* Loop with dependencies */
    for (int j = 0; j < 8; j++) {
        i5 = i5 + j;
        f5 = f5 * (1.0f + y);
        d5 = d5 * (1.0 + z);
    }
    
    /* Use all variables in final computation */
    i6 = i1 + i2 + i3 + i4 + i5;
    f6 = f1 + f2 + f3 + f4 + f5;
    d6 = d1 + d2 + d3 + d4 + d5;
    
    /* Inline assembly to create dependencies */
    asm volatile("" : "+r"(i6), "+f"(f6), "+r"((int)d6));
    
    return (double)i6 + (double)f6 + d6;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(int mode, long a, long b, long c) {
    long result = 0;
    
    switch (mode % 4) {
        case 0: {
            /* Block with many integer operations */
            long t1 = a * b;
            long t2 = t1 + c;
            long t3 = t2 - a;
            long t4 = t3 * b;
            long t5 = t4 / (c + 1);
            long t6 = t5 | t1;
            long t7 = t6 & t2;
            long t8 = t7 ^ t3;
            long t9 = t8 << 3;
            long t10 = t9 >> 1;
            result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
            break;
        }
        
        case 1: {
            /* Block with mixed operations and memory access pattern */
            long arr[10];
            for (int i = 0; i < 10; i++) {
                arr[i] = a + i * b;
            }
            
            for (int i = 1; i < 10; i++) {
                arr[i] = arr[i] + arr[i-1] * c;
            }
            
            for (int i = 0; i < 10; i++) {
                result += arr[i];
            }
            break;
        }
        
        case 2: {
            /* Block with complex dependency chain */
            long x1 = a;
            long x2 = b;
            long x3 = c;
            
            for (int i = 0; i < 5; i++) {
                x1 = x1 * x2 + x3;
                x2 = x2 * x3 + x1;
                x3 = x3 * x1 + x2;
                
                /* Inline assembly to prevent optimization */
                asm volatile("" : "+r"(x1), "+r"(x2), "+r"(x3));
            }
            
            result = x1 + x2 + x3;
            break;
        }
        
        case 3: {
            /* Block with many temporary variables */
            long v1 = a, v2 = b, v3 = c;
            long v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
            
            v4 = v1 * v2;
            v5 = v2 * v3;
            v6 = v3 * v1;
            v7 = v4 + v5;
            v8 = v5 + v6;
            v9 = v6 + v4;
            v10 = v7 - v8;
            v11 = v8 - v9;
            v12 = v9 - v7;
            v13 = v10 * v11;
            v14 = v11 * v12;
            v15 = v12 * v10;
            
            result = v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                     v11 + v12 + v13 + v14 + v15;
            break;
        }
    }
    
    return result;
}

/* Main function that calls all complex functions */
int main(int argc, char *argv[]) {
    /* Use command line arguments or default values */
    int base1 = (argc > 1) ? atoi(argv[1]) : g_input1;
    int base2 = (argc > 2) ? atoi(argv[2]) : g_input2;
    float base3 = (argc > 3) ? atof(argv[3]) : g_input3;
    float base4 = (argc > 4) ? atof(argv[4]) : g_input4;
    
    /* Volatile sink to prevent dead code elimination */
    volatile long total_result = 0;
    
    /* Call all complex functions in sequence */
    int res1 = integer_computation(base1, base2, base1+1, base2+1, 
                                   base1+2, base2+2);
    total_result += res1;
    
    float res2 = float_array_processing(base3, base4, base3*2, base4*2);
    total_result += (long)res2;
    
    double res3 = mixed_operations(base1, base3, base4);
    total_result += (long)res3;
    
    long res4 = switch_based_computation(base1, base2, base1*2, base2*2);
    total_result += res4;
    
    /* Additional calls with different parameters */
    for (int i = 0; i < 3; i++) {
        total_result += integer_computation(base1+i, base2-i, 
                                           base1+i+1, base2-i+1,
                                           base1+i+2, base2-i+2);
    }
    
    /* Print result to prevent optimization */
    printf("Total checksum: %ld\n", total_result);
    
    return 0;
}
