/* test_scheduler_context.c
 * 
 * This program creates complex basic blocks with various dependencies,
 * control flow, and register pressure to force GCC's instruction scheduler
 * to allocate and later free scheduling context during compilation.
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
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 | v1;
    v6 = v5 & v2;
    v7 = v6 ^ v3;
    v8 = v7 << 2;
    v9 = v8 >> 1;
    v10 = v9 + v4;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10;          /* WAW if v11 was previously used */
    v12 = v11 + v1;     /* RAW on v11, WAR on v1 */
    v1 = v12 * 2;       /* WAW on v1, RAW on v12 */
    v13 = v1 - v2;      /* RAW on v1 and v2 */
    v2 = v13 / 3;       /* WAW on v2, RAW on v13 */
    
    /* More complex dependency chains */
    v14 = (v2 * v3) + (v4 * v5);
    v15 = v14 % 17;
    v16 = v15 | v6;
    v17 = v16 & 0xFF;
    v18 = v17 ^ v7;
    v19 = v18 << v8;
    v20 = v19 >> 1;
    
    /* Control flow to create multiple basic blocks */
    if (v20 > 1000) {
        v21 = v20 * 2;
        v22 = v21 + v9;
        v23 = v22 - v10;
    } else {
        v21 = v20 / 2;
        v22 = v21 - v9;
        v23 = v22 + v10;
    }
    
    /* Another dependency chain in both branches */
    v24 = v23 * v11;
    v25 = v24 + v12;
    v26 = v25 - v13;
    v27 = v26 / v14;
    v28 = v27 | v15;
    v29 = v28 & v16;
    v30 = v29 ^ v17;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v30));
    
    /* Final computation using most variables */
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
        /* True dependencies in loop */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Complex floating-point operations */
    for (int iter = 0; iter < iterations; iter++) {
        float temp[32];
        
        /* Multiple dependency chains */
        for (int i = 0; i < 32; i++) {
            temp[i] = arr[i] * arr[(i+1)%32];
        }
        
        /* More operations with anti-dependencies */
        for (int i = 0; i < 32; i++) {
            arr[i] = temp[i] + sinf(arr[i]) * 0.1f;
        }
        
        /* Conditional inside loop */
        if (iter % 3 == 0) {
            for (int i = 0; i < 16; i++) {
                arr[i] = arr[i] * 2.0f - arr[31-i];
            }
        } else if (iter % 3 == 1) {
            for (int i = 0; i < 16; i++) {
                arr[i] = arr[i] / 1.5f + arr[16+i];
            }
        }
        
        /* Accumulate sum */
        for (int i = 0; i < 32; i++) {
            sum += arr[i];
        }
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many locals */
double func3_mixed_control(int seed, double factor) {
    /* Declare many local variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    
    /* Initialize with seed */
    i1 = seed;
    d1 = (double)seed * factor;
    f1 = (float)seed / factor;
    
    /* Complex control flow with switch */
    switch (seed % 5) {
        case 0:
            i2 = i1 * 2;
            d2 = d1 * 1.5;
            f2 = f1 + 0.1f;
            i3 = i2 + i1;
            d3 = d2 - d1;
            f3 = f2 * f1;
            break;
        case 1:
            i2 = i1 / 2;
            d2 = d1 / 1.5;
            f2 = f1 - 0.1f;
            i3 = i2 | i1;
            d3 = d2 + d1;
            f3 = f2 / f1;
            break;
        case 2:
            i2 = i1 << 1;
            d2 = sin(d1);
            f2 = cosf(f1);
            i3 = i2 & i1;
            d3 = cos(d2);
            f3 = sinf(f2);
            break;
        case 3:
            i2 = i1 >> 1;
            d2 = exp(d1);
            f2 = logf(f1);
            i3 = i2 ^ i1;
            d3 = log(d2);
            f3 = expf(f2);
            break;
        default:
            i2 = i1 + 100;
            d2 = sqrt(d1);
            f2 = sqrtf(f1);
            i3 = i2 * i1;
            d3 = pow(d2, 2.0);
            f3 = powf(f2, 2.0f);
            break;
    }
    
    /* More operations after switch */
    i4 = i3 * i2;
    d4 = d3 * d2;
    f4 = f3 * f2;
    
    i5 = i4 / 7;
    d5 = d4 / 3.14;
    f5 = f4 / 2.71f;
    
    /* Nested if-else */
    if (i5 > 0) {
        i6 = i5 * 3;
        d6 = d5 * 1.1;
        f6 = f5 * 1.2f;
        
        if (d6 > 10.0) {
            i7 = i6 + 50;
            d7 = d6 - 5.0;
            f7 = f6 + 1.0f;
        } else {
            i7 = i6 - 50;
            d7 = d6 + 5.0;
            f7 = f6 - 1.0f;
        }
    } else {
        i6 = i5 * 2;
        d6 = d5 * 0.9;
        f6 = f5 * 0.8f;
        
        i7 = i6 | 0xFF;
        d7 = d6 * 0.5;
        f7 = f6 * 0.5f;
    }
    
    /* Final computations using all variables */
    i8 = i7 + i6 + i5 + i4 + i3 + i2 + i1;
    d8 = d7 + d6 + d5 + d4 + d3 + d2 + d1;
    f8 = f7 + f6 + f5 + f4 + f3 + f2 + f1;
    
    i9 = i8 * 2;
    d9 = d8 * 1.5;
    f9 = f8 * 1.1f;
    
    i10 = i9 % 1000;
    d10 = fmod(d9, 100.0);
    f10 = fmodf(f9, 10.0f);
    
    /* Use inline assembly to prevent optimization */
    asm volatile("" : "+r"(i10), "+r"(d10), "+r"(f10));
    
    return (double)i10 + d10 + (double)f10;
}

/* Function 4: Switch statement with different operation blocks */
long func4_switch_blocks(int mode, long val) {
    long result = val;
    
    switch (mode % 6) {
        case 0: {
            /* Block with integer arithmetic chain */
            long a = result * 3;
            long b = a + result;
            long c = b - a;
            long d = c * b;
            long e = d / (a + 1);
            long f = e | c;
            long g = f & d;
            long h = g ^ e;
            result = h << 2;
            break;
        }
        case 1: {
            /* Block with memory-like operations */
            long temp[8];
            for (int i = 0; i < 8; i++) {
                temp[i] = result + i;
            }
            for (int i = 1; i < 8; i++) {
                temp[i] = temp[i] * temp[i-1];
            }
            result = 0;
            for (int i = 0; i < 8; i++) {
                result += temp[i];
            }
            break;
        }
        case 2: {
            /* Block with conditional operations */
            long x = result;
            long y = x * 2;
            long z = y + 100;
            
            if (x > y) {
                result = (x - y) * z;
            } else if (x < y) {
                result = (y - x) / (z + 1);
            } else {
                result = x | y | z;
            }
            
            /* Nested condition */
            for (int i = 0; i < 4; i++) {
                if (result % 2 == 0) {
                    result = result >> 1;
                } else {
                    result = result * 3 + 1;
                }
            }
            break;
        }
        case 3: {
            /* Block with mixed-width operations */
            int i1 = (int)result;
            short s1 = (short)result;
            char c1 = (char)result;
            
            i1 = i1 * s1 + c1;
            s1 = (short)(i1 / (c1 + 1));
            c1 = (char)(s1 | i1);
            
            result = (long)i1 * (long)s1 * (long)c1;
            break;
        }
        case 4: {
            /* Block with dependency chain across many ops */
            long chain[10];
            chain[0] = result;
            for (int i = 1; i < 10; i++) {
                chain[i] = chain[i-1] * 2 + i;
            }
            
            /* Reverse chain */
            for (int i = 8; i >= 0; i--) {
                chain[i] = chain[i+1] - chain[i];
            }
            
            result = 0;
            for (int i = 0; i < 10; i++) {
                result += chain[i];
            }
            break;
        }
        default: {
            /* Default block with bit operations */
            result = ~result;
            result = result ^ 0xAAAAAAAA;
            result = result | 0x55555555;
            result = result & 0xFFFFFFFF;
            result = (result << 16) | (result >> 16);
            result = result * 1103515245 + 12345;
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
    volatile double input5;
    
    /* Read inputs or use command line arguments */
    if (argc > 1) {
        input1 = atoi(argv[1]);
        input2 = atoi(argv[argc > 2 ? 2 : 1]);
        input3 = atoi(argv[argc > 3 ? 3 : 1]);
        input4 = atof(argv[argc > 4 ? 4 : "1.5"]);
        input5 = atof(argv[argc > 5 ? 5 : "2.5"]);
    } else {
        /* Default values if no arguments */
        input1 = 42;
        input2 = 17;
        input3 = 99;
        input4 = 1.5f;
        input5 = 2.5;
    }
    
    /* Call all functions to ensure they're compiled and executed */
    int result1 = func1_intensive(input1, input2, input3, input1 + input2, input2 + input3);
    float result2 = func2_fp_loop(input4, 5);
    double result3 = func3_mixed_control(input1, input5);
    long result4 = func4_switch_blocks(input2, input3);
    
    /* Aggregate results to a volatile sink */
    volatile double final_result = 0.0;
    final_result += (double)result1;
    final_result += (double)result2;
    final_result += result3;
    final_result += (double)result4;
    
    /* Print to prevent dead code elimination */
    printf("Result: %f\n", final_result);
    
    return 0;
}
