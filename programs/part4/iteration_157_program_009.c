/* test_scheduler_context.c
 * 
 * This program creates complex basic blocks with various dependencies,
 * control flow, and register pressure to force GCC's instruction scheduler
 * to allocate and later free scheduling context during compilation.
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
int complex_int_computation(int a, int b, int c, int d, int e, int f) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 | f;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v6 = v5 << 2;
    v7 = v6 + v1;      /* Uses v1 again - anti-dependency */
    v1 = v7 * 3;       /* Re-defines v1 - output dependency */
    
    /* More complex dependency chain */
    v8 = v1 & v2;
    v9 = v8 | v3;
    v10 = v9 ^ v4;
    v11 = v10 + v5;
    v12 = v11 - v6;
    v13 = v12 * v7;
    v14 = v13 / v8;
    v15 = v14 | v9;
    
    /* Inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v15));
    
    v16 = v15 + v10;
    v17 = v16 * v11;
    v18 = v17 - v12;
    v19 = v18 / v13;
    v20 = v19 | v14;
    
    /* Control flow to create multiple basic blocks */
    if (v20 > 1000) {
        v21 = v20 * 2;
        v22 = v21 + v15;
        v23 = v22 - v16;
    } else {
        v21 = v20 / 2;
        v22 = v21 - v15;
        v23 = v22 + v16;
    }
    
    /* More operations in both branches converge */
    v24 = v23 * v17;
    v25 = v24 + v18;
    v26 = v25 - v19;
    v27 = v26 / v20;
    v28 = v27 | v21;
    v29 = v28 + v22;
    v30 = v29 * v23;
    
    /* Final result uses many variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float floating_point_processing(float a, float b, float c, float d) {
    /* Local array with many elements to increase register pressure */
    float arr[32];
    float temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    float temp9, temp10, temp11, temp12, temp13, temp14, temp15, temp16;
    
    /* Initialize array with dependencies */
    arr[0] = a;
    arr[1] = b;
    for (int i = 2; i < 32; i++) {
        /* True data dependencies in loop */
        arr[i] = arr[i-1] + arr[i-2] * c;
        
        /* Mix with other operations */
        temp1 = arr[i] * d;
        temp2 = temp1 + arr[i-1];
        temp3 = temp2 - arr[i-2];
        
        /* Inline assembly to prevent reordering */
        if (i % 4 == 0) {
            asm volatile("" : "+r"(temp3));
        }
        
        temp4 = temp3 / (i + 1);
        arr[i] = arr[i] + temp4;
    }
    
    /* Process array with multiple accumulators */
    temp5 = 0.0f; temp6 = 0.0f; temp7 = 0.0f; temp8 = 0.0f;
    temp9 = 0.0f; temp10 = 0.0f; temp11 = 0.0f; temp12 = 0.0f;
    
    for (int i = 0; i < 32; i += 4) {
        temp5 += arr[i] * arr[i];
        temp6 += arr[i+1] * arr[i+1];
        temp7 += arr[i+2] * arr[i+2];
        temp8 += arr[i+3] * arr[i+3];
        
        /* Cross dependencies between accumulators */
        temp9 = temp5 - temp6;
        temp10 = temp7 + temp8;
        temp11 = temp9 * temp10;
        temp12 = temp11 / (i + 1);
    }
    
    /* Complex final computation */
    temp13 = sqrtf(fabsf(temp5));
    temp14 = sqrtf(fabsf(temp6));
    temp15 = sqrtf(fabsf(temp7));
    temp16 = sqrtf(fabsf(temp8));
    
    return temp13 + temp14 + temp15 + temp16 + temp9 + temp10 + temp11 + temp12;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int a, float b, double c, int d) {
    /* Declare many variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Initial computations with type conversions */
    i1 = a * d;
    f1 = b * i1;
    d1 = c + f1;
    
    i2 = i1 + 100;
    f2 = f1 - 50.0f;
    d2 = d1 * 2.0;
    
    /* Complex control flow */
    if (i2 > f2) {
        i3 = i2 * 2;
        f3 = f2 / 2.0f;
        d3 = d2 + 1.0;
        
        i4 = i3 | i1;
        f4 = f3 * f1;
        d4 = d3 - d1;
    } else if (i2 < f2) {
        i3 = i2 / 2;
        f3 = f2 * 2.0f;
        d3 = d2 - 1.0;
        
        i4 = i3 & i1;
        f4 = f3 / f1;
        d4 = d3 + d1;
    } else {
        i3 = i2 + 1;
        f3 = f2 - 1.0f;
        d3 = d2 * 0.5;
        
        i4 = i3 ^ i1;
        f4 = f3 + f1;
        d4 = d3 / d1;
    }
    
    /* More operations after control flow */
    i5 = i3 << i4;
    f5 = f3 * f4;
    d5 = d3 / d4;
    
    i6 = i5 >> 1;
    f6 = f5 + f4;
    d6 = d5 * d4;
    
    /* Inline assembly to create dependencies */
    asm volatile("" : "+r"(i6), "+r"(f6), "+r"(d6));
    
    i7 = i6 * i4;
    f7 = f6 - f3;
    d7 = d6 + d3;
    
    i8 = i7 & i5;
    f8 = f7 / f5;
    d8 = d7 - d5;
    
    i9 = i8 | i6;
    f9 = f8 * f6;
    d9 = d8 / d6;
    
    i10 = i9 ^ i7;
    f10 = f9 + f7;
    d10 = d9 * d7;
    
    /* Final result with all types */
    return (double)i10 + (double)f10 + d10;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_based_computation(int mode, long x, long y, long z) {
    long result = 0;
    long t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Initialize temporaries */
    t1 = x + y;
    t2 = y * z;
    t3 = z - x;
    t4 = t1 | t2;
    t5 = t2 & t3;
    
    switch (mode % 5) {
        case 0:
            /* Arithmetic intensive block */
            t6 = t1 * t2;
            t7 = t3 + t4;
            t8 = t5 - t6;
            t9 = t7 / 2;
            t10 = t8 << 3;
            t11 = t9 >> 1;
            t12 = t10 ^ t11;
            result = t6 + t7 + t8 + t9 + t10 + t11 + t12;
            break;
            
        case 1:
            /* Bit manipulation block */
            t6 = t1 ^ t2;
            t7 = t3 | t4;
            t8 = t5 & t6;
            t9 = t7 << 2;
            t10 = t8 >> 1;
            t11 = t9 ^ t10;
            t12 = t11 | t1;
            t13 = t12 & t2;
            result = t6 | t7 | t8 | t9 | t10 | t11 | t12 | t13;
            break;
            
        case 2:
            /* Mixed operations block */
            t6 = t1 + 100;
            t7 = t2 - 50;
            t8 = t3 * 2;
            t9 = t4 / 3;
            t10 = t5 % 7;
            t11 = t6 & t7;
            t12 = t8 | t9;
            t13 = t10 ^ t11;
            t14 = t12 + t13;
            result = t6 + t7 + t8 + t9 + t10 + t11 + t12 + t13 + t14;
            break;
            
        case 3:
            /* Dependency chain block */
            t6 = t1 + t2;
            t7 = t6 * t3;
            t8 = t7 - t4;
            t9 = t8 / t5;
            t10 = t9 + t6;
            t11 = t10 * t7;
            t12 = t11 - t8;
            t13 = t12 / t9;
            t14 = t13 + t10;
            t15 = t14 * t11;
            result = t6 + t7 + t8 + t9 + t10 + t11 + t12 + t13 + t14 + t15;
            break;
            
        case 4:
            /* All operations block */
            t6 = t1 * t2 + t3;
            t7 = t4 - t5 / t1;
            t8 = (t2 | t3) & t4;
            t9 = t5 ^ t6 << 1;
            t10 = t7 >> 2 | t8;
            t11 = t9 + t10 * 3;
            t12 = t11 - t6 / 4;
            t13 = t7 & t8 | t9;
            t14 = t10 ^ t11 << 3;
            t15 = t12 + t13 - t14;
            t16 = t15 * t6 / t7;
            result = t6 + t7 + t8 + t9 + t10 + t11 + t12 + t13 + t14 + t15 + t16;
            break;
    }
    
    /* Post-switch operations */
    t17 = result * 2;
    t18 = t17 + x;
    t19 = t18 - y;
    t20 = t19 / (z + 1);
    
    /* Inline assembly to prevent optimization */
    asm volatile("" : "+r"(t20));
    
    return result + t17 + t18 + t19 + t20;
}

/* Main function that calls all complex functions */
int main(int argc, char *argv[]) {
    /* Use volatile and command line to prevent constant propagation */
    volatile int input1 = g_input1;
    volatile int input2 = g_input2;
    volatile float input3 = g_input3;
    volatile float input4 = g_input4;
    
    /* Read from command line if available */
    if (argc > 1) {
        input1 = atoi(argv[1]);
    }
    if (argc > 2) {
        input2 = atoi(argv[2]);
    }
    if (argc > 3) {
        input3 = atof(argv[3]);
    }
    if (argc > 4) {
        input4 = atof(argv[4]);
    }
    
    /* Call all complex functions */
    int result1 = complex_int_computation(input1, input2, 
                                         input1 + 1, input2 - 1,
                                         input1 * 2, input2 / 2);
    
    float result2 = floating_point_processing(input3, input4,
                                             input3 * 2.0f, input4 / 2.0f);
    
    double result3 = mixed_operations(input1, input3,
                                     (double)input2, input2);
    
    long result4 = switch_based_computation(input1, 
                                           (long)input1 * 100,
                                           (long)input2 * 50,
                                           (long)(input1 + input2));
    
    /* Aggregate results to a volatile sink to prevent dead code elimination */
    volatile double final_result = 0.0;
    final_result += (double)result1;
    final_result += (double)result2;
    final_result += result3;
    final_result += (double)result4;
    
    /* Print checksum to ensure all computations are used */
    printf("Checksum: %f\n", final_result);
    
    return 0;
}
