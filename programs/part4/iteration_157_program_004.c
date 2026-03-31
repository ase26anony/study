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
int integer_computation(int a, int b, int c, int d, int e) {
    /* Create many local variables to increase register pressure */
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
    
    /* Anti-dependencies (WAR) */
    v7 = v6 + 1;
    v6 = v7 * 2;  /* WAR: v6 is written after being read */
    
    /* Output dependencies (WAW) */
    v8 = v7 + v6;
    v8 = v8 * 3;   /* WAW: v8 is written twice */
    
    /* More complex dependency chain */
    v9 = v8 >> 1;
    v10 = v9 & 0x0F;
    v11 = v10 ^ v9;
    v12 = v11 + v10;
    v13 = v12 - v11;
    v14 = v13 * v12;
    v15 = v14 / (v13 + 1);
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v15));
    
    v16 = v15 + 100;
    v17 = v16 * 2;
    v18 = v17 - 50;
    v19 = v18 / 3;
    v20 = v19 | 0xAA;
    
    /* Control flow to create multiple basic blocks */
    if (v20 > 1000) {
        v21 = v20 * 2;
        v22 = v21 + 1;
        v23 = v22 - 5;
    } else {
        v21 = v20 / 2;
        v22 = v21 - 1;
        v23 = v22 + 5;
    }
    
    /* More operations in both branches merge here */
    v24 = v23 * v21;
    v25 = v24 + v22;
    v26 = v25 - v23;
    v27 = v26 / (v24 + 1);
    v28 = v27 << 3;
    v29 = v28 | 0x55;
    v30 = v29 ^ 0xFF;
    
    return v30;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float* arr, int size) {
    float result = 0.0f;
    float temp1, temp2, temp3, temp4, temp5;
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f, acc4 = 0.0f;
    
    /* Loop with internal dependencies */
    for (int i = 1; i < size - 1; i++) {
        /* Memory dependencies with address calculations */
        temp1 = arr[i-1];
        temp2 = arr[i];
        temp3 = arr[i+1];
        
        /* Floating-point operations with dependencies */
        temp4 = temp1 * 1.5f;
        temp5 = temp2 + temp3;
        
        /* Mixed operations */
        acc1 += temp4 * temp5;
        acc2 += temp4 - temp5;
        acc3 += temp4 / (temp5 + 0.001f);
        acc4 = acc1 * acc2 - acc3;
        
        /* Anti-dependency */
        temp1 = acc4 + 1.0f;  /* WAR on temp1 */
        
        /* Write to memory with dependency */
        arr[i] = temp1 + acc4;
    }
    
    /* Final computation with many variables */
    result = acc1 + acc2 * 2.0f - acc3 / 3.0f + acc4 * 4.0f;
    
    /* Inline assembly to prevent optimization */
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int x, double y, float z) {
    /* Declare many local variables of different types */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    
    /* Initial assignments */
    i1 = x;
    d1 = y;
    f1 = z;
    
    /* Complex dependency graph across types */
    i2 = i1 * 2;
    d2 = d1 + (double)i2;
    f2 = f1 * (float)d2;
    
    i3 = i2 + (int)f2;
    d3 = d2 - (double)i3;
    f3 = f2 / (float)d3;
    
    i4 = i3 << 1;
    d4 = d3 * 2.0;
    f4 = f3 + 1.0f;
    
    /* Control flow with dependencies carried across */
    if (i4 > 0) {
        i5 = i4 + 100;
        d5 = d4 * 1.5;
        f5 = f4 - 0.5f;
        
        i6 = i5 / 2;
        d6 = d5 + d4;
        f6 = f5 * f4;
    } else {
        i5 = i4 - 100;
        d5 = d4 / 1.5;
        f5 = f4 + 0.5f;
        
        i6 = i5 * 2;
        d6 = d5 - d4;
        f6 = f5 / f4;
    }
    
    /* More operations after merge */
    i7 = i6 | 0x0F;
    d7 = d6 * d5;
    f7 = f6 + f5;
    
    i8 = i7 ^ i6;
    d8 = d7 / (d6 + 0.001);
    f8 = f7 * (f6 + 0.001f);
    
    i9 = i8 << 2;
    d9 = d8 - d7;
    f9 = f8 / f7;
    
    i10 = i9 >> 1;
    d10 = d9 + d8;
    f10 = f9 - f8;
    
    /* Final result using all variables */
    double result = (double)i10 + d10 + (double)f10;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_operations(int mode, long base) {
    long result = base;
    long a, b, c, d, e, f, g, h, i, j;
    
    switch (mode % 4) {
        case 0:
            /* Block with serial dependencies */
            a = result + 1;
            b = a * 2;
            c = b - 3;
            d = c / 4;
            e = d | 0xF0;
            f = e ^ 0x0F;
            g = f << 2;
            h = g >> 1;
            i = h + g;
            j = i * h;
            result = j;
            break;
            
        case 1:
            /* Block with parallel-like dependencies */
            a = result * 3;
            b = result / 2;
            c = a + b;
            d = a - b;
            e = c * d;
            f = c / (d + 1);
            g = e | f;
            h = e & f;
            i = g ^ h;
            j = i << 3;
            result = j;
            break;
            
        case 2:
            /* Block with memory-style operations */
            a = result;
            for (int k = 0; k < 5; k++) {
                b = a + k;
                c = b * 2;
                d = c - 1;
                a = d;
            }
            result = a;
            break;
            
        case 3:
            /* Block with mixed operations */
            a = result;
            b = a + 100;
            c = b * 2;
            d = c - 50;
            e = d / 3;
            f = e | 0xAA;
            g = f ^ 0x55;
            h = g << 1;
            i = h >> 2;
            j = i + h;
            result = j;
            break;
    }
    
    /* Common tail with dependencies on switch result */
    a = result + 10;
    b = a * 3;
    c = b - 5;
    d = c / 2;
    
    /* Inline assembly to create barrier */
    asm volatile("" : "+r"(d));
    
    return d;
}

/* Main function to ensure all code is executed */
int main(int argc, char** argv) {
    int result = 0;
    volatile int input1, input2, input3, input4;
    
    /* Use volatile inputs to prevent constant propagation */
    if (argc > 1) {
        input1 = atoi(argv[1]);
        input2 = atoi(argv[argc > 2 ? 2 : 1]);
        input3 = atoi(argv[argc > 3 ? 3 : 1]);
        input4 = atoi(argv[argc > 4 ? 4 : 1]);
    } else {
        /* Default values if no arguments */
        input1 = 42;
        input2 = 17;
        input3 = 99;
        input4 = 123;
    }
    
    /* Call all functions to ensure they're compiled and executed */
    result += integer_computation(input1, input2, input3, input4, 5);
    
    float arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = (float)(i * input1);
    }
    result += (int)float_array_processing(arr, 20);
    
    result += (int)mixed_operations(input2, (double)input3, (float)input4);
    
    result += (int)switch_operations(input1, input2);
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;
    
    /* Print to ensure execution */
    printf("Result: %d\n", sink);
    
    return 0;
}
