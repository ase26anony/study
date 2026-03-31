/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-optimize-sibling-calls -fdump-rtl-all caller-save-test.c external-effects.c -o caller-save-test */

#include <stdio.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect(int, double);
extern void another_effect(float, long);
extern void mixed_effect(int, float, double, long);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: High pressure with integer and floating point mix */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Unpack into many scalar variables - all must be kept in registers */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] ^ ints[8];
    
    double d1 = doubles[0] * 1.5;
    double d2 = doubles[1] + doubles[2];
    double d3 = doubles[3] / doubles[4];
    double d4 = sin(doubles[5]);
    double d5 = cos(doubles[6]);
    
    float f1 = (float)doubles[7] * 2.0f;
    float f2 = (float)doubles[8] + 1.0f;
    float f3 = (float)doubles[9] - 0.5f;
    
    long l1 = (long)ints[9] * 1000L;
    long l2 = (long)ints[10] + 5000L;
    
    /* Use all variables in computation before call */
    int temp1 = v1 + v2 - v3;
    double temp2 = d1 * d2 + d3;
    float temp3 = f1 * f2 - f3;
    long temp4 = l1 ^ l2;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call - clobbers call-used registers */
    unknown_effect(temp1, temp2);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all variables again after call - they must be restored */
    v1 = v1 * 3 + temp1;
    v2 = v2 / 2 + v4;
    v3 = v3 ^ v5;
    
    d1 = d1 + d4 * 2.0;
    d2 = d2 - d5 / 3.0;
    d3 = d3 * sin(d2);
    
    f1 = f1 + temp3;
    f2 = f2 * 2.0f - f3;
    
    l1 = l1 + temp4;
    l2 = l2 * 2 - l1;
    
    /* Complex computation mixing all types */
    double result = (double)v1 + (double)v2 * 0.5 + d1 + d2 * 2.0 + 
                    (double)f1 + (double)f2 * 0.25 + (double)l1 * 0.001;
    
    return (int)result + v3 + (int)l2;
}

/* Function 2: Different pattern with nested control flow */
double high_pressure_call_2(int *ints, float *floats, int selector) {
    /* Even more variables */
    int a1 = ints[0], a2 = ints[1], a3 = ints[2], a4 = ints[3], a5 = ints[4];
    int b1 = ints[5], b2 = ints[6], b3 = ints[7], b4 = ints[8], b5 = ints[9];
    
    float f1 = floats[0], f2 = floats[1], f3 = floats[2], f4 = floats[3];
    float f5 = floats[4], f6 = floats[5], f7 = floats[6], f8 = floats[7];
    
    double d1 = (double)f1 * 1.1;
    double d2 = (double)f2 * 2.2;
    double d3 = (double)f3 * 3.3;
    
    long l1 = (long)a1 * a2;
    long l2 = (long)a3 * a4;
    long l3 = (long)a5 * b1;
    
    /* Complex pre-call computation */
    for (int i = 0; i < 3; i++) {
        a1 = a1 + a2 * i;
        a3 = a3 ^ a4;
        f1 = f1 * 1.1f + f2;
        f3 = f3 - f4 * 0.5f;
        d1 = d1 + sin(d2);
        
        /* Conditional call inside loop */
        if (i % 2 == 0) {
            asm volatile("" : : : "memory");
            another_effect(f1, l1);
            asm volatile("" : : : "memory");
        }
        
        /* More computations */
        a2 = a2 + b2;
        a4 = a4 ^ b3;
        f2 = f2 * 2.0f - f5;
        d2 = d2 * cos(d3);
        l1 = l1 + l2;
    }
    
    /* Switch statement with calls at multiple points */
    switch (selector % 4) {
        case 0:
            asm volatile("" : : : "memory");
            mixed_effect(a1, f1, d1, l1);
            asm volatile("" : : : "memory");
            a1 = a1 * 2 + b4;
            break;
        case 1:
            asm volatile("" : : : "memory");
            unknown_effect(a2, d2);
            asm volatile("" : : : "memory");
            a2 = a2 ^ b5;
            f2 = f2 + f6;
            break;
        case 2:
            asm volatile("" : : : "memory");
            another_effect(f3, l2);
            asm volatile("" : : : "memory");
            a3 = a3 + a4 * 3;
            d3 = d3 * 1.5;
            break;
        default:
            asm volatile("" : : : "memory");
            mixed_effect(a4, f4, d3, l3);
            asm volatile("" : : : "memory");
            a4 = a4 | b1;
            f4 = f4 * f7;
    }
    
    /* Final computation using all variables */
    double result = (double)a1 + (double)a2 * 0.3 + (double)a3 * 0.7 +
                    (double)a4 + (double)a5 * 1.2 +
                    (double)f1 + (double)f2 * 2.0 + (double)f3 * 3.0 +
                    (double)f4 + (double)f5 * 0.5 + (double)f6 * 0.25 +
                    d1 + d2 * 0.8 + d3 * 1.1 +
                    (double)l1 * 0.001 + (double)l2 * 0.0005 + (double)l3 * 0.0001;
    
    return result;
}

/* Function 3: Loop with multiple call sites */
void high_pressure_loop(int iterations, int *data, double *results) {
    for (int i = 0; i < iterations; i++) {
        /* Create many live values */
        int x1 = data[i * 10 + 0];
        int x2 = data[i * 10 + 1];
        int x3 = data[i * 10 + 2];
        int x4 = data[i * 10 + 3];
        int x5 = data[i * 10 + 4];
        
        double y1 = (double)data[i * 10 + 5] * 0.1;
        double y2 = (double)data[i * 10 + 6] * 0.2;
        double y3 = (double)data[i * 10 + 7] * 0.3;
        
        float z1 = (float)y1 * 1.5f;
        float z2 = (float)y2 * 2.5f;
        float z3 = (float)y3 * 3.5f;
        
        long l1 = (long)x1 * x2;
        long l2 = (long)x3 * x4;
        
        /* First computation block */
        x1 = x1 * 2 + i;
        x2 = x2 ^ x3;
        y1 = y1 + sin(y2);
        z1 = z1 * z2 - z3;
        
        /* First call - will clobber registers */
        asm volatile("" : : : "memory");
        unknown_effect(x1, y1);
        asm volatile("" : : : "memory");
        
        /* Restore and continue computation */
        x3 = x3 + x4 * x5;
        x4 = x4 | x1;
        y2 = y2 * cos(y3);
        z2 = z2 + z1 / 2.0f;
        l1 = l1 + l2 * i;
        
        /* Second call with different function */
        asm volatile("" : : : "memory");
        another_effect(z2, l1);
        asm volatile("" : : : "memory");
        
        /* More computation */
        x5 = x5 * 3 - x2;
        y3 = y3 + tan(y1);
        z3 = z3 * 1.1f + z2;
        l2 = l2 ^ l1;
        
        /* Third call */
        asm volatile("" : : : "memory");
        mixed_effect(x5, z3, y3, l2);
        asm volatile("" : : : "memory");
        
        /* Final result */
        results[i] = (double)x1 + (double)x2 * 0.5 + (double)x3 * 0.3 +
                     (double)x4 + (double)x5 * 1.5 +
                     y1 + y2 * 2.0 + y3 * 0.7 +
                     (double)z1 + (double)z2 * 0.4 + (double)z3 * 0.6 +
                     (double)l1 * 0.0001 + (double)l2 * 0.00005;
    }
}

int main() {
    /* Initialize test data */
    int int_data[100];
    double double_data[100];
    float float_data[100];
    double results[100];
    
    for (int i = 0; i < 100; i++) {
        int_data[i] = i * 3 + 7;
        double_data[i] = (double)i * 1.7 + 2.3;
        float_data[i] = (float)i * 0.7f + 1.3f;
    }
    
    int total = 0;
    double total_double = 0.0;
    
    /* Call high-pressure functions in loops */
    for (int iter = 0; iter < 50; iter++) {
        /* Vary the selector to exercise different paths */
        int selector = iter % 10;
        
        /* Call first function */
        int result1 = high_pressure_call_1(int_data, double_data);
        total += result1;
        
        /* Call second function */
        double result2 = high_pressure_call_2(int_data, float_data, selector);
        total_double += result2;
        
        /* Update global to prevent dead code elimination */
        global_counter += result1;
    }
    
    /* Call loop-based function */
    high_pressure_loop(30, int_data, results);
    
    /* Use results to prevent optimization */
    for (int i = 0; i < 30; i++) {
        total += (int)results[i];
        total_double += results[i];
    }
    
    printf("Total: %d, Total double: %f\n", total, total_double);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
