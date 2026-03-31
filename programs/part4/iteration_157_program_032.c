/* test_sched_context.c - Complex program to trigger scheduler context allocation and cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FORCE_USE(x) asm volatile("" : "+r"(x))

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_int_chain(int seed) {
    volatile int v1 = seed * 2;
    volatile int v2 = seed + 7;
    volatile int v3 = seed - 3;
    volatile int v4 = seed / 2;
    volatile int v5 = seed % 17;
    volatile int v6 = seed ^ 0x55AA;
    volatile int v7 = seed | 0xFF00;
    volatile int v8 = seed & 0x0F0F;
    volatile int v9 = seed << 3;
    volatile int v10 = seed >> 2;
    
    /* Create long dependency chain */
    int a = v1 + v2;          FORCE_USE(a);
    int b = a * v3 - v4;      FORCE_USE(b);
    int c = b / (v5 + 1);     FORCE_USE(c);
    int d = c ^ v6;           FORCE_USE(d);
    int e = d | v7;           FORCE_USE(e);
    int f = e & v8;           FORCE_USE(f);
    int g = f << (v9 & 3);    FORCE_USE(g);
    int h = g >> (v10 & 3);   FORCE_USE(h);
    int i = h * v1 / v2;      FORCE_USE(i);
    int j = i + v3 - v4;      FORCE_USE(j);
    int k = j ^ v5 | v6;      FORCE_USE(k);
    int l = k & v7 << 1;      FORCE_USE(l);
    int m = l + v8 - v9;      FORCE_USE(m);
    int n = m * v10 / 2;      FORCE_USE(n);
    int o = n % 256;          FORCE_USE(o);
    
    /* Control flow to create multiple basic blocks */
    if (o > 128) {
        int p = o * 3 + 7;    FORCE_USE(p);
        int q = p / 2 - 1;    FORCE_USE(q);
        return q;
    } else if (o > 64) {
        int r = o << 2;       FORCE_USE(r);
        int s = r | 0x0F;     FORCE_USE(s);
        return s;
    } else {
        int t = o + v1 + v2;  FORCE_USE(t);
        int u = t ^ v3;       FORCE_USE(u);
        return u;
    }
}

/* Function 2: Floating-point array processing with loops */
float floating_point_processing(int seed, int iterations) {
    volatile float f1 = seed * 0.1f;
    volatile float f2 = seed * 0.2f;
    volatile float f3 = seed * 0.3f;
    volatile float f4 = seed * 0.4f;
    volatile float f5 = seed * 0.5f;
    volatile float f6 = seed * 0.6f;
    volatile float f7 = seed * 0.7f;
    volatile float f8 = seed * 0.8f;
    volatile float f9 = seed * 0.9f;
    volatile float f10 = seed * 1.0f;
    
    float arr[20];
    float result = 0.0f;
    
    /* Initialize array with complex dependencies */
    for (int i = 0; i < 20; i++) {
        if (i == 0) {
            arr[i] = f1 + f2;
        } else if (i == 1) {
            arr[i] = f3 * f4 - f5;
        } else {
            arr[i] = arr[i-1] * arr[i-2] + f6;
        }
        FORCE_USE(arr[i]);
    }
    
    /* Process with loop-carried dependencies */
    for (int iter = 0; iter < iterations; iter++) {
        float temp = 0.0f;
        for (int i = 0; i < 19; i++) {
            if (i % 3 == 0) {
                arr[i] = arr[i] * f7 + f8;
            } else if (i % 3 == 1) {
                arr[i] = arr[i] / f9 - f10;
            } else {
                arr[i] = sqrtf(fabsf(arr[i] + arr[i+1]));
            }
            temp += arr[i];
            FORCE_USE(temp);
        }
        result = result * 0.9f + temp * 0.1f;
        FORCE_USE(result);
    }
    
    return result;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int seed) {
    /* Many local variables to create register pressure */
    volatile double d1 = seed * 1.1;
    volatile double d2 = seed * 2.2;
    volatile double d3 = seed * 3.3;
    volatile double d4 = seed * 4.4;
    volatile double d5 = seed * 5.5;
    volatile int i1 = seed + 1;
    volatile int i2 = seed + 2;
    volatile int i3 = seed + 3;
    volatile int i4 = seed + 4;
    volatile int i5 = seed + 5;
    volatile long l1 = seed * 10;
    volatile long l2 = seed * 20;
    volatile long l3 = seed * 30;
    volatile float f1 = seed * 0.01f;
    volatile float f2 = seed * 0.02f;
    volatile float f3 = seed * 0.03f;
    volatile char c1 = seed & 0xFF;
    volatile char c2 = (seed >> 8) & 0xFF;
    volatile short s1 = seed & 0xFFFF;
    volatile short s2 = (seed >> 16) & 0xFFFF;
    
    /* Complex mixed-type computations */
    double result = d1;
    FORCE_USE(result);
    
    if (i1 > 0) {
        result = result * d2 + i1 * d3;
        int tmp_i = i2 * i3 - i4;
        result += tmp_i * 0.5;
        FORCE_USE(tmp_i);
    } else {
        result = result / d4 - i5 * 0.25;
        long tmp_l = l1 | l2 & l3;
        result -= tmp_l * 0.01;
        FORCE_USE(tmp_l);
    }
    
    switch (seed % 4) {
        case 0:
            result = result + f1 * f2;
            result = result * (c1 + 1) / 256.0;
            break;
        case 1:
            result = result - f3 * 0.5;
            result = result * (c2 + 1) / 256.0;
            break;
        case 2:
            result = result * (s1 + 1) / 65536.0;
            result = result + d5 * 0.1;
            break;
        case 3:
            result = result / (s2 + 1) * 65536.0;
            result = result - d1 * 0.2;
            break;
    }
    
    /* More operations to extend basic block */
    for (int i = 0; i < 3; i++) {
        result = result * 1.1 - 0.1;
        result = result + i * 0.01;
        FORCE_USE(result);
    }
    
    return result;
}

/* Function 4: Switch statement with different operation blocks per case */
int switch_complex(int seed) {
    volatile int a = seed * 3;
    volatile int b = seed + 11;
    volatile int c = seed - 7;
    volatile int d = seed ^ 0x1234;
    volatile int e = seed | 0xABCD;
    volatile int f = seed & 0x3333;
    volatile int g = seed << 2;
    volatile int h = seed >> 1;
    
    int result = 0;
    
    switch (seed % 5) {
        case 0: {
            /* Block with arithmetic chain */
            int t1 = a + b;   FORCE_USE(t1);
            int t2 = t1 * c;  FORCE_USE(t2);
            int t3 = t2 - d;  FORCE_USE(t3);
            int t4 = t3 / 2;  FORCE_USE(t4);
            int t5 = t4 ^ e;  FORCE_USE(t5);
            result = t5;
            break;
        }
        case 1: {
            /* Block with bit operations */
            int t1 = b & c;   FORCE_USE(t1);
            int t2 = t1 | d;  FORCE_USE(t2);
            int t3 = t2 ^ e;  FORCE_USE(t3);
            int t4 = t3 << 3; FORCE_USE(t4);
            int t5 = t4 >> 1; FORCE_USE(t5);
            result = t5;
            break;
        }
        case 2: {
            /* Block with mixed operations */
            int t1 = c * d;   FORCE_USE(t1);
            int t2 = t1 + e;  FORCE_USE(t2);
            int t3 = t2 & f;  FORCE_USE(t3);
            int t4 = t3 | g;  FORCE_USE(t4);
            int t5 = t4 ^ h;  FORCE_USE(t5);
            result = t5;
            break;
        }
        case 3: {
            /* Block with control flow inside */
            int t1 = d + e;
            if (t1 > 1000) {
                t1 = t1 * 2 - f;
            } else {
                t1 = t1 / 2 + g;
            }
            int t2 = t1 & h;
            result = t2;
            FORCE_USE(t1); FORCE_USE(t2);
            break;
        }
        case 4: {
            /* Block with small loop */
            int t1 = e;
            for (int i = 0; i < 4; i++) {
                t1 = t1 * 2 + f + i;
            }
            result = t1;
            FORCE_USE(t1);
            break;
        }
    }
    
    /* Post-switch operations */
    if (result > 0) {
        result = result * 3 / 2;
    } else {
        result = result + 1000;
    }
    
    return result;
}

/* Main function to ensure all code is executed */
int main(int argc, char *argv[]) {
    /* Use command line or stdin for dynamic inputs */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        printf("Enter a seed value: ");
        scanf("%d", &seed);
    }
    
    volatile int result1 = 0;
    volatile float result2 = 0.0f;
    volatile double result3 = 0.0;
    volatile int result4 = 0;
    
    /* Call all complex functions */
    result1 = complex_int_chain(seed);
    printf("Result1: %d\n", result1);
    
    result2 = floating_point_processing(seed, 5);
    printf("Result2: %f\n", result2);
    
    result3 = mixed_operations(seed);
    printf("Result3: %lf\n", result3);
    
    result4 = switch_complex(seed);
    printf("Result4: %d\n", result4);
    
    /* Aggregate results to prevent elimination */
    volatile int final = result1 + (int)result2 + (int)result3 + result4;
    printf("Final checksum: %d\n", final);
    
    return 0;
}
