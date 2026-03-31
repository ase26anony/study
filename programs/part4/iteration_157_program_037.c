/* test_scheduler_context.c
 * Complex program to trigger scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int g_input1 = 42;
volatile int g_input2 = 17;
volatile float g_input3 = 3.14159f;
volatile double g_input4 = 2.71828;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_int_chain(int a, int b, int c, int d, int e) {
    /* Create register pressure with many variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initial computations with dependencies */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 << 2;
    v6 = v5 | 0xFF;
    v7 = v6 & 0x0F;
    v8 = v7 ^ v1;
    v9 = v8 * v2;
    v10 = v9 + v3;
    
    /* Artificial dependency chain with inline assembly */
    asm volatile("" : "+r"(v10));
    
    v11 = v10 - v4;
    v12 = v11 * v5;
    v13 = v12 / 3;
    v14 = v13 << 1;
    v15 = v14 | v6;
    v16 = v15 & 0x7F;
    v17 = v16 ^ v7;
    v18 = v17 * v8;
    v19 = v18 + v9;
    v20 = v19 - v10;
    
    /* More computations with cross-dependencies */
    v21 = v20 * v11;
    v22 = v21 / v12;
    v23 = v22 + v13;
    v24 = v23 - v14;
    v25 = v24 | v15;
    v26 = v25 & v16;
    v27 = v26 ^ v17;
    v28 = v27 * v18;
    v29 = v28 + v19;
    v30 = v29 - v20;
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* Branch with more computations */
        v30 = v30 * 2;
        v29 = v29 / 2;
        v28 = v28 + 100;
    } else {
        /* Alternative branch */
        v30 = v30 / 2;
        v29 = v29 * 2;
        v28 = v28 - 50;
    }
    
    /* Final computation using all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
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
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Complex loop with multiple dependencies */
    for (i = 0; i < iterations; i++) {
        float temp = 0.0f;
        
        /* Inner computations */
        for (j = 0; j < 16; j++) {
            temp += arr[j] * sinf((float)j * 0.1f);
            arr[j] = arr[j] * cosf((float)i * 0.05f) + 1.0f;
        }
        
        /* Anti-dependencies and output dependencies */
        result = result + temp;
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : "+r"(result));
        
        /* More computations */
        for (j = 16; j < 32; j++) {
            arr[j] = arr[j] - tanf((float)(i+j) * 0.01f);
            result = result + arr[j];
        }
        
        /* Conditional inside loop */
        if (result > 1000.0f) {
            result = result * 0.9f;
            for (j = 0; j < 32; j++) {
                arr[j] = arr[j] * 0.95f;
            }
        }
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int mode, double x, double y) {
    /* Many local variables for register pressure */
    double a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    
    /* Initial mixed computations */
    a1 = x + y;
    a2 = x - y;
    a3 = x * y;
    a4 = x / (y + 1.0);
    
    c1 = (int)x;
    c2 = (int)y;
    c3 = c1 + c2;
    c4 = c1 * c2;
    
    f1 = (float)x;
    f2 = (float)y;
    f3 = f1 * f2;
    f4 = f1 / f2;
    
    /* Complex control flow */
    switch (mode % 4) {
        case 0:
            /* Integer-heavy path */
            a5 = (double)(c3 * c4);
            a6 = a5 + a1;
            c5 = c3 << 2;
            c6 = c4 >> 1;
            f5 = f3 + f4;
            break;
            
        case 1:
            /* Float-heavy path */
            a5 = (double)(f3 * f4);
            a6 = a5 - a2;
            f5 = f3 * 2.0f;
            f6 = f4 / 2.0f;
            c5 = (int)f5;
            break;
            
        case 2:
            /* Mixed path with dependencies */
            a5 = a3 * a4;
            a6 = a5 / a1;
            c5 = (int)a5;
            f5 = (float)a6;
            f6 = f5 * 2.0f;
            break;
            
        default:
            /* Complex path */
            a5 = sin(x) * cos(y);
            a6 = exp(a5);
            c5 = (int)(a6 * 100);
            f5 = (float)log(fabs(a6) + 1.0);
            break;
    }
    
    /* More computations using all variables */
    a7 = a1 + a2 + a3 + a4 + a5 + a6;
    a8 = a7 * 0.5;
    a9 = sqrt(fabs(a8));
    a10 = a9 * 2.0;
    
    c7 = c1 + c2 + c3 + c4 + c5;
    c8 = c7 * 3;
    c9 = c8 / 2;
    c10 = c9 | 0xFF;
    
    f7 = f1 + f2 + f3 + f4 + f5;
    f8 = f7 * 1.5f;
    f9 = f8 / 3.0f;
    f10 = sqrtf(fabsf(f9));
    
    /* Artificial dependency */
    asm volatile("" : "+r"(a10), "+r"(c10), "+r"(f10));
    
    /* Final result combining all types */
    return a10 + (double)c10 + (double)f10;
}

/* Function 4: Switch statement with different operation blocks */
long switch_based_computation(int selector, long base) {
    long result = base;
    int i;
    
    switch (selector % 6) {
        case 0: {
            /* Block with sequential dependencies */
            long a = base * 2;
            long b = a + 100;
            long c = b * 3;
            long d = c - 50;
            long e = d / 2;
            long f = e | 0xAAAA;
            long g = f & 0x5555;
            long h = g ^ 0x3333;
            result = a + b + c + d + e + f + g + h;
            break;
        }
            
        case 1: {
            /* Block with loop and dependencies */
            long arr[10];
            arr[0] = base;
            for (i = 1; i < 10; i++) {
                arr[i] = arr[i-1] * i + 1;
            }
            for (i = 0; i < 9; i++) {
                arr[i] = arr[i] + arr[i+1];
            }
            result = 0;
            for (i = 0; i < 10; i++) {
                result += arr[i];
            }
            break;
        }
            
        case 2: {
            /* Block with many temporary variables */
            long t1 = base + 1;
            long t2 = t1 * 2;
            long t3 = t2 - 3;
            long t4 = t3 / 4;
            long t5 = t4 | 0xFF;
            long t6 = t5 & 0xF0;
            long t7 = t6 ^ 0x0F;
            long t8 = t7 << 2;
            long t9 = t8 >> 1;
            long t10 = t9 + 100;
            long t11 = t10 * 2;
            long t12 = t11 - 50;
            long t13 = t12 / 3;
            long t14 = t13 | 0xCC;
            long t15 = t14 & 0x33;
            result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                     t11 + t12 + t13 + t14 + t15;
            break;
        }
            
        case 3: {
            /* Block with conditional computations */
            long x = base;
            for (i = 0; i < 8; i++) {
                if (x % 2 == 0) {
                    x = x / 2;
                } else {
                    x = x * 3 + 1;
                }
                x = x + i;
            }
            result = x;
            break;
        }
            
        case 4: {
            /* Block with mixed operations */
            double d = (double)base;
            d = d * 1.5;
            d = sin(d) * 100.0;
            d = d + cos(d * 0.1);
            result = (long)d;
            break;
        }
            
        default: {
            /* Complex default block */
            long sum = 0;
            for (i = 0; i < 20; i++) {
                long val = base * i;
                if (i % 3 == 0) val = val + 100;
                if (i % 4 == 0) val = val * 2;
                if (i % 5 == 0) val = val - 50;
                sum += val;
                
                /* Inline assembly to create barrier */
                asm volatile("" : "+r"(sum));
            }
            result = sum;
            break;
        }
    }
    
    return result;
}

/* Main function to drive all computations */
int main(int argc, char *argv[]) {
    int result_int;
    float result_float;
    double result_double;
    long result_long;
    volatile int sink = 0;
    
    /* Use command line arguments or defaults to prevent constant propagation */
    int input1 = (argc > 1) ? atoi(argv[1]) : g_input1;
    int input2 = (argc > 2) ? atoi(argv[2]) : g_input2;
    float input3 = (argc > 3) ? atof(argv[3]) : g_input3;
    double input4 = (argc > 4) ? atof(argv[4]) : g_input4;
    
    /* Call all complex functions */
    result_int = complex_int_chain(input1, input2, input1+1, input2-1, 5);
    result_float = floating_point_processing(input3, 10);
    result_double = mixed_operations(input1 % 4, input4, input4 * 0.5);
    result_long = switch_based_computation(input2, input1 * 100L);
    
    /* Aggregate results to prevent dead code elimination */
    sink = result_int + (int)result_float + (int)result_double + (int)result_long;
    
    /* Print checksum to ensure all computations are used */
    printf("Checksum: %d\n", sink);
    
    return 0;
}
