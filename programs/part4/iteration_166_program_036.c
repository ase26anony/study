/* Test program to trigger caller-save insertion during reload pass */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fdump-rtl-reload -fdump-rtl-all caller-save-test.c external.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect(int id, double val);
extern void another_effect(float f1, float f2, float f3);
extern int side_effect_func(long a, long b, long c);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: High pressure with mixed integer types */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Create many local variables that must be kept in registers */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + v1;
    int v3 = ints[2] - v2;
    int v4 = ints[3] ^ v3;
    int v5 = ints[4] | v4;
    int v6 = ints[5] & v5;
    int v7 = ints[6] << 2;
    int v8 = ints[7] >> 1;
    int v9 = ints[8] + v7;
    int v10 = ints[9] - v8;
    
    /* Floating point variables */
    double d1 = doubles[0] * 1.5;
    double d2 = doubles[1] / 2.0;
    double d3 = doubles[2] + d1;
    double d4 = doubles[3] - d2;
    double d5 = doubles[4] * d3;
    double d6 = doubles[5] / d4;
    double d7 = doubles[6] + 3.14159;
    double d8 = doubles[7] - 2.71828;
    
    /* Mix computations to create dependencies */
    v1 = v1 + (int)d1;
    v2 = v2 * (int)d2;
    d3 = d3 + v3;
    d4 = d4 * v4;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call - forces caller-save for all live registers */
    unknown_effect(v1 + v2, d3 * d4);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all variables after call - they must be restored */
    v5 = v5 + v6 + v7 + v8;
    v9 = v9 ^ v10 ^ v1;
    d5 = d5 + d6 + d7 + d8;
    d6 = d6 * d3 * d4;
    
    /* More computations mixing types */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);
    
    /* Another call with different arguments */
    unknown_effect(result, d5 - d6);
    
    return result;
}

/* Function 2: Different pattern with loops inside */
double high_pressure_call_2(float *floats, long *longs) {
    /* Many float variables */
    float f1 = floats[0];
    float f2 = floats[1] * 2.0f;
    float f3 = floats[2] / 3.0f;
    float f4 = floats[3] + f1;
    float f5 = floats[4] - f2;
    float f6 = floats[5] * f3;
    float f7 = floats[6] / f4;
    float f8 = floats[7] + 1.0f;
    float f9 = floats[8] - 0.5f;
    float f10 = floats[9] * 1.1f;
    
    /* Many long variables */
    long l1 = longs[0];
    long l2 = longs[1] << 2;
    long l3 = longs[2] >> 1;
    long l4 = longs[3] + l1;
    long l5 = longs[4] - l2;
    long l6 = longs[5] | l3;
    long l7 = longs[6] & l4;
    long l8 = longs[7] ^ l5;
    long l9 = longs[8] * 3;
    long l10 = longs[9] / 2;
    
    /* Complex computation before call */
    for (int i = 0; i < 3; i++) {
        f1 = f1 * 1.1f + f2;
        f2 = f2 / 1.2f - f3;
        l1 = l1 + (long)f1;
        l2 = l2 - (long)f2;
    }
    
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - uses multiple calling convention registers */
    another_effect(f1, f2, f3);
    
    asm volatile("" : : : "memory");
    
    /* Use variables after call */
    f4 = f4 + f5 + f6;
    f7 = f7 * f8 * f9;
    l3 = l3 + l4 + l5;
    l6 = l6 * l7 / 8;
    
    /* Nested call in conditional */
    if (l1 > l2) {
        asm volatile("" : : : "memory");
        int temp = side_effect_func(l1, l2, l3);
        asm volatile("" : : : "memory");
        f10 = f10 * temp;
    } else {
        asm volatile("" : : : "memory");
        int temp = side_effect_func(l4, l5, l6);
        asm volatile("" : : : "memory");
        f10 = f10 / temp;
    }
    
    /* Final computation using all variables */
    double result = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
    result += (double)(l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10);
    
    return result;
}

/* Function 3: Complex control flow with multiple call sites */
int complex_control_flow(int *data, int n) {
    int sum = 0;
    
    /* Loop with call inside - will trigger caller-save insertion */
    for (int i = 0; i < n; i++) {
        /* Create many live values across the loop */
        int a = data[i] * 2;
        int b = data[i] + i;
        int c = data[i] ^ 0x55;
        int d = data[i] << 3;
        int e = data[i] >> 1;
        
        double x = sin(data[i] * 0.01);
        double y = cos(data[i] * 0.02);
        double z = x * y + 1.0;
        
        /* Use variables in computation */
        a = a + b + c;
        d = d ^ e ^ a;
        x = x * 2.0 + z;
        y = y / 2.0 - z;
        
        /* Conditional with call */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
            unknown_effect(a, x);
            asm volatile("" : : : "memory");
            
            /* More computations */
            b = b + (int)x;
            c = c * (int)y;
        } else if (i % 3 == 1) {
            asm volatile("" : : : "memory");
            unknown_effect(d, y);
            asm volatile("" : : : "memory");
            
            /* Different computations */
            e = e - (int)x;
            a = a | (int)y;
        } else {
            asm volatile("" : : : "memory");
            unknown_effect(e, z);
            asm volatile("" : : : "memory");
            
            /* Yet more computations */
            d = d & (int)(x * 100);
            b = b ^ (int)(y * 100);
        }
        
        /* Use all variables after conditional calls */
        sum += a + b + c + d + e;
        sum += (int)(x + y + z);
        
        /* Update global to prevent dead code elimination */
        global_counter += i;
    }
    
    return sum;
}

/* Main function with loop calling high-pressure functions */
int main() {
    const int N = 100;
    int *int_data = malloc(N * sizeof(int));
    double *double_data = malloc(N * sizeof(double));
    float *float_data = malloc(N * sizeof(float));
    long *long_data = malloc(N * sizeof(long));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        int_data[i] = (i * 37) % 1000;
        double_data[i] = (i * 0.12345);
        float_data[i] = (i * 0.6789f);
        long_data[i] = (i * 12345L);
    }
    
    int total = 0;
    double total_double = 0.0;
    
    /* Loop calling high-pressure functions multiple times */
    for (int iter = 0; iter < 50; iter++) {
        /* Call first high-pressure function */
        int result1 = high_pressure_call_1(int_data + iter, double_data + iter);
        total += result1;
        
        /* Call second high-pressure function */
        double result2 = high_pressure_call_2(float_data + iter, long_data + iter);
        total_double += result2;
        
        /* Call complex control flow function */
        int result3 = complex_control_flow(int_data, (iter % 10) + 5);
        total += result3;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Total: %d, Double total: %f\n", total, total_double);
    printf("Global counter: %d\n", global_counter);
    
    free(int_data);
    free(double_data);
    free(float_data);
    free(long_data);
    
    return 0;
}
