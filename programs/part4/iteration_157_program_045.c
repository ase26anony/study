/* test_sched_context.c - Complex program to trigger scheduler context allocation and cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function 1: Integer-heavy computation with many serial dependencies */
int int_heavy_computation(int a, int b, int c, int d, int e) {
    /* Create many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long chain of true data dependencies (RAW) */
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
    
    /* Anti-dependencies (WAR) */
    v11 = v10;
    v10 = v4 + v5;  /* v10 written after being read */
    v12 = v11;
    v11 = v6 * v7;  /* v11 written after being read */
    
    /* Output dependencies (WAW) */
    v13 = v8 + v9;
    v13 = v10 * v11;  /* v13 written twice */
    
    /* More complex dependencies with control flow */
    if (v13 > 1000) {
        v14 = v13 / 2;
        v15 = v14 + v1;
        v16 = v15 * 3;
    } else {
        v14 = v13 * 2;
        v15 = v14 - v1;
        v16 = v15 / 3;
    }
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v16));
    
    v17 = v16 + v2;
    v18 = v17 - v3;
    v19 = v18 * v4;
    v20 = v19 / v5;
    
    /* Another control flow with dependencies */
    switch (v20 & 3) {
        case 0:
            v21 = v20 + v6;
            v22 = v21 * v7;
            break;
        case 1:
            v21 = v20 - v6;
            v22 = v21 / v7;
            break;
        case 2:
            v21 = v20 ^ v6;
            v22 = v21 | v7;
            break;
        default:
            v21 = v20 & v6;
            v22 = v21 ^ v7;
    }
    
    /* Use remaining variables to prevent optimization */
    v23 = v22 + v8;
    v24 = v23 - v9;
    v25 = v24 * v10;
    v26 = v25 / v11;
    v27 = v26 << 1;
    v28 = v27 >> 2;
    v29 = v28 | v12;
    v30 = v29 & v13;
    
    /* Final computation using all variables */
    return v14 + v15 + v16 + v17 + v18 + v19 + v20 + 
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + 
           v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float fp_array_processing(float* arr, int size) {
    float result = 0.0f;
    float temp1, temp2, temp3, temp4, temp5;
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f, acc4 = 0.0f;
    
    /* Loop with data dependencies across iterations */
    for (int i = 2; i < size; i++) {
        /* Memory accesses with address calculations */
        temp1 = arr[i-2];
        temp2 = arr[i-1];
        temp3 = arr[i];
        
        /* Floating-point operations with dependencies */
        temp4 = temp1 * 1.5f;
        temp5 = temp2 + 2.0f;
        
        /* Mix of operations */
        arr[i] = temp4 + temp5 * temp3;
        
        /* Accumulate results */
        acc1 += arr[i];
        acc2 += temp4;
        acc3 += temp5;
        acc4 += temp1 * temp2;
        
        /* Inline assembly to prevent reordering */
        if (i % 4 == 0) {
            asm volatile("" : "+r"(i), "+m"(arr[i]));
        }
    }
    
    /* Complex floating-point computation */
    result = acc1 * sinf(acc2) + acc3 * cosf(acc4);
    
    /* More operations to increase block size */
    float t1 = result * 2.0f;
    float t2 = t1 / 3.14159f;
    float t3 = t2 + acc1;
    float t4 = t3 - acc2;
    float t5 = t4 * acc3;
    float t6 = t5 / acc4;
    
    return t6;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int x, double y, char z) {
    /* Declare many variables of different types */
    int i1 = x, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    double d1 = y, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    char c1 = z, c2, c3, c4, c5;
    
    /* Type conversions and mixed operations */
    d2 = (double)i1 * d1;
    i2 = (int)d2 + c1;
    d3 = sin(d2) * cos(d1);
    i3 = i2 * 7;
    
    /* Complex control flow */
    if (c1 > 'm') {
        d4 = d2 * 2.5;
        i4 = i3 << 2;
        c2 = c1 + 1;
    } else if (c1 > 'g') {
        d4 = d2 / 2.5;
        i4 = i3 >> 2;
        c2 = c1 - 1;
    } else {
        d4 = d2 + 2.5;
        i4 = i3 & 0xFF;
        c2 = c1 * 2;
    }
    
    /* Loop with dependencies */
    for (int j = 0; j < 8; j++) {
        d5 = d4 + j;
        i5 = i4 ^ j;
        c3 = c2 + j;
        
        /* Nested if */
        if (j % 2 == 0) {
            d6 = d5 * 1.1;
            i6 = i5 + 100;
        } else {
            d6 = d5 / 1.1;
            i6 = i5 - 100;
        }
        
        d7 += d6;
        i7 += i6;
    }
    
    /* More operations using all variables */
    d8 = d7 * d3;
    i8 = i7 | i3;
    c4 = c3 ^ c2;
    
    d9 = exp(d8);
    i9 = i8 * i4;
    c5 = c4 & 0x7F;
    
    d10 = log(fabs(d9) + 1.0);
    i10 = i9 % 256;
    
    /* Final result mixing all types */
    return d10 + (double)i10 + (double)c5;
}

/* Function 4: Switch statement with different operation blocks */
long switch_based_computation(int mode, long base) {
    long result = base;
    long a, b, c, d, e, f, g, h, i, j;
    
    /* Large switch creating multiple basic blocks */
    switch (mode % 5) {
        case 0:
            /* Block with arithmetic dependencies */
            a = result + 100;
            b = a * 2;
            c = b - 50;
            d = c / 3;
            e = d << 1;
            f = e | 0xAAAA;
            g = f & 0x5555;
            h = g ^ result;
            i = h + a;
            j = i - b;
            result = j * 2;
            break;
            
        case 1:
            /* Block with bit operations */
            a = result ^ 0xFFFFFFFF;
            b = a >> 4;
            c = b << 8;
            d = c | 0xFF00;
            e = d & 0x00FF;
            f = ~e;
            g = f + result;
            h = g * 3;
            i = h / 5;
            j = i % 7;
            result = j;
            break;
            
        case 2:
            /* Block with mixed operations */
            a = result * 3;
            b = a + 17;
            c = b - 29;
            d = c / 2;
            e = d * d;
            f = sqrt(e);
            g = f + 1;
            h = g * g;
            i = h - e;
            j = i / 2;
            result = j;
            break;
            
        case 3:
            /* Block with memory-style operations */
            a = result;
            for (int k = 0; k < 10; k++) {
                b = a + k;
                c = b * k;
                d = c - a;
                a = d;
            }
            result = a;
            break;
            
        default:
            /* Block with complex dependencies */
            a = result;
            b = a + 1;
            c = b * 2;
            d = c - 3;
            e = d / 4;
            f = e + 5;
            g = f * 6;
            h = g - 7;
            i = h / 8;
            j = i + 9;
            result = j * 10;
    }
    
    /* Post-switch operations */
    long k = result + 1000;
    long l = k * 2;
    long m = l - 500;
    long n = m / 2;
    long o = n | 0x1234;
    long p = o & 0x5678;
    long q = p ^ result;
    
    return q;
}

/* Main function to drive execution */
int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int input1, input2, input3, input4, input5;
    volatile float finput;
    volatile double dinput;
    volatile char cinput;
    
    /* Initialize with non-constant values */
    if (argc > 1) {
        input1 = atoi(argv[1]);
        input2 = atoi(argv[argc > 2 ? 2 : 1]);
        input3 = atoi(argv[argc > 3 ? 3 : 1]);
        input4 = atoi(argv[argc > 4 ? 4 : 1]);
        input5 = atoi(argv[argc > 5 ? 5 : 1]);
        finput = (float)atof(argv[argc > 6 ? 6 : "1.5"]);
        dinput = atof(argv[argc > 7 ? 7 : "2.718"]);
        cinput = argv[argc > 8 ? 8 : "a"][0];
    } else {
        /* Default values if no args */
        input1 = 42;
        input2 = 17;
        input3 = 99;
        input4 = 123;
        input5 = 7;
        finput = 3.14159f;
        dinput = 2.71828;
        cinput = 'x';
    }
    
    /* Prepare array for FP function */
    float arr[50];
    for (int i = 0; i < 50; i++) {
        arr[i] = (float)i * 0.5f;
    }
    
    /* Call all complex functions */
    int result1 = int_heavy_computation(input1, input2, input3, input4, input5);
    float result2 = fp_array_processing(arr, 50);
    double result3 = mixed_operations(input1, dinput, cinput);
    long result4 = switch_based_computation(input1, input2);
    
    /* Aggregate results to a volatile sink */
    volatile long final_result = 0;
    final_result += result1;
    final_result += (long)result2;
    final_result += (long)result3;
    final_result += result4;
    
    /* Print to prevent dead code elimination */
    printf("Result: %ld\n", final_result);
    
    return 0;
}
