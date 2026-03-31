/* caller-save-test.c
 * Test program to trigger GCC's caller-save insertion logic
 * Specifically targets lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* External functions that cannot be inlined */
extern void unknown_effect_int(int, int, int, int, int, int);
extern void unknown_effect_double(double, double, double, double);
extern void unknown_effect_mixed(int, double, int, double, int, double);
extern int compute_something(int a, double b, int c, double d);

/* Prevent inlining of these functions */
__attribute__((noinline, noipa))
void unknown_effect_int(int a, int b, int c, int d, int e, int f) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f) : "memory");
    global_counter += a + b + c + d + e + f;
}

__attribute__((noinline, noipa))
void unknown_effect_double(double a, double b, double c, double d) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "x"(a), "x"(b), "x"(c), "x"(d) : "memory");
    global_counter += (int)(a + b + c + d);
}

__attribute__((noinline, noipa))
void unknown_effect_mixed(int a, double b, int c, double d, int e, double f) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(a), "x"(b), "r"(c), "x"(d), "r"(e), "x"(f) : "memory");
    global_counter += a + c + e + (int)(b + d + f);
}

__attribute__((noinline, noipa))
int compute_something(int a, double b, int c, double d) {
    int result;
    asm volatile("imull %1, %0\n\t"
                 "cvtsd2si %2, %%eax\n\t"
                 "addl %%eax, %0"
                 : "=r"(result)
                 : "r"(a), "x"(b), "0"(c)
                 : "%eax");
    return result + (int)d;
}

/* High pressure function with many integer live values */
__attribute__((noipa))
int high_pressure_int_call(int iter) {
    /* Create many integer variables that must be kept in registers */
    int v1 = iter * 1;
    int v2 = iter * 2;
    int v3 = iter * 3;
    int v4 = iter * 4;
    int v5 = iter * 5;
    int v6 = iter * 6;
    int v7 = iter * 7;
    int v8 = iter * 8;
    int v9 = iter * 9;
    int v10 = iter * 10;
    int v11 = iter * 11;
    int v12 = iter * 12;
    int v13 = iter * 13;
    int v14 = iter * 14;
    int v15 = iter * 15;
    
    /* Use all variables in computation to keep them live */
    int sum1 = v1 + v2 + v3 + v4 + v5;
    int sum2 = v6 + v7 + v8 + v9 + v10;
    int sum3 = v11 + v12 + v13 + v14 + v15;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Call that clobbers call-used registers */
    unknown_effect_int(v1, v2, v3, v4, v5, v6);
    
    /* More computations to ensure variables stay live */
    sum1 = sum1 * v7 - v8 / (v9 + 1);
    sum2 = sum2 + v10 * v11 - v12;
    
    /* Another call */
    unknown_effect_int(v7, v8, v9, v10, v11, v12);
    
    /* Use remaining variables */
    sum3 = sum3 + v13 * v14 - v15;
    
    /* Final call */
    unknown_effect_int(v13, v14, v15, sum1, sum2, sum3);
    
    return sum1 + sum2 + sum3;
}

/* High pressure function with mixed integer and floating point */
__attribute__((noipa))
double high_pressure_mixed_call(int iter) {
    /* Mix integer and floating point variables */
    int i1 = iter * 1;
    int i2 = iter * 2;
    int i3 = iter * 3;
    int i4 = iter * 4;
    int i5 = iter * 5;
    int i6 = iter * 6;
    
    double d1 = iter * 1.1;
    double d2 = iter * 2.2;
    double d3 = iter * 3.3;
    double d4 = iter * 4.4;
    double d5 = iter * 5.5;
    double d6 = iter * 6.6;
    double d7 = iter * 7.7;
    double d8 = iter * 8.8;
    
    /* Complex computations mixing types */
    double temp1 = d1 * i1 + d2 * i2;
    double temp2 = d3 * i3 + d4 * i4;
    int itemp1 = i5 * (int)d5 + i6 * (int)d6;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call that uses both integer and FP registers */
    unknown_effect_mixed(i1, d1, i2, d2, i3, d3);
    
    /* More computations */
    temp1 = temp1 + d4 * i4 - d5;
    temp2 = temp2 * d6 + d7 / d8;
    itemp1 = itemp1 + i5 * i6 - (int)d7;
    
    /* Another call */
    unknown_effect_double(d4, d5, d6, d7);
    
    /* Use compute_something which returns a value */
    int computed = compute_something(i4, d4, i5, d5);
    
    /* Final complex expression */
    double result = temp1 + temp2 + d8 * computed + itemp1;
    
    return result;
}

/* Function with calls inside loops and conditionals */
__attribute__((noipa))
int high_pressure_loop_call(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many live variables inside the loop */
        int a = i * 1;
        int b = i * 2;
        int c = i * 3;
        int d = i * 4;
        int e = i * 5;
        int f = i * 6;
        int g = i * 7;
        int h = i * 8;
        
        double x = i * 1.1;
        double y = i * 2.2;
        double z = i * 3.3;
        double w = i * 4.4;
        
        /* Conditional with calls */
        if (i % 3 == 0) {
            unknown_effect_int(a, b, c, d, e, f);
            total += a + b;
        } else if (i % 3 == 1) {
            unknown_effect_double(x, y, z, w);
            total += (int)(x + y);
        } else {
            unknown_effect_mixed(a, x, b, y, c, z);
            total += a + (int)x;
        }
        
        /* More computations keeping variables live */
        int temp1 = a * b + c * d - e;
        double temp2 = x * y + z / w;
        
        /* Another call in the loop */
        unknown_effect_int(f, g, h, temp1, (int)temp2, i);
        
        /* Update total using all variables */
        total += temp1 + (int)temp2 + g + h;
    }
    
    return total;
}

/* Function with nested control flow */
__attribute__((noipa))
double complex_control_flow(int seed) {
    double result = 0.0;
    int counter = seed;
    
    /* Switch statement with calls */
    switch (seed % 4) {
        case 0: {
            int a = seed * 11;
            int b = seed * 22;
            int c = seed * 33;
            double x = seed * 1.234;
            double y = seed * 2.345;
            
            unknown_effect_int(a, b, c, seed, a+b, b+c);
            result = x + y + a + b;
            break;
        }
        case 1: {
            int d = seed * 44;
            int e = seed * 55;
            double z = seed * 3.456;
            double w = seed * 4.567;
            
            unknown_effect_double(z, w, z*2, w*2);
            result = d * e + z - w;
            break;
        }
        case 2: {
            int f = seed * 66;
            int g = seed * 77;
            int h = seed * 88;
            double u = seed * 5.678;
            double v = seed * 6.789;
            
            for (int i = 0; i < 3; i++) {
                unknown_effect_mixed(f+i, u+i, g+i, v+i, h+i, u+v);
                result += (f + g + h) * (u + v);
            }
            break;
        }
        default: {
            int j = seed * 99;
            double k = seed * 7.891;
            
            int computed = compute_something(j, k, seed, k*2);
            result = computed * k;
            break;
        }
    }
    
    return result;
}

int main() {
    int total_int = 0;
    double total_double = 0.0;
    
    printf("Starting caller-save test...\n");
    
    /* Create high register pressure by calling functions in loops */
    for (int i = 0; i < 100; i++) {
        /* Each call creates many live values across calls */
        total_int += high_pressure_int_call(i);
        total_double += high_pressure_mixed_call(i);
        
        /* Alternate between different patterns */
        if (i % 10 == 0) {
            total_int += high_pressure_loop_call(5);
            total_double += complex_control_flow(i);
        }
    }
    
    printf("Results: int_total = %d, double_total = %f\n", total_int, total_double);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
