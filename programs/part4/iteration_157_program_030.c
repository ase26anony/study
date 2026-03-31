/* test_scheduler_context.c
 * Complex program to trigger GCC scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile input to prevent constant propagation */
volatile int g_input = 42;
volatile float g_float_input = 3.14159f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_int_computation(int a, int b, int c) {
    /* Create many local variables to increase register pressure */
    int v1 = a + b;
    int v2 = b * c;
    int v3 = v1 ^ v2;
    int v4 = v3 - a;
    int v5 = v4 * b;
    int v6 = v5 / (c + 1);
    int v7 = v6 << 2;
    int v8 = v7 | v3;
    int v9 = v8 & 0xFFFF;
    int v10 = v9 + v4;
    int v11 = v10 * v5;
    int v12 = v11 - v6;
    int v13 = v12 >> 1;
    int v14 = v13 ^ v8;
    int v15 = v14 + v9;
    int v16 = v15 * v10;
    int v17 = v16 % 997;
    int v18 = v17 | v11;
    int v19 = v18 & v12;
    int v20 = v19 + v13;
    
    /* Artificial dependency chain with inline asm */
    asm volatile("" : "+r"(v20));
    
    /* More operations with anti-dependencies */
    v1 = v20 + v14;
    v2 = v1 * v15;
    v3 = v2 - v16;
    v4 = v3 / (v17 + 1);
    v5 = v4 ^ v18;
    
    /* Output dependency chain */
    int result = v5;
    result = result + v19;
    result = result * v20;
    result = result - v1;
    
    return result;
}

/* Function 2: Floating-point array processing with loops */
float floating_point_processing(float base, int iterations) {
    /* Many local float variables for register pressure */
    float f1 = base;
    float f2 = base * 1.1f;
    float f3 = base * 0.9f;
    float f4 = base + 1.0f;
    float f5 = base - 0.5f;
    float f6, f7, f8, f9, f10;
    
    /* Loop with data dependencies */
    for (int i = 0; i < iterations; i++) {
        f1 = f1 * f2 + f3;
        f2 = f2 - f4 * 0.1f;
        f3 = f3 + f1 * 0.01f;
        f4 = f4 / (f2 + 0.001f);
        f5 = f5 * f3 - f4;
        
        /* Memory access with address calculation */
        float temp[5];
        temp[0] = f1;
        temp[1] = f2;
        temp[2] = f3;
        temp[3] = f4;
        temp[4] = f5;
        
        /* Process array with dependencies */
        f6 = temp[0] + temp[1];
        f7 = temp[1] * temp[2];
        f8 = temp[2] - temp[3];
        f9 = temp[3] / (temp[4] + 0.0001f);
        f10 = f6 * f7 - f8 / f9;
        
        /* Anti-dependencies */
        f1 = f10 + 0.5f;
        f2 = f1 * 0.8f;
    }
    
    /* Complex final computation */
    float result = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
    
    /* Inline asm to prevent optimization */
    asm volatile("" : "+f"(result));
    
    return result;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int mode, double x, double y) {
    /* Declare many variables to stress register allocation */
    double d1 = x;
    double d2 = y;
    double d3 = x + y;
    double d4 = x * y;
    double d5 = x - y;
    double d6 = x / (y + 0.001);
    double d7, d8, d9, d10, d11, d12, d13, d14, d15;
    
    /* Control flow with different dependency patterns */
    if (mode % 3 == 0) {
        /* Block A: Serial floating point chain */
        d7 = sin(d1) + cos(d2);
        d8 = d7 * tan(d3);
        d9 = d8 - exp(d4);
        d10 = d9 / (log(fabs(d5)) + 1.0);
        d11 = d10 * d6;
        
        /* Memory operations */
        double arr[4];
        arr[0] = d7;
        arr[1] = d8;
        arr[2] = d9;
        arr[3] = d10;
        
        d12 = arr[0] * arr[1] + arr[2] - arr[3];
    } 
    else if (mode % 3 == 1) {
        /* Block B: Different dependency pattern */
        d7 = d1 * d1 + d2 * d2;
        d8 = sqrt(d7);
        d9 = d3 * d3 - d4 * d4;
        d10 = pow(d5, 2.0) + pow(d6, 2.0);
        d11 = d8 + d9 + d10;
        
        /* More operations with output dependencies */
        d12 = d11;
        d12 = d12 * 0.5;
        d12 = d12 + d7;
    } 
    else {
        /* Block C: Another pattern */
        d7 = d1 + d2 + d3 + d4 + d5 + d6;
        d8 = d7 * 0.318309886;
        d9 = d8 - 1.570796327;
        d10 = d9 * d9;
        d11 = d10 / (d7 + 0.000001);
        d12 = d11 * 2.0;
    }
    
    /* Common post-processing with many dependencies */
    d13 = d7 + d8;
    d14 = d9 * d10;
    d15 = d11 - d12;
    
    double result = d13 * d14 / (d15 + 0.000001);
    
    /* Inline asm to create artificial dependency */
    asm volatile("" : "+r"(mode), "+f"(result));
    
    return result;
}

/* Function 4: Switch statement with different operation blocks */
long switch_based_computation(int selector, long a, long b, long c) {
    long result = 0;
    
    switch (selector % 5) {
        case 0: {
            /* Case 0: Many integer operations with dependencies */
            long l1 = a + b;
            long l2 = b * c;
            long l3 = l1 ^ l2;
            long l4 = l3 - a;
            long l5 = l4 * b;
            long l6 = l5 / (c + 1);
            long l7 = l6 << 3;
            long l8 = l7 | l3;
            long l9 = l8 & 0xFFFFFF;
            result = l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
            break;
        }
        case 1: {
            /* Case 1: Different pattern with memory access */
            long arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = a * i + b * (i + 1) + c * (i + 2);
            }
            
            /* Process array with dependencies */
            long sum = 0;
            for (int i = 0; i < 7; i++) {
                arr[i] = arr[i] + arr[i + 1];
                sum += arr[i];
            }
            result = sum;
            break;
        }
        case 2: {
            /* Case 2: Complex bit manipulation */
            result = a;
            for (int i = 0; i < 16; i++) {
                result = (result << 1) | ((result >> 31) & 1);
                result = result ^ b;
                result = result + c;
                result = result & 0xFFFFFFFF;
            }
            break;
        }
        case 3: {
            /* Case 3: Mixed operations */
            result = (a * b) / (c + 1);
            result = result + (a << 2);
            result = result - (b >> 1);
            result = result ^ c;
            result = result * 1103515245 + 12345;
            break;
        }
        case 4: {
            /* Case 4: Another dependency chain */
            long t1 = a;
            long t2 = b;
            long t3 = c;
            for (int i = 0; i < 10; i++) {
                t1 = t1 + t2;
                t2 = t2 * t3;
                t3 = t3 - t1;
                t1 = t1 ^ t2;
                t2 = t2 | t3;
                t3 = t3 & t1;
            }
            result = t1 + t2 + t3;
            break;
        }
    }
    
    /* Final computation that uses all inputs */
    result = result + a - b * c;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Main function that ensures all code paths are executed */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int input1 = g_input;
    volatile float input2 = g_float_input;
    
    /* Read from command line if available for more variability */
    int dynamic_input = 10;
    if (argc > 1) {
        dynamic_input = atoi(argv[1]);
    }
    
    /* Call all complex functions with dynamic inputs */
    int result1 = complex_int_computation(input1, input1 + 1, input1 + 2);
    float result2 = floating_point_processing(input2, dynamic_input % 20 + 5);
    double result3 = mixed_operations(dynamic_input, input1 * 0.5, input1 * 0.3);
    long result4 = switch_based_computation(dynamic_input, input1, input1 * 2L, input1 * 3L);
    
    /* Aggregate results into a volatile sink to prevent optimization */
    volatile double final_result = 0;
    final_result += result1;
    final_result += result2;
    final_result += result3;
    final_result += result4;
    
    /* Print checksum to ensure all computations are used */
    printf("Result checksum: %f\n", final_result);
    
    return 0;
}
