/* Test program to trigger caller-save insertion during reload */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* External function with unknown side effects - prevents inlining */
extern void unknown_effect(int a, double b) __attribute__((noinline));
extern void another_effect(float f, long l) __attribute__((noinline));
extern void mixed_effect(int i1, int i2, double d1, double d2) __attribute__((noinline));

/* Function 1: High pressure with integer and floating point values */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Unpack many variables to create register pressure */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + v1;
    int v3 = ints[2] - v2;
    int v4 = ints[3] * v3;
    int v5 = ints[4] / (v4 ? v4 : 1);
    int v6 = ints[5] ^ v5;
    int v7 = ints[6] | v6;
    int v8 = ints[7] & v7;
    int v9 = ints[8] << 2;
    int v10 = ints[9] >> 1;
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] + d1;
    double d3 = doubles[2] - d2;
    double d4 = doubles[3] * d3;
    double d5 = doubles[4] / (d4 + 1.0);
    double d6 = doubles[5] * sin(d5);
    double d7 = doubles[6] + cos(d6);
    double d8 = doubles[7] * exp(d7);
    double d9 = doubles[8] / (d8 + 1.0);
    double d10 = doubles[9] * log(fabs(d9) + 1.0);
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Call external function - all variables must be preserved */
    unknown_effect(v1, d1);
    
    /* More computations using the live values */
    v1 = v2 * v3 + v4;
    v2 = v5 - v6 * v7;
    v3 = v8 ^ v9 | v10;
    v4 = (v1 + v2) * (v3 - v4);
    
    d1 = d2 * d3 - d4;
    d2 = d5 / (d6 + d7);
    d3 = sin(d8) * cos(d9);
    d4 = d10 * exp(d1);
    
    /* Another call with different arguments */
    asm volatile("" : : : "memory");
    another_effect((float)d2, (long)v4);
    
    /* Final computations */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    double dresult = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    
    return result + (int)dresult;
}

/* Function 2: Different pattern with nested control flow */
double high_pressure_call_2(int *ints, double *doubles, int selector) {
    /* Create many live values */
    int a1 = ints[0];
    int a2 = ints[1];
    int a3 = ints[2];
    int a4 = ints[3];
    int a5 = ints[4];
    int a6 = ints[5];
    int a7 = ints[6];
    int a8 = ints[7];
    
    double b1 = doubles[0];
    double b2 = doubles[1];
    double b3 = doubles[2];
    double b4 = doubles[3];
    double b5 = doubles[4];
    double b6 = doubles[5];
    double b7 = doubles[6];
    double b8 = doubles[7];
    
    /* Complex control flow with calls inside */
    double result = 0.0;
    
    for (int i = 0; i < 10; i++) {
        /* Modify values in loop */
        a1 += i;
        a2 -= i * 2;
        a3 *= (i + 1);
        a4 /= (i ? i : 1);
        
        b1 += sin(i * 0.1);
        b2 *= cos(i * 0.2);
        b3 = b4 * exp(b5);
        b4 = b6 / (b7 + 1.0);
        
        /* Call inside loop - forces caller-save around loop */
        asm volatile("" : : : "memory");
        if (i % 3 == 0) {
            mixed_effect(a1, a2, b1, b2);
        } else if (i % 3 == 1) {
            unknown_effect(a3, b3);
        } else {
            another_effect((float)b4, (long)a4);
        }
        asm volatile("" : : : "memory");
        
        /* Use values after call */
        result += a1 * b1 + a2 * b2 + a3 * b3 + a4 * b4;
        
        /* Switch statement with calls */
        switch (selector % 4) {
            case 0:
                a5 = a6 * a7;
                b5 = b6 * b7;
                unknown_effect(a5, b5);
                break;
            case 1:
                a6 = a7 + a8;
                b6 = b7 - b8;
                another_effect((float)b6, (long)a6);
                break;
            case 2:
                a7 = a8 ^ a1;
                b7 = b8 * sin(b1);
                mixed_effect(a7, a8, b7, b8);
                break;
            default:
                a8 = a1 & a2;
                b8 = b1 + cos(b2);
                unknown_effect(a8, b8);
                break;
        }
    }
    
    return result;
}

/* Function 3: Deeply nested conditionals */
int high_pressure_call_3(int *ints, double *doubles, int depth) {
    /* Even more variables */
    int x1 = ints[0], x2 = ints[1], x3 = ints[2], x4 = ints[3];
    int x5 = ints[4], x6 = ints[5], x7 = ints[6], x8 = ints[7];
    int x9 = ints[8], x10 = ints[9], x11 = ints[10], x12 = ints[11];
    
    double y1 = doubles[0], y2 = doubles[1], y3 = doubles[2], y4 = doubles[3];
    double y5 = doubles[4], y6 = doubles[5], y7 = doubles[6], y8 = doubles[7];
    double y9 = doubles[8], y10 = doubles[9], y11 = doubles[10], y12 = doubles[11];
    
    /* Recursive-like conditional structure */
    int total = 0;
    
    if (depth > 0) {
        /* First computation block */
        x1 = x2 * x3 + x4;
        x2 = x5 - x6 * x7;
        y1 = y2 * y3 - y4;
        y2 = y5 / (y6 + y7);
        
        asm volatile("" : : : "memory");
        unknown_effect(x1, y1);
        asm volatile("" : : : "memory");
        
        if (depth > 1) {
            x3 = x8 ^ x9 | x10;
            x4 = (x11 + x12) * (x3 - x4);
            y3 = sin(y8) * cos(y9);
            y4 = y10 * exp(y11);
            
            asm volatile("" : : : "memory");
            another_effect((float)y2, (long)x2);
            asm volatile("" : : : "memory");
            
            if (depth > 2) {
                x5 = x1 + x2 * x3;
                x6 = x4 / (x5 ? x5 : 1);
                y5 = y1 + y2 * y3;
                y6 = y4 / (fabs(y5) + 1.0);
                
                asm volatile("" : : : "memory");
                mixed_effect(x5, x6, y5, y6);
                asm volatile("" : : : "memory");
            }
        }
        
        /* Use all variables in final computation */
        total = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 + x11 + x12;
        double dtotal = y1 + y2 + y3 + y4 + y5 + y6 + y7 + y8 + y9 + y10 + y11 + y12;
        total += (int)dtotal;
    }
    
    return total;
}

int main() {
    /* Initialize test data */
    int int_data[20];
    double double_data[20];
    
    for (int i = 0; i < 20; i++) {
        int_data[i] = rand() % 100 + 1;
        double_data[i] = (rand() % 1000) / 100.0 + 0.1;
    }
    
    int total = 0;
    double dtotal = 0.0;
    
    /* Loop with multiple high-pressure calls */
    for (int i = 0; i < 100; i++) {
        /* Modify data slightly each iteration */
        int_data[i % 20] += i;
        double_data[i % 20] *= 1.01;
        
        /* Call different high-pressure functions */
        asm volatile("" : : : "memory");
        total += high_pressure_call_1(int_data, double_data);
        
        asm volatile("" : : : "memory");
        dtotal += high_pressure_call_2(int_data, double_data, i);
        
        asm volatile("" : : : "memory");
        total += high_pressure_call_3(int_data, double_data, i % 4);
    }
    
    printf("Result: %d, %f\n", total, dtotal);
    return 0;
}
