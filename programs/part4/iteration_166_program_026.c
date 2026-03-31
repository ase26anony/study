#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect1(int, double);
extern void unknown_effect2(float, long);
extern void unknown_effect3(double, double, int);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Non-inlinable external functions */
void __attribute__((noinline)) unknown_effect1(int a, double b) {
    global_counter += a + (int)b;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) unknown_effect2(float a, long b) {
    global_counter += (int)a + (int)(b % 100);
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) unknown_effect3(double a, double b, int c) {
    global_counter += (int)(a + b) * c;
    asm volatile("" : : : "memory");
}

/* High pressure function with many live values across call */
int __attribute__((noinline)) high_pressure_function1(int base) {
    /* Declare many variables of mixed types */
    int v1 = base + 1;
    int v2 = base + 2;
    int v3 = base + 3;
    int v4 = base + 4;
    int v5 = base + 5;
    double d1 = base * 1.1;
    double d2 = base * 2.2;
    double d3 = base * 3.3;
    double d4 = base * 4.4;
    float f1 = base * 0.5f;
    float f2 = base * 1.5f;
    float f3 = base * 2.5f;
    long l1 = base * 10L;
    long l2 = base * 20L;
    
    /* Complex computation using all variables */
    v1 = v1 * 2 + v2 / 3;
    v3 = v3 ^ v4 | v5;
    d1 = d1 * d2 + sin(d3);
    d4 = d4 / d1 * cos(d2);
    f1 = f1 + f2 * f3;
    f2 = f2 - f1 / f3;
    l1 = l1 << 2 | l2 >> 1;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Function call with many live values */
    unknown_effect1(v1 + v2, d1 * d2);
    
    /* More computations after call - all variables still live */
    v4 = v4 * v5 + v3;
    v5 = v5 ^ v1 & v2;
    d3 = d3 * 2.0 + d4;
    d2 = d2 / 3.0 - d1;
    f3 = f3 * f1 - f2;
    l2 = l2 + l1 * 2;
    
    /* Another call with different arguments */
    unknown_effect2(f1 + f3, l1 + l2);
    
    /* Final computation using all variables */
    int result = v1 + v2 + v3 + v4 + v5 + 
                 (int)d1 + (int)d2 + (int)d3 + (int)d4 +
                 (int)f1 + (int)f2 + (int)f3 +
                 (int)l1 + (int)l2;
    
    /* Force use of frame pointer */
    asm volatile("" : : : "rbp");
    
    return result % 1000;
}

/* Second high pressure function with different pattern */
double __attribute__((noinline)) high_pressure_function2(double base) {
    /* Even more variables to increase pressure */
    double a1 = base * 1.0;
    double a2 = base * 2.0;
    double a3 = base * 3.0;
    double a4 = base * 4.0;
    double a5 = base * 5.0;
    double a6 = base * 6.0;
    double a7 = base * 7.0;
    double a8 = base * 8.0;
    int b1 = (int)base + 1;
    int b2 = (int)base + 2;
    int b3 = (int)base + 3;
    int b4 = (int)base + 4;
    int b5 = (int)base + 5;
    int b6 = (int)base + 6;
    float c1 = (float)base * 0.1f;
    float c2 = (float)base * 0.2f;
    float c3 = (float)base * 0.3f;
    
    /* Nested control flow with calls */
    for (int i = 0; i < 10; i++) {
        /* Complex computation before call */
        a1 = a1 * a2 + sin(a3);
        a4 = a4 / a5 * cos(a6);
        a7 = a7 + a8 * tan(a1);
        b1 = b1 * b2 + b3;
        b4 = b4 ^ b5 | b6;
        c1 = c1 + c2 * c3;
        
        /* Conditional call */
        if (i % 3 == 0) {
            unknown_effect3(a1, a2, b1);
        } else if (i % 3 == 1) {
            unknown_effect1(b2 + b3, a3 * a4);
        } else {
            unknown_effect2(c1 * 2.0f, (long)(a5 * 100));
        }
        
        /* Computation after call */
        a2 = a2 + a1 * 0.5;
        a5 = a5 - a4 / 2.0;
        a8 = a8 * 1.1 + a7;
        b3 = b3 << 1 | b4;
        b6 = b6 + b5 * 2;
        c2 = c2 - c1 / 3.0f;
        c3 = c3 * 1.5f + c2;
    }
    
    /* Switch statement with calls */
    int selector = ((int)base) % 4;
    switch (selector) {
        case 0:
            unknown_effect1(b1 + b2, a1 + a2);
            a3 = a3 * 2.0;
            break;
        case 1:
            unknown_effect2(c1, (long)a4);
            a5 = a5 / 3.0;
            break;
        case 2:
            unknown_effect3(a6, a7, b3);
            a8 = a8 + 1.0;
            break;
        default:
            unknown_effect1(b4, a1 * a8);
            break;
    }
    
    /* Final result using all variables */
    double result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 +
                    b1 + b2 + b3 + b4 + b5 + b6 +
                    c1 + c2 + c3;
    
    return result;
}

/* Third function with different register pressure pattern */
float __attribute__((noinline)) high_pressure_function3(int iterations) {
    /* Mixed computations in loop with calls */
    float accum = 0.0f;
    double daccum = 0.0;
    int iaccum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many temporary values */
        float f1 = i * 0.1f;
        float f2 = i * 0.2f;
        float f3 = i * 0.3f;
        double d1 = i * 1.1;
        double d2 = i * 2.2;
        double d3 = i * 3.3;
        int i1 = i * 10;
        int i2 = i * 20;
        int i3 = i * 30;
        int i4 = i * 40;
        int i5 = i * 50;
        
        /* Use all temporaries in computation */
        f1 = f1 * f2 + f3;
        f2 = f2 - f1 / f3;
        d1 = d1 * sin(d2) + cos(d3);
        d2 = d2 / d1 * tan(d3);
        i1 = i1 * i2 + i3;
        i2 = i2 ^ i4 | i5;
        i3 = i3 << 2 | i4 >> 1;
        
        /* Call with many live values */
        if (i % 2 == 0) {
            unknown_effect1(i1 + i2, d1 + d2);
        } else {
            unknown_effect2(f1 + f2, (long)(d3 * 100));
        }
        
        /* More computations after call */
        f3 = f3 * 1.5f - f1;
        d3 = d3 * 2.0 + d1;
        i4 = i4 + i5 * i3;
        i5 = i5 ^ i1 & i2;
        
        /* Accumulate results */
        accum += f1 + f2 + f3;
        daccum += d1 + d2 + d3;
        iaccum += i1 + i2 + i3 + i4 + i5;
    }
    
    /* Final call */
    unknown_effect3(daccum, accum * 2.0, iaccum % 1000);
    
    return accum + (float)daccum + (float)iaccum;
}

int main() {
    int total = 0;
    
    /* Loop to ensure caller-save insertion happens multiple times */
    for (int i = 0; i < 100; i++) {
        /* Call different high-pressure functions */
        int r1 = high_pressure_function1(i);
        double r2 = high_pressure_function2(i * 1.0);
        float r3 = high_pressure_function3(5 + (i % 10));
        
        total += r1 + (int)r2 + (int)r3;
        
        /* Prevent loop optimization */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d (global: %d)\n", total, global_counter);
    return total != 0 ? 0 : 1;
}
