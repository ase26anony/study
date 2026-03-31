/* test_scheduler_context.c
 * Complex program to trigger scheduler context allocation and cleanup
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile input to prevent constant propagation */
volatile int g_input = 0;
volatile float g_finput = 0.0f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int func1_intensive(int a, int b, int c, int d, int e) {
    /* Many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 << 2;
    v6 = v5 | 0xFF;
    v7 = v6 & 0x0F;
    v8 = v7 ^ v1;
    v9 = v8 + v2;
    v10 = v9 - v3;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10 * 3;
    v10 = v11 + 1;      /* WAR: v10 reused after v11 calculation */
    v12 = v11 * 2;
    v11 = v12 / 2;      /* WAW: v11 written again */
    
    /* More complex dependency chain */
    v13 = v10 + v11;
    v14 = v13 * v12;
    v15 = v14 - v9;
    v16 = v15 / v8;
    v17 = v16 << 1;
    v18 = v17 | 0xAA;
    v19 = v18 & 0x55;
    v20 = v19 ^ v13;
    
    /* Use all variables to prevent dead code elimination */
    v21 = v1 + v2 + v3 + v4 + v5;
    v22 = v6 + v7 + v8 + v9 + v10;
    v23 = v11 + v12 + v13 + v14 + v15;
    v24 = v16 + v17 + v18 + v19 + v20;
    
    /* Inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v21), "+r"(v22));
    
    v25 = v21 * v22;
    v26 = v23 / (v24 + 1);
    
    asm volatile("" : "+r"(v25), "+r"(v26));
    
    v27 = v25 - v26;
    v28 = v27 * 2;
    v29 = v28 + v21;
    v30 = v29 - v22;
    
    /* Control flow to create basic block boundaries */
    if (v30 > 1000) {
        v30 = v30 / 2;
        v29 = v29 * 3;
    } else {
        v30 = v30 * 3;
        v29 = v29 / 2;
    }
    
    return v30 + v29 + v27 + v28;
}

/* Function 2: Floating-point array processing with loops */
float func2_fp_loop(float base, int iterations) {
    float arr[32];
    float sum = 0.0f;
    int i, j;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1f;
    for (i = 2; i < 32; i++) {
        /* RAW dependencies in loop */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Nested loops with mixed operations */
    for (i = 0; i < iterations; i++) {
        float temp = 0.0f;
        for (j = 0; j < 16; j++) {
            /* Complex FP operations */
            temp += arr[j] * sinf((float)(i+j) * 0.1f);
            arr[j] = temp * 0.9f + arr[31-j] * 0.1f;
        }
        
        /* Conditional inside loop */
        if (temp > 100.0f) {
            for (j = 16; j < 32; j++) {
                arr[j] = arr[j] * 0.8f + temp * 0.2f;
            }
        } else {
            for (j = 16; j < 32; j++) {
                arr[j] = arr[j] * 1.2f - temp * 0.2f;
            }
        }
        
        sum += temp;
    }
    
    /* Final reduction with dependencies */
    float result = sum;
    for (i = 0; i < 32; i++) {
        result += arr[i] * (float)i;
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many locals */
double func3_mixed_ops(int x, float y, double z) {
    /* Many local variables of different types */
    int i1 = x, i2 = x+1, i3 = x+2, i4 = x+3, i5 = x+4;
    float f1 = y, f2 = y*2.0f, f3 = y*3.0f, f4 = y*4.0f, f5 = y*5.0f;
    double d1 = z, d2 = z*0.5, d3 = z*0.25, d4 = z*0.125, d5 = z*0.0625;
    
    /* Complex dependency chain across types */
    i1 = (int)(f1 * 10.0f) + i1;
    f2 = (float)(d1 * 2.0) + f2;
    d3 = (double)(i2 * 3) + d3;
    
    /* Inline assembly barriers */
    asm volatile("" : "+r"(i1), "+r"(i2), "+r"(i3));
    
    i3 = i1 * i2 + i3;
    f3 = f1 * f2 + f3;
    d4 = d1 * d2 + d4;
    
    /* Switch statement for control flow */
    switch (x % 5) {
        case 0:
            i4 = i3 << 2;
            f4 = f3 * 4.0f;
            d5 = d4 * 16.0;
            break;
        case 1:
            i4 = i3 >> 2;
            f4 = f3 / 4.0f;
            d5 = d4 / 16.0;
            break;
        case 2:
            i4 = i3 | 0xFF;
            f4 = f3 + 100.0f;
            d5 = d4 - 50.0;
            break;
        case 3:
            i4 = i3 & 0x0F;
            f4 = f3 - 100.0f;
            d5 = d4 + 50.0;
            break;
        default:
            i4 = i3 ^ 0xAA;
            f4 = f3 * f3;
            d5 = d4 * d4;
            break;
    }
    
    /* More operations using all variables */
    i5 = i4 + i1 + i2 + i3;
    f5 = f4 + f1 + f2 + f3;
    d1 = d5 + d1 + d2 + d3 + d4;
    
    /* Final computation with type conversions */
    double result = (double)i5 + (double)f5 + d1;
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Function 4: Switch statement with different operation blocks */
long func4_switch_blocks(int mode, long val) {
    long result = val;
    int i;
    
    switch (mode % 4) {
        case 0: {
            /* Block with integer arithmetic chain */
            long a = val, b = val+1, c = val+2, d = val+3;
            for (i = 0; i < 10; i++) {
                a = b + c;
                b = c * d;
                c = d - a;
                d = a / (b + 1);
            }
            result = a + b + c + d;
            break;
        }
        case 1: {
            /* Block with memory-like operations */
            long temp[8];
            for (i = 0; i < 8; i++) {
                temp[i] = val * i;
            }
            for (i = 1; i < 8; i++) {
                temp[i] = temp[i-1] + temp[i];
            }
            result = 0;
            for (i = 0; i < 8; i++) {
                result += temp[i];
            }
            break;
        }
        case 2: {
            /* Block with mixed operations and conditions */
            long x = val, y = val * 2, z = val * 3;
            for (i = 0; i < 15; i++) {
                if (i % 3 == 0) {
                    x = y + z;
                } else if (i % 3 == 1) {
                    y = z - x;
                } else {
                    z = x * y;
                }
                
                /* Inline assembly to prevent reordering */
                asm volatile("" : "+r"(x), "+r"(y), "+r"(z));
            }
            result = x + y + z;
            break;
        }
        case 3: {
            /* Block with complex dependency graph */
            long p = val, q = val, r = val, s = val;
            for (i = 0; i < 12; i++) {
                long t = p + q;
                p = q * r;
                q = r + s;
                r = s - t;
                s = t * p;
                
                if (i % 4 == 0) {
                    asm volatile("" : "+r"(p), "+r"(q));
                }
            }
            result = p + q + r + s;
            break;
        }
    }
    
    return result;
}

/* Main function that calls all complex functions */
int main(int argc, char *argv[]) {
    /* Use command line arguments or stdin for dynamic inputs */
    int base_int = g_input;
    float base_float = g_finput;
    
    if (argc > 1) {
        base_int = atoi(argv[1]);
        base_float = (float)atof(argv[1]);
    } else {
        /* Read from stdin if no arguments */
        scanf("%d %f", &base_int, &base_float);
    }
    
    /* Ensure inputs are non-zero to avoid trivial paths */
    if (base_int == 0) base_int = 42;
    if (base_float == 0.0f) base_float = 3.14159f;
    
    /* Call all functions with dynamic inputs */
    int r1 = func1_intensive(base_int, base_int+1, base_int+2, 
                            base_int+3, base_int+4);
    
    float r2 = func2_fp_loop(base_float, 5);
    
    double r3 = func3_mixed_ops(base_int, base_float, (double)base_int * 0.5);
    
    long r4 = func4_switch_blocks(base_int, (long)base_int * 100);
    
    /* Aggregate results into volatile sink to prevent dead code elimination */
    volatile double final_result = 0.0;
    final_result += (double)r1;
    final_result += (double)r2;
    final_result += r3;
    final_result += (double)r4;
    
    /* Print checksum to ensure all computations are used */
    printf("Result checksum: %f\n", final_result);
    
    return 0;
}
