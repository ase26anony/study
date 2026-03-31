/* test_scheduler_context.c
 * A program designed to trigger GCC's instruction scheduler context allocation
 * and cleanup, specifically covering the free_sched_context block in haifa-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile variables to prevent constant propagation */
volatile int g_input1 = 42;
volatile int g_input2 = 17;
volatile float g_input3 = 3.14159f;
volatile double g_input4 = 2.71828;

/* Function 1: Integer-heavy computation with many serial dependencies */
int integer_heavy_computation(int a, int b, int c) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - a;
    v4 = v3 / b;
    v5 = v4 << 2;
    v6 = v5 | v1;
    v7 = v6 & 0xFF;
    v8 = v7 ^ v2;
    v9 = v8 + v3;
    v10 = v9 - v4;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10;          /* WAR: v10 read before potential write */
    v10 = v5 + v6;      /* WAR on v10 */
    v12 = v11 * 2;      /* Use v11 */
    v11 = v12 + 1;      /* WAR on v11 */
    
    /* More complex dependency chain */
    v13 = v11 + v12;
    v14 = v13 * v10;
    v15 = v14 - v9;
    v16 = v15 / v8;
    v17 = v16 << 1;
    v18 = v17 | v7;
    v19 = v18 & 0x7F;
    v20 = v19 ^ v6;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 + v5;
    v22 = v21 * v4;
    v23 = v22 - v3;
    v24 = v23 / v2;
    
    /* Control flow to create multiple basic blocks */
    if (v24 > 1000) {
        v25 = v24 >> 2;
        v26 = v25 * 3;
        v27 = v26 + v1;
    } else {
        v25 = v24 << 2;
        v26 = v25 / 3;
        v27 = v26 - v1;
    }
    
    v28 = v27 + v20;
    v29 = v28 * v15;
    v30 = v29 - v10;
    
    /* Ensure all variables are used to prevent dead code elimination */
    return v30 + v25 + v11 + v1;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float base, int iterations) {
    float arr[32];
    float sum = 0.0f;
    int i, j;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 2.0f;
    for (i = 2; i < 32; i++) {
        /* True data dependencies in loop */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Nested loops with mixed operations */
    for (i = 0; i < iterations; i++) {
        float temp = arr[i % 32];
        
        /* Floating-point operations with dependencies */
        temp = temp * 1.1f;
        temp = temp + sinf(temp);
        temp = temp * cosf(temp);
        temp = temp / (temp + 1.0f);
        
        /* Memory access with address calculation */
        arr[(i + 1) % 32] = arr[i % 32] + temp;
        
        sum += temp;
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : "+r"(i), "+m"(arr[0]));
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int a, float b, double c) {
    /* Many local variables of different types */
    int i1 = a, i2 = a * 2, i3 = a + 1, i4 = a - 1, i5 = a * 3;
    int i6, i7, i8, i9, i10, i11, i12, i13, i14, i15;
    float f1 = b, f2 = b * 2.0f, f3 = b + 1.0f, f4 = b - 1.0f;
    float f5, f6, f7, f8, f9, f10;
    double d1 = c, d2 = c * 2.0, d3 = c + 1.0, d4 = c - 1.0;
    double d5, d6, d7, d8, d9, d10;
    
    /* Complex control flow with multiple basic blocks */
    if (a > 0) {
        /* Block A: Integer operations */
        i6 = i1 + i2;
        i7 = i6 * i3;
        i8 = i7 - i4;
        i9 = i8 / i5;
        i10 = i9 << 3;
        
        f5 = f1 * f2;
        f6 = f5 + f3;
        f7 = f6 - f4;
        
        d5 = d1 * d2;
        d6 = d5 + d3;
        d7 = d6 - d4;
    } else {
        /* Block B: Different operations */
        i6 = i1 - i2;
        i7 = i6 / (i3 + 1);
        i8 = i7 * i4;
        i9 = i8 | i5;
        i10 = i9 >> 2;
        
        f5 = f1 / f2;
        f6 = f5 * f3;
        f7 = f6 + f4;
        
        d5 = d1 / d2;
        d6 = d5 * d3;
        d7 = d6 + d4;
    }
    
    /* Another level of control flow */
    switch (a % 4) {
        case 0:
            i11 = i10 + 100;
            f8 = f7 * 2.0f;
            d8 = d7 * 2.0;
            break;
        case 1:
            i11 = i10 - 100;
            f8 = f7 / 2.0f;
            d8 = d7 / 2.0;
            break;
        case 2:
            i11 = i10 * 2;
            f8 = f7 + 10.0f;
            d8 = d7 + 10.0;
            break;
        default:
            i11 = i10 / 2;
            f8 = f7 - 10.0f;
            d8 = d7 - 10.0;
            break;
    }
    
    /* More operations using all variable types */
    i12 = i11 + (int)f8;
    f9 = f8 + (float)i12;
    d9 = d8 + (double)f9;
    
    i13 = i12 * 3;
    f10 = f9 * 1.5f;
    d10 = d9 * 1.5;
    
    /* Artificial dependency chain with inline assembly */
    asm volatile("" : "+r"(i13), "+r"(f10), "+r"(d10));
    
    i14 = i13 + (int)d10;
    i15 = i14 ^ (int)f10;
    
    /* Use all variables in return to prevent optimization */
    return d10 + f10 + i15;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(int mode, long seed) {
    long result = seed;
    int i;
    
    switch (mode % 5) {
        case 0:
            /* Integer arithmetic chain */
            for (i = 0; i < 10; i++) {
                result = result * 1103515245 + 12345;
                result = (result >> 16) & 0x7FFF;
                result = result ^ (result << 13);
            }
            break;
            
        case 1:
            /* Bit manipulation operations */
            result = result | 0x5555555555555555UL;
            result = result & 0xAAAAAAAAAAAAAAAUL;
            result = result ^ 0xFFFFFFFFFFFFFFFFUL;
            result = (result << 5) | (result >> 59);
            result = ~result;
            break;
            
        case 2:
            /* Mixed operations with memory */
            {
                long temp[8];
                for (i = 0; i < 8; i++) {
                    temp[i] = result + i;
                }
                for (i = 0; i < 8; i++) {
                    result = result ^ temp[i];
                }
                result = result * 6364136223846793005UL;
            }
            break;
            
        case 3:
            /* Complex dependency chain */
            {
                long a = result, b = result * 2, c = result + 1;
                for (i = 0; i < 8; i++) {
                    a = a + b;
                    b = b ^ c;
                    c = c * a;
                    asm volatile("" : "+r"(a), "+r"(b), "+r"(c));
                }
                result = a + b + c;
            }
            break;
            
        default:
            /* Simple but many operations */
            result = result + 1;
            result = result * 3;
            result = result - 2;
            result = result / 2;
            result = result | 1;
            result = result << 4;
            result = result >> 1;
            result = result & 0x0F0F0F0F0F0F0F0FUL;
            break;
    }
    
    return result;
}

/* Main function that calls all test functions */
int main(int argc, char *argv[]) {
    int int_result;
    float float_result;
    double double_result;
    long long_result;
    volatile int sink = 0;  /* Volatile sink to prevent optimization */
    
    /* Read inputs to make them dynamic */
    int base_int = g_input1;
    if (argc > 1) {
        base_int = atoi(argv[1]);
    }
    
    /* Call all complex functions */
    int_result = integer_heavy_computation(base_int, g_input2, base_int + g_input2);
    sink += int_result;
    
    float_result = float_array_processing(g_input3, 20);
    sink += (int)float_result;
    
    double_result = mixed_operations(base_int, g_input3, g_input4);
    sink += (int)double_result;
    
    long_result = switch_based_computation(base_int, 0x123456789ABCDEFUL);
    sink += (int)long_result;
    
    /* Print checksum to ensure all computations are used */
    printf("Checksum: %d\n", sink);
    
    return 0;
}
