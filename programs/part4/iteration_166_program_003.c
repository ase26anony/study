/* Test program to trigger caller-save insertion logic in reload pass */
/* Specifically targets lines 905-913 in caller-save.cc */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect_int(int x);
extern void unknown_effect_double(double x);
extern void unknown_effect_mixed(int a, double b, int c, double d);
extern void unknown_effect_multi(int a, int b, double c, double d, int e, double f);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: High integer register pressure around call */
__attribute__((noinline))
int high_int_pressure(int iter) {
    /* Many integer variables that must stay in registers */
    int v1 = iter * 2;
    int v2 = iter + 12345;
    int v3 = iter ^ 0xABCDEF;
    int v4 = iter * iter;
    int v5 = v1 + v2 + v3;
    int v6 = v4 - v1;
    int v7 = v2 * v3;
    int v8 = v5 ^ v6;
    int v9 = v7 + v8;
    int v10 = v9 * 3;
    int v11 = v10 - v1;
    int v12 = v11 + v2;
    int v13 = v12 * v3;
    int v14 = v13 ^ v4;
    int v15 = v14 + v5;
    
    /* Use all variables in computation before call */
    int sum_before = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                     v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    unknown_effect_int(sum_before);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all variables again after call - they must be restored */
    int sum_after = (v1 * 2) + (v2 / 3) + (v3 ^ 0xFF) + (v4 - 100) +
                    (v5 + 50) + (v6 * v7) + (v8 & 0xFFFF) +
                    (v9 | 0xFF00) + (v10 - v11) + (v12 + v13) +
                    (v14 ^ v15) + (v1 * v15) + (v2 + v14) +
                    (v3 * v13);
    
    return sum_before + sum_after;
}

/* Function 2: Mixed integer and floating-point pressure */
__attribute__((noinline))
double mixed_pressure(int base, double multiplier) {
    /* Mix of integer and floating variables */
    int i1 = base;
    int i2 = base + 1;
    int i3 = base * 2;
    int i4 = base ^ 0x1234;
    int i5 = i1 + i2 + i3;
    
    double d1 = multiplier;
    double d2 = multiplier * 1.5;
    double d3 = multiplier / 2.0;
    double d4 = sin(multiplier);
    double d5 = cos(multiplier);
    double d6 = d1 + d2 + d3;
    double d7 = d4 * d5;
    double d8 = d6 / d7;
    
    /* Complex computation using all variables */
    double result1 = (i1 * d1) + (i2 * d2) + (i3 * d3) + (i4 * d4) + (i5 * d5);
    double result2 = (d6 * i1) + (d7 * i2) + (d8 * i3);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call that uses both int and double registers */
    unknown_effect_mixed(i1, d1, i2, d2);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* More computations after call */
    double final_result = result1 * result2;
    final_result += (i3 * d3) / (i4 + 1);
    final_result += sin(d4) * cos(d5);
    final_result += (i5 * d6) - (d7 * d8);
    
    return final_result;
}

/* Function 3: Extreme pressure with many live values across nested calls */
__attribute__((noinline))
double extreme_pressure(int seed) {
    /* Create many live values */
    double vals[20];
    int ints[15];
    
    for (int i = 0; i < 20; i++) {
        vals[i] = sin(seed * 0.1 * i) * cos(seed * 0.05 * i);
    }
    
    for (int i = 0; i < 15; i++) {
        ints[i] = seed * (i + 1) ^ 0xDEADBEEF;
    }
    
    /* Use values in complex computation */
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    
    for (int i = 0; i < 10; i++) {
        sum1 += vals[i] * ints[i % 15];
        sum2 += vals[i + 5] * ints[(i + 3) % 15];
        sum3 += vals[i + 10] * ints[(i + 7) % 15];
    }
    
    /* First external call */
    asm volatile("" : : : "memory");
    unknown_effect_multi(ints[0], ints[1], vals[0], vals[1], ints[2], vals[2]);
    asm volatile("" : : : "memory");
    
    /* Intermediate computation keeping values live */
    double intermediate = sum1 * sum2 / (sum3 + 1.0);
    
    /* Second call in conditional */
    if (intermediate > 0) {
        asm volatile("" : : : "memory");
        unknown_effect_double(intermediate);
        asm volatile("" : : : "memory");
        
        intermediate *= 1.5;
    } else {
        asm volatile("" : : : "memory");
        unknown_effect_int((int)intermediate);
        asm volatile("" : : : "memory");
        
        intermediate /= 2.0;
    }
    
    /* Final computation using all original values */
    double final_val = 0.0;
    for (int i = 0; i < 15; i++) {
        final_val += vals[i % 20] * ints[i];
    }
    
    return final_val + intermediate;
}

/* Function 4: Loop with calls creating multiple insertion points */
__attribute__((noinline))
double loop_with_calls(int iterations) {
    double accumulator = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live values inside loop */
        double a = sin(i * 0.01);
        double b = cos(i * 0.02);
        double c = a * b;
        double d = a + b;
        double e = a - b;
        
        int x = i * 3;
        int y = i + 100;
        int z = x ^ y;
        
        /* Complex computation */
        double temp = (a * x) + (b * y) + (c * z) + (d * x * y) + (e * y * z);
        
        /* Call inside loop - will need caller-save at each iteration */
        asm volatile("" : : : "memory");
        unknown_effect_mixed(x, a, y, b);
        asm volatile("" : : : "memory");
        
        /* Use values after call */
        accumulator += temp + (z * d) - (x * e);
        
        /* Conditional with another call */
        if (i % 7 == 0) {
            double special = accumulator * 0.1;
            asm volatile("" : : : "memory");
            unknown_effect_double(special);
            asm volatile("" : : : "memory");
            accumulator += special;
        }
    }
    
    return accumulator;
}

/* Main function with multiple call sites */
int main() {
    double total = 0.0;
    
    /* Call different high-pressure functions in loops */
    for (int i = 0; i < 100; i++) {
        int int_result = high_int_pressure(i);
        total += int_result;
        
        double mixed_result = mixed_pressure(i, i * 0.5);
        total += mixed_result;
        
        if (i % 10 == 0) {
            double extreme_result = extreme_pressure(i);
            total += extreme_result;
        }
    }
    
    /* Loop with internal calls */
    double loop_result = loop_with_calls(50);
    total += loop_result;
    
    /* Use result to prevent optimization */
    printf("Total result: %f\n", total);
    global_counter = (int)total;
    
    return (global_counter > 0) ? 0 : 1;
}
