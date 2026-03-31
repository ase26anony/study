/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fdump-rtl-reload -fdump-rtl-all caller_save_test.c external_effects.c */

#include <stdio.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect_int(int a, int b, int c, int d, int e, int f);
extern void unknown_effect_double(double a, double b, double c, double d);
extern void unknown_effect_mixed(int a, double b, int c, double d);

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Function with high integer register pressure around call */
int high_pressure_int_call(int iter) {
    /* Many integer variables that must stay in registers */
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
    
    /* Use all variables in computation to keep them live */
    int sum1 = v1 + v2 + v3 + v4;
    int sum2 = v5 + v6 + v7 + v8;
    int sum3 = v9 + v10 + v11 + v12;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    unknown_effect_int(v1, v2, v3, v4, v5, v6);
    
    /* More computations after call - variables must be restored */
    int result1 = sum1 * v1 - v2 + v3 / (v4 ? v4 : 1);
    int result2 = sum2 * v5 - v6 + v7 / (v8 ? v8 : 1);
    int result3 = sum3 * v9 - v10 + v11 / (v12 ? v12 : 1);
    
    /* Use results to prevent elimination */
    global_counter += result1 + result2 + result3;
    
    return result1 + result2 * 2 + result3 * 3;
}

/* Function with high floating-point register pressure */
double high_pressure_double_call(double base) {
    /* Many floating-point variables */
    double d1 = base * 1.1;
    double d2 = base * 2.2;
    double d3 = base * 3.3;
    double d4 = base * 4.4;
    double d5 = base * 5.5;
    double d6 = base * 6.6;
    double d7 = base * 7.7;
    double d8 = base * 8.8;
    double d9 = base * 9.9;
    double d10 = base * 10.10;
    
    /* Complex FP computations */
    double prod1 = d1 * d2 * d3;
    double prod2 = d4 * d5 * d6;
    double prod3 = d7 * d8 * d9 * d10;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call that clobbers FP registers */
    unknown_effect_double(d1, d2, d3, d4);
    
    /* More FP computations after call */
    double result1 = prod1 + sin(d1) * cos(d2);
    double result2 = prod2 + sin(d3) * cos(d4);
    double result3 = prod3 + sin(d5) * cos(d6);
    
    global_counter += (int)(result1 + result2 + result3);
    
    return result1 * result2 / (result3 ? result3 : 1.0);
}

/* Function with mixed integer and floating-point pressure */
double high_pressure_mixed_call(int i, double d) {
    /* Mixed variables */
    int i1 = i + 1;
    int i2 = i + 2;
    int i3 = i + 3;
    int i4 = i + 4;
    int i5 = i + 5;
    int i6 = i + 6;
    
    double d1 = d * 1.234;
    double d2 = d * 2.345;
    double d3 = d * 3.456;
    double d4 = d * 4.567;
    double d5 = d * 5.678;
    double d6 = d * 6.789;
    
    /* Mixed computations */
    int int_sum = i1 * i2 - i3 + i4 / (i5 ? i5 : 1) * i6;
    double double_prod = d1 * d2 + d3 * d4 - d5 / (d6 ? d6 : 1.0);
    
    /* Complex expression forcing register use */
    double mixed = (double)i1 * d1 + (double)i2 * d2 - (double)i3 * d3;
    
    asm volatile("" : : : "memory");
    
    /* Call clobbering both integer and FP registers */
    unknown_effect_mixed(i1, d1, i2, d2);
    
    /* Post-call computations needing restored values */
    int int_result = int_sum + i1 * i3 - i2 * i4 + i5 * i6;
    double double_result = double_prod * d1 / d2 + d3 * d4 - d5 * d6;
    double final_mixed = mixed + (double)int_result * double_result;
    
    global_counter += (int)final_mixed;
    
    return final_mixed;
}

/* Main function with loops and multiple call patterns */
int main() {
    int total = 0;
    double total_d = 0.0;
    
    /* Loop with moderate iteration count */
    for (int i = 0; i < 100; i++) {
        /* Varying call patterns to trigger different allocation decisions */
        if (i % 3 == 0) {
            total += high_pressure_int_call(i);
        } else if (i % 3 == 1) {
            total_d += high_pressure_double_call((double)i);
        } else {
            total_d += high_pressure_mixed_call(i, (double)i / 10.0);
        }
        
        /* Additional call in loop to increase pressure */
        if (i % 7 == 0) {
            unknown_effect_int(i, i+1, i+2, i+3, i+4, i+5);
        }
    }
    
    /* Use results to prevent elimination */
    printf("Result: %d, %f, global: %d\n", total, total_d, global_counter);
    
    return total > 0 ? 0 : 1;
}
