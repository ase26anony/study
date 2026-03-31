/* test_scheduler_context.c
 * This program creates complex basic blocks with dependencies, control flow,
 * and register pressure to force GCC's instruction scheduler to allocate
 * and later free scheduling context data structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile inputs to prevent constant propagation */
volatile int input1 = 7;
volatile int input2 = 13;
volatile float input3 = 3.14159f;
volatile float input4 = 2.71828f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int integer_computation(int a, int b, int c, int d, int e, int f) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long chain of true data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 ^ f;
    v6 = v5 | a;
    v7 = v6 & b;
    v8 = v7 << 3;
    v9 = v8 >> 1;
    v10 = v9 + c;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10 * 2;      /* WAW with v10 if not careful */
    v12 = v11 - v1;     /* RAW on v11, WAR on v1 */
    v13 = v12 + v2;
    v14 = v13 * v3;
    v15 = v14 / v4;
    
    /* More operations to create scheduling complexity */
    v16 = v15 ^ v5;
    v17 = v16 | v6;
    v18 = v17 & v7;
    v19 = v18 << 2;
    v20 = v19 >> 1;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 + v8;
    v22 = v21 * v9;
    v23 = v22 - v10;
    v24 = v23 / v11;
    v25 = v24 ^ v12;
    
    /* Control flow to create multiple basic blocks */
    if (v25 > 1000) {
        v26 = v25 * 2;
        v27 = v26 - 50;
        v28 = v27 / 3;
    } else {
        v26 = v25 + 100;
        v27 = v26 * 4;
        v28 = v27 - 200;
    }
    
    /* Another dependency chain */
    v29 = v28 + v13;
    v30 = v29 * v14;
    
    /* Final result uses many variables to prevent optimization */
    return v15 + v20 + v25 + v30;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float base, int iterations) {
    float arr[32];
    float sum = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1f;
    
    for (int i = 2; i < 32; i++) {
        /* True dependencies across loop iterations */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
        
        /* Anti-dependency: reading arr[i-1], then modifying sum */
        sum += arr[i-1];
        
        /* Output dependency: arr[i] written each iteration */
    }
    
    /* Complex floating-point operations */
    float temp1 = arr[0] * arr[1];
    float temp2 = arr[2] / arr[3];
    float temp3 = temp1 + temp2;
    float temp4 = sqrtf(fabsf(temp3));
    
    /* Mixed integer/float operations */
    int int_temp = (int)temp4;
    float temp5 = temp4 * int_temp;
    
    /* Loop with conditional inside */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            temp5 += arr[i % 32] * 0.3f;
        } else if (i % 3 == 1) {
            temp5 -= arr[i % 32] * 0.7f;
        } else {
            temp5 *= 1.01f;
        }
        
        /* Another artificial dependency */
        asm volatile("" : "+r"(temp5));
    }
    
    return sum + temp5;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int a, float b, double c, int d) {
    /* Declare many variables of different types */
    int i1 = a, i2 = d, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1 = b, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1 = c, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Complex dependency chain mixing types */
    i3 = i1 * 2 + i2;
    f2 = f1 * 3.14f;
    d2 = d1 / 2.0;
    
    i4 = (int)f2 + i3;
    f3 = (float)i4 * 0.5f;
    d3 = d2 * f3;
    
    /* Control flow with dependencies carried across */
    if (i4 > 100) {
        i5 = i4 - 50;
        f4 = f3 * 2.0f;
        d4 = d3 + 10.0;
    } else {
        i5 = i4 + 50;
        f4 = f3 / 2.0f;
        d4 = d3 - 10.0;
    }
    
    /* More operations */
    i6 = i5 * 3;
    f5 = sinf(f4);
    d5 = cos(d4);
    
    i7 = i6 / 2;
    f6 = expf(f5);
    d6 = log(d5);
    
    /* Cross-type dependencies */
    i8 = i7 + (int)f6;
    f7 = f6 + (float)i8;
    d7 = d6 + (double)f7;
    
    /* Use all variables to prevent dead code elimination */
    i9 = i8 * i7;
    f8 = f7 * f6;
    d8 = d7 * d6;
    
    i10 = i9 + (int)d8;
    f9 = f8 + (float)i10;
    d9 = d8 + (double)f9;
    
    f10 = f9 * 0.1f;
    d10 = d9 * 0.01;
    
    return d10 + f10 + i10;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(int mode, long seed) {
    long result = seed;
    
    switch (mode % 5) {
        case 0: {
            /* Block with integer arithmetic */
            long a = result * 3;
            long b = a + 0x7FFF;
            long c = b ^ 0xABCD;
            long d = c << 3;
            long e = d >> 1;
            result = e - a;
            break;
        }
        case 1: {
            /* Block with memory-like operations */
            long temp[8];
            for (int i = 0; i < 8; i++) {
                temp[i] = result + i * 17;
            }
            for (int i = 1; i < 8; i++) {
                temp[i] += temp[i-1];
            }
            result = temp[7];
            break;
        }
        case 2: {
            /* Block with mixed operations */
            double d = (double)result;
            d = d * 1.23456789;
            d = sin(d) * cos(d);
            result = (long)d * 1000;
            break;
        }
        case 3: {
            /* Block with many dependencies */
            long x1 = result + 111;
            long x2 = x1 * 222;
            long x3 = x2 - 333;
            long x4 = x3 / 444;
            long x5 = x4 ^ 555;
            long x6 = x5 | 666;
            long x7 = x6 & 777;
            result = x7;
            break;
        }
        case 4: {
            /* Block with artificial dependencies */
            asm volatile("" : "+r"(result));
            long y = result;
            for (int i = 0; i < 10; i++) {
                y = y * 3 + i;
                asm volatile("" : "+r"(y));
            }
            result = y;
            break;
        }
    }
    
    return result;
}

/* Main function that ensures all functions are called */
int main(int argc, char *argv[]) {
    /* Use command line arguments or stdin to get dynamic values */
    int dynamic1, dynamic2;
    float dynamic3, dynamic4;
    
    if (argc > 2) {
        dynamic1 = atoi(argv[1]);
        dynamic2 = atoi(argv[2]);
        dynamic3 = atof(argv[3]);
        dynamic4 = atof(argv[4]);
    } else {
        /* Read from stdin if no arguments */
        printf("Enter 4 numbers (int int float float): ");
        scanf("%d %d %f %f", &dynamic1, &dynamic2, &dynamic3, &dynamic4);
    }
    
    /* Call all complex functions in sequence */
    int res1 = integer_computation(
        dynamic1, dynamic2, input1, input2, 
        dynamic1 * 2, dynamic2 / 2
    );
    
    float res2 = float_array_processing(
        dynamic3 * input3, 
        20 + (dynamic1 % 10)
    );
    
    double res3 = mixed_operations(
        dynamic1, dynamic3, 
        (double)dynamic4 * input4,
        dynamic2
    );
    
    long res4 = switch_based_computation(
        dynamic1, 
        (long)dynamic2 * 1000
    );
    
    /* Aggregate results into a volatile sink to prevent optimization */
    volatile long final_result = 0;
    final_result += res1;
    final_result += (long)res2;
    final_result += (long)res3;
    final_result += res4;
    
    /* Print checksum to ensure all computations are used */
    printf("Result checksum: %ld\n", final_result);
    
    return 0;
}
