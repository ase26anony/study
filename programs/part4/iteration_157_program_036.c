/* test_scheduler_context.c
 * 
 * This program creates complex basic blocks with various dependencies,
 * control flow, and register pressure to force GCC's instruction scheduler
 * to allocate and later free scheduling context data structures.
 * 
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-sched test_scheduler_context.c -o test
 * Or with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
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
    
    /* Long chain of true data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 << 2;
    v6 = v5 | 0xFF;
    v7 = v6 & 0x0F;
    v8 = v7 ^ v1;
    v9 = v8 % 17;
    v10 = v9 + v2;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10 * 3;      /* Write v11 */
    v12 = v11 + v3;     /* Read v3, Write v12 */
    v3 = v12 / 2;       /* Write v3 (WAW with earlier v3) */
    v13 = v3 - v11;     /* Read v11 (WAR with v11) */
    v11 = v13 * 4;      /* Write v11 (WAW) */
    
    /* More operations to create scheduling complexity */
    v14 = (v4 + v5) * (v6 - v7);
    v15 = v14 >> (v8 & 3);
    v16 = v15 ^ v9;
    v17 = v16 | v10;
    v18 = v17 & v11;
    v19 = v18 + v12;
    v20 = v19 - v13;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 * 2;
    v22 = v21 / 3;
    v23 = v22 + v14;
    v24 = v23 - v15;
    v25 = v24 * v16;
    v26 = v25 | v17;
    v27 = v26 & v18;
    v28 = v27 ^ v19;
    v29 = v28 + v20;
    v30 = v29 - v21;
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* Branch with its own dependencies */
        v30 = v30 * 2 + v22 - v23;
        asm volatile("" : "+r"(v30));
    } else {
        v30 = v30 / 2 + v24 * v25;
        asm volatile("" : "+r"(v30));
    }
    
    /* Use all variables to prevent dead code elimination */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float float_array_processing(float* arr, int size) {
    float result = 0.0f;
    float temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f, acc4 = 0.0f;
    
    /* Loop with dependencies across iterations */
    for (int i = 2; i < size - 2; i++) {
        /* Memory accesses with address calculations */
        temp1 = arr[i] * 1.5f;
        temp2 = arr[i-1] + arr[i-2];
        temp3 = arr[i+1] - arr[i+2];
        
        /* Floating-point operations with dependencies */
        temp4 = temp1 * temp2;
        temp5 = temp4 / temp3;
        temp6 = sqrtf(fabsf(temp5));
        temp7 = temp6 + sinf(temp4);
        temp8 = temp7 * cosf(temp5);
        
        /* Accumulate with anti-dependencies */
        acc1 = acc1 + temp8;      /* WAR on acc1 */
        acc2 = acc2 - temp7;      /* WAR on acc2 */
        acc3 = acc3 * 1.01f + temp6;
        acc4 = acc4 / 1.02f - temp5;
        
        /* Inline assembly to prevent reordering */
        if (i % 4 == 0) {
            asm volatile("" : "+r"(acc1), "+r"(acc2));
        }
    }
    
    /* Complex basic block after loop */
    result = acc1 * acc2 + acc3 / acc4;
    
    /* More floating-point operations */
    float f1 = result * 2.0f;
    float f2 = f1 / 3.14159f;
    float f3 = f2 + sinf(result);
    float f4 = f3 * cosf(f1);
    float f5 = f4 - tanf(f2);
    float f6 = f5 * expf(f3);
    float f7 = f6 / logf(fabsf(f4) + 1.0f);
    float f8 = f7 + f5 * f6;
    
    asm volatile("" : "+r"(f8));
    
    return result + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int x, double y, char* data) {
    /* Declare many variables of different types */
    int i1 = x, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    double d1 = y, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5;
    long l1, l2, l3, l4, l5;
    
    /* Complex control flow */
    if (x > 0) {
        /* Block A with integer-heavy operations */
        i2 = i1 * 2 + 1;
        i3 = i2 / 3 - 4;
        i4 = i3 << (i1 & 3);
        i5 = i4 | 0xABCD;
        i6 = i5 & 0x1234;
        i7 = i6 ^ i2;
        i8 = i7 % 19;
        i9 = i8 + i3;
        i10 = i9 - i4;
        
        d2 = d1 * 2.5;
        d3 = d2 / 1.7;
        d4 = sin(d3) * cos(d2);
        d5 = d4 + tan(d1);
        
        asm volatile("" : "+r"(i10), "+r"(d5));
    } else {
        /* Block B with different operations */
        i2 = i1 / 2 - 1;
        i3 = i2 * 3 + 4;
        i4 = i3 >> (i1 & 1);
        i5 = i4 & 0xDCBA;
        i6 = i5 | 0x4321;
        i7 = i6 ^ i2;
        i8 = i7 % 23;
        i9 = i8 - i3;
        i10 = i9 + i4;
        
        d2 = d1 / 3.5;
        d3 = d2 * 1.9;
        d4 = cos(d3) + sin(d2);
        d5 = d4 - atan(d1);
        
        asm volatile("" : "+r"(i10), "+r"(d5));
    }
    
    /* Common code with mixed-type operations */
    f1 = (float)d5;
    f2 = f1 * 2.0f;
    f3 = f2 + (float)i10;
    f4 = f3 / 1.414f;
    f5 = sqrtf(fabsf(f4));
    
    l1 = (long)i10 * 1000L;
    l2 = l1 + (long)(d5 * 1000.0);
    l3 = l2 ^ 0xFFFFFFFF;
    l4 = l3 << 2;
    l5 = l4 >> 1;
    
    /* Memory access pattern */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        if (data) {
            sum += data[i] * f5;
        }
    }
    
    d6 = sum + d5;
    d7 = d6 * f1;
    d8 = d7 / (f2 + 1.0);
    d9 = pow(d8, 2.0);
    d10 = d9 + f3 * f4;
    
    /* Use all variables */
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 +
           f1 + f2 + f3 + f4 + f5 + l1 + l2 + l3 + l4 + l5;
}

/* Function 4: Switch statement with different operation blocks per case */
long switch_operations(int mode, long base) {
    long result = base;
    long a, b, c, d, e, f, g, h, i, j;
    long k, l, m, n, o, p, q, r, s, t;
    
    switch (mode % 5) {
        case 0:
            /* Case 0: Arithmetic operations */
            a = result + 100;
            b = a * 3;
            c = b - 50;
            d = c / 2;
            e = d << 3;
            f = e | 0xFF00;
            g = f & 0x0F0F;
            h = g ^ a;
            i = h % 17;
            j = i + b;
            asm volatile("" : "+r"(j));
            result = j;
            break;
            
        case 1:
            /* Case 1: Bit manipulation */
            a = result ^ 0xAAAAAAAA;
            b = a << 1;
            c = b >> 2;
            d = c | 0x55555555;
            e = d & 0x33333333;
            f = e ^ c;
            g = f * 2;
            h = g + d;
            i = h - e;
            j = i % 255;
            asm volatile("" : "+r"(j));
            result = j;
            break;
            
        case 2:
            /* Case 2: Mixed shifts and arithmetic */
            a = result * 7;
            b = a >> (result & 3);
            c = b + 12345;
            d = c - 54321;
            e = d << 2;
            f = e / 3;
            g = f | 0x1234;
            h = g & 0x5678;
            i = h ^ result;
            j = i % 97;
            asm volatile("" : "+r"(j));
            result = j;
            break;
            
        case 3:
            /* Case 3: Long dependency chain */
            a = result + 1;
            b = a * a;
            c = b + a;
            d = c * c;
            e = d + b;
            f = e * e;
            g = f + c;
            h = g * g;
            i = h + d;
            j = i * i;
            asm volatile("" : "+r"(j));
            result = j % 1000000;
            break;
            
        case 4:
            /* Case 4: Complex expression */
            a = (result << 4) | (result >> 28);
            b = a * 0x9E3779B9;
            c = b ^ (b >> 16);
            d = c * 0x85EBCA6B;
            e = d ^ (d >> 13);
            f = e * 0xC2B2AE35;
            g = f ^ (f >> 16);
            h = g + result;
            i = h * 0x5F356495;
            j = i ^ (i >> 15);
            asm volatile("" : "+r"(j));
            result = j;
            break;
    }
    
    /* Post-switch operations with all variables */
    k = result * 2;
    l = k + a;
    m = l - b;
    n = m * c;
    o = n / (d + 1);
    p = o | e;
    q = p & f;
    r = q ^ g;
    s = r + h;
    t = s - i;
    
    /* Final complex expression */
    return result + a + b + c + d + e + f + g + h + i + j +
           k + l + m + n + o + p + q + r + s + t;
}

/* Main function to ensure all code is executed */
int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int input1 = 42;
    volatile int input2 = 17;
    volatile int input3 = 99;
    volatile int input4 = 123;
    
    /* Read from command line to get dynamic values */
    if (argc > 1) {
        input1 = atoi(argv[1]);
    }
    if (argc > 2) {
        input2 = atoi(argv[2]);
    }
    if (argc > 3) {
        input3 = atoi(argv[3]);
    }
    if (argc > 4) {
        input4 = atoi(argv[4]);
    }
    
    /* Prepare data for float array function */
    float float_array[100];
    for (int i = 0; i < 100; i++) {
        float_array[i] = (float)(i * 1.1 + input1 * 0.5);
    }
    
    char char_data[16];
    for (int i = 0; i < 16; i++) {
        char_data[i] = (char)(i + input2);
    }
    
    /* Call all functions and accumulate results */
    int result1 = integer_computation(input1, input2, input3, input4, 5);
    float result2 = float_array_processing(float_array, 100);
    double result3 = mixed_operations(input1, (double)input2 * 1.234, char_data);
    long result4 = switch_operations(input3, input4 * 1000L);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int final_result = 0;
    final_result += result1;
    final_result += (int)result2;
    final_result += (int)result3;
    final_result += (int)result4;
    
    /* Print checksum to ensure all computations are used */
    printf("Checksum: %d\n", final_result);
    
    return 0;
}
