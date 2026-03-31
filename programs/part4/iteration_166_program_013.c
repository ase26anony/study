/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fdump-rtl-reload caller_save_test.c external_effects.c */

#include <stdio.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect(int, double);
extern void another_effect(float, long);
extern void mixed_effect(int, float, double, long);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: High pressure with integer and floating point values */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Unpack many variables to create register pressure */
    int v1 = ints[0] + 1;
    int v2 = ints[1] * 2;
    int v3 = ints[2] - ints[0];
    int v4 = ints[3] ^ ints[1];
    int v5 = ints[4] | ints[2];
    double d1 = doubles[0] * 1.5;
    double d2 = doubles[1] / 2.0;
    double d3 = doubles[2] + doubles[0];
    double d4 = doubles[3] - doubles[1];
    float f1 = (float)doubles[0] * 2.5f;
    float f2 = (float)doubles[1] + 3.14f;
    
    /* Use all variables in computation before call */
    int sum_int = v1 + v2 + v3 + v4 + v5;
    double sum_double = d1 + d2 + d3 + d4;
    float sum_float = f1 + f2;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    unknown_effect(sum_int, sum_double);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Use variables again after call - they must be restored */
    v1 = v1 * 2 + sum_int;
    v2 = v2 / 2 ^ v1;
    d1 = d1 * 3.0 + sum_double;
    d2 = d2 / 1.5 - d1;
    f1 = f1 * 2.0f + sum_float;
    
    /* More computations mixing types */
    double mixed = (double)v1 + d1 + (double)f1;
    int result = (int)mixed + v2 + (int)d2;
    
    return result;
}

/* Function 2: Different pattern with more variables */
double high_pressure_call_2(float *floats, long *longs) {
    /* Even more variables to increase pressure */
    float f1 = floats[0];
    float f2 = floats[1];
    float f3 = floats[2];
    float f4 = floats[3];
    float f5 = floats[4];
    float f6 = floats[5];
    
    long l1 = longs[0];
    long l2 = longs[1];
    long l3 = longs[2];
    long l4 = longs[3];
    long l5 = longs[4];
    
    /* Complex pre-call computations */
    float fsum = f1 * f2 + f3 / f4 - f5 * f6;
    long lsum = (l1 ^ l2) | (l3 & l4) + l5;
    
    /* Use in intermediate calculations */
    double d1 = (double)f1 * (double)l1;
    double d2 = (double)f2 / (double)l2;
    double d3 = (double)f3 + (double)l3;
    double d4 = (double)f4 - (double)l4;
    
    asm volatile("" : : : "memory");
    
    /* Call with mixed arguments */
    mixed_effect((int)fsum, fsum, (double)lsum, lsum);
    
    asm volatile("" : : : "memory");
    
    /* Post-call computations requiring all values */
    f1 = f1 + fsum;
    f2 = f2 - fsum;
    f3 = f3 * 2.0f;
    f4 = f4 / 2.0f;
    l1 = l1 + lsum;
    l2 = l2 - lsum;
    
    d1 = d1 + (double)f1;
    d2 = d2 - (double)f2;
    d3 = d3 * (double)f3;
    d4 = d4 / (double)f4;
    
    return d1 + d2 + d3 + d4 + (double)l1 + (double)l2;
}

/* Function 3: Nested control flow with calls */
int high_pressure_conditional(int *data, double *coeffs, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Create many live values inside loop */
        int a = data[i];
        int b = data[i + 1];
        int c = data[i + 2];
        double x = coeffs[i];
        double y = coeffs[i + 1];
        double z = coeffs[i + 2];
        float fa = (float)a * 0.5f;
        float fb = (float)b * 1.5f;
        
        /* Conditional with calls inside */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
            unknown_effect(a, x);
            asm volatile("" : : : "memory");
            
            a = a * 2;
            x = x * 2.0;
        } else if (i % 3 == 1) {
            asm volatile("" : : : "memory");
            another_effect(fa, (long)b);
            asm volatile("" : : : "memory");
            
            fa = fa * 3.0f;
            b = b + 1;
        } else {
            asm volatile("" : : : "memory");
            mixed_effect(c, fb, y, (long)c);
            asm volatile("" : : : "memory");
            
            c = c ^ 0xFF;
            y = y / 2.0;
        }
        
        /* Use all variables after conditional calls */
        result += (int)((double)a + x + (double)b + y + (double)c + z + (double)fa + (double)fb);
        
        /* Update global to prevent loop elimination */
        global_counter++;
    }
    
    return result;
}

int main() {
    /* Initialize test data */
    int int_data[100];
    double double_data[100];
    float float_data[100];
    long long_data[100];
    
    for (int i = 0; i < 100; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 1.5 + 0.5;
        float_data[i] = i * 0.7f + 0.3f;
        long_data[i] = i * 5L + 2L;
    }
    
    int total = 0;
    double total_d = 0.0;
    
    /* Loop with calls to trigger caller-save insertion */
    for (int iter = 0; iter < 100; iter++) {
        /* Modify data slightly each iteration */
        int_data[iter % 50] += iter;
        double_data[iter % 50] += (double)iter * 0.1;
        
        /* Call high-pressure functions */
        total += high_pressure_call_1(int_data, double_data);
        total_d += high_pressure_call_2(float_data, long_data);
        
        /* Call conditional version */
        total += high_pressure_conditional(int_data, double_data, 10);
    }
    
    printf("Result: %d, %.2f\n", total, total_d);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
