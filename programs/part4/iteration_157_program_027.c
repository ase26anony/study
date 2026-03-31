/* test_scheduler_context.c
 * Complex program to trigger GCC scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test_scheduler
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
    
    /* Long dependency chain with RAW dependencies */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 << 2;
    v6 = v5 ^ v1;
    v7 = v6 & 0xFF;
    v8 = v7 | v2;
    v9 = v8 >> 1;
    v10 = v9 + v3;
    
    /* Anti-dependencies (WAR) */
    v11 = v10 * 2;
    v10 = v11 - 5;  /* WAR: v10 written after v11 reads old v10 */
    
    /* Output dependencies (WAW) */
    v12 = v11 + 7;
    v12 = v12 * 3;  /* WAW: v12 written twice */
    
    /* More complex dependencies */
    v13 = v12 % 17;
    v14 = v13 + v4;
    v15 = v14 * v5;
    v16 = v15 - v6;
    v17 = v16 / v7;
    v18 = v17 | v8;
    v19 = v18 ^ v9;
    v20 = v19 & v10;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 * 11;
    v22 = v21 + 13;
    v23 = v22 - 17;
    v24 = v23 / 19;
    v25 = v24 << 3;
    v26 = v25 ^ 0x55;
    v27 = v26 & 0xAA;
    v28 = v27 | 0x33;
    v29 = v28 >> 2;
    v30 = v29 + v20;
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        v30 = v30 * 2;
        v29 = v29 + v30;
    } else {
        v30 = v30 / 2;
        v29 = v29 - v30;
    }
    
    /* Final computation using many variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float func2_float_array(float* arr, int n) {
    float sum = 0.0f;
    float prod = 1.0f;
    float diff = 0.0f;
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    
    /* Loop with dependencies inside */
    for (int i = 2; i < n; i++) {
        /* RAW dependencies in loop */
        t1 = arr[i-2] * 1.5f;
        t2 = arr[i-1] + 2.5f;
        t3 = t1 + t2;
        t4 = t3 * 0.7f;
        t5 = t4 - arr[i];
        
        /* Anti-dependencies */
        t6 = t5 / 3.0f;
        t5 = t6 * 2.0f;  /* WAR */
        
        /* Complex floating operations */
        t7 = sinf(t5);
        t8 = cosf(t6);
        t9 = t7 * t8;
        t10 = t9 + t4;
        
        /* Update array with dependency */
        arr[i] = t10;
        
        /* Accumulate results */
        sum += arr[i];
        prod *= (arr[i] + 1.0f);
        diff -= (arr[i] - arr[i-1]);
        
        /* Inline assembly to prevent optimization */
        asm volatile("" : "+r"(i), "+m"(arr[i]));
    }
    
    /* Mix integer and float operations */
    int count = n;
    float result = sum;
    while (count-- > 0) {
        result = result * 1.01f;
        result = result - 0.5f;
    }
    
    return sum + prod + diff + result;
}

/* Function 3: Mixed operations with control flow and many locals */
double func3_mixed_control(int mode, double x, double y) {
    /* Many local variables of different types */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5;
    
    /* Initial computations */
    d1 = x * y;
    d2 = x + y;
    d3 = x - y;
    d4 = x / (y + 1.0);
    
    i1 = (int)x;
    i2 = (int)y;
    i3 = i1 * i2;
    i4 = i1 + i2;
    i5 = i1 - i2;
    
    f1 = (float)d1;
    f2 = (float)d2;
    f3 = f1 * f2;
    f4 = f1 + f2;
    f5 = f1 - f2;
    
    /* Complex control flow with dependencies */
    switch (mode % 5) {
        case 0:
            d5 = d1 * d2;
            d6 = d3 + d4;
            d7 = sqrt(d5);
            d8 = pow(d6, 2.0);
            i6 = i3 << 2;
            i7 = i4 ^ i5;
            break;
        case 1:
            d5 = sin(d1) + cos(d2);
            d6 = exp(d3) * log(fabs(d4) + 1.0);
            d7 = d5 * d6;
            d8 = d7 / (d1 + 1.0);
            i6 = i3 & i4;
            i7 = i5 | i6;
            break;
        case 2:
            d5 = d1 * 2.0;
            d6 = d2 / 3.0;
            d7 = d3 + d4;
            d8 = d5 - d6;
            i6 = i3 % 17;
            i7 = i4 * i6;
            break;
        case 3:
            d5 = d1 + d2 + d3 + d4;
            d6 = d5 * 0.5;
            d7 = d6 * d6;
            d8 = sqrt(d7);
            i6 = i3 + i4 + i5;
            i7 = i6 * 2;
            break;
        default:
            d5 = d1;
            d6 = d2;
            d7 = d3;
            d8 = d4;
            i6 = i3;
            i7 = i4;
            break;
    }
    
    /* More operations after switch */
    d9 = d5 + d6 + d7 + d8;
    d10 = d9 * (i6 + i7);
    
    i8 = i6 * i7;
    i9 = i8 + (int)d9;
    i10 = i9 ^ (int)d10;
    
    f1 = (float)d9;
    f2 = (float)d10;
    f3 = f1 * f2;
    f4 = f1 / f2;
    f5 = f3 + f4;
    
    /* Final dependency chain */
    double result = d10;
    for (int i = 0; i < 5; i++) {
        result = result * 1.1;
        result = result - 0.1;
        asm volatile("" : "+r"(result));
    }
    
    return result + i10 + f5;
}

/* Function 4: Switch statement with different operation blocks */
long func4_switch_blocks(int selector, long base) {
    long a = base, b = base + 1, c = base + 2, d = base + 3;
    long e, f, g, h, i, j, k, l, m, n;
    long o, p, q, r, s, t, u, v, w, x;
    
    /* Large switch with different computation patterns */
    switch (selector % 8) {
        case 0:
            e = a * b;
            f = c + d;
            g = e << 3;
            h = f >> 2;
            i = g ^ h;
            j = i & 0xFF;
            k = j | a;
            l = k * b;
            m = l + c;
            n = m - d;
            break;
        case 1:
            e = a + b + c + d;
            f = e * 2;
            g = f / 3;
            h = g % 5;
            i = h << 1;
            j = i >> 2;
            k = j ^ e;
            l = k & f;
            m = l | g;
            n = m * h;
            break;
        case 2:
            e = (a << 1) | (b >> 1);
            f = (c & d) ^ a;
            g = (b | c) & d;
            h = (a ^ b) + c;
            i = (d - a) * b;
            j = (c + d) / 2;
            k = (a % 7) + b;
            l = (c & 0xF0) | (d & 0x0F);
            m = (e << 4) + (f >> 4);
            n = (g * h) - (i / j);
            break;
        case 3:
            e = a * a;
            f = b * b;
            g = c * c;
            h = d * d;
            i = e + f;
            j = g + h;
            k = i * j;
            l = k / (a + 1);
            m = l % (b + 1);
            n = m ^ (c + d);
            break;
        case 4:
            e = ~a;
            f = ~b;
            g = e & f;
            h = c | d;
            i = g ^ h;
            j = i * 3;
            k = j + 5;
            l = k - 7;
            m = l / 11;
            n = m % 13;
            break;
        case 5:
            e = a + (b << 1);
            f = c + (d << 2);
            g = e * f;
            h = g >> 3;
            i = h & 0x3F;
            j = i | 0xC0;
            k = j * a;
            l = k + b;
            m = l - c;
            n = m * d;
            break;
        case 6:
            e = (a & b) | (c & d);
            f = (a | b) & (c | d);
            g = e ^ f;
            h = g + a;
            i = h * b;
            j = i - c;
            k = j / d;
            l = k % e;
            m = l << 2;
            n = m >> 1;
            break;
        default:
            e = a;
            f = b;
            g = c;
            h = d;
            i = e + f;
            j = g + h;
            k = i * j;
            l = k - a;
            m = l + b;
            n = m - c;
            break;
    }
    
    /* Post-switch computations with dependencies */
    o = n * 2;
    p = o + e;
    q = p - f;
    r = q * g;
    s = r / h;
    t = s % i;
    u = t ^ j;
    v = u & k;
    w = v | l;
    x = w * m;
    
    /* Final chain with inline assembly */
    long result = x;
    asm volatile("" : "+r"(result));
    result = result + n;
    asm volatile("" : "+r"(result));
    result = result * 3;
    asm volatile("" : "+r"(result));
    result = result - o;
    
    return result;
}

/* Main function to ensure all code is executed */
int main(int argc, char** argv) {
    /* Use volatile inputs to prevent constant propagation */
    volatile int input1 = 100;
    volatile int input2 = 200;
    volatile int input3 = 300;
    volatile int input4 = 400;
    volatile int input5 = 500;
    
    /* Read from command line if available to make values truly dynamic */
    if (argc > 1) input1 = atoi(argv[1]);
    if (argc > 2) input2 = atoi(argv[2]);
    if (argc > 3) input3 = atoi(argv[3]);
    if (argc > 4) input4 = atoi(argv[4]);
    if (argc > 5) input5 = atoi(argv[5]);
    
    /* Prepare array for float function */
    float arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = (float)(i * 1.5);
    }
    
    /* Call all functions with dynamic inputs */
    int result1 = func1_intensive(input1, input2, input3, input4, input5);
    float result2 = func2_float_array(arr, 100);
    double result3 = func3_mixed_control(input1, (double)input2, (double)input3);
    long result4 = func4_switch_blocks(input4, (long)input5);
    
    /* Aggregate results to a volatile sink to prevent dead code elimination */
    volatile int final_result = 0;
    final_result += result1;
    final_result += (int)result2;
    final_result += (int)result3;
    final_result += (int)result4;
    
    /* Print checksum to ensure all computations are used */
    printf("Checksum: %d\n", final_result);
    
    return 0;
}
