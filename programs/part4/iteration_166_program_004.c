/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-inline -fdump-rtl-reload caller_save_test.c external_effects.c */

#include <stdio.h>
#include <math.h>

/* External functions with side effects - prevent inlining */
extern void unknown_effect_int(int val);
extern void unknown_effect_double(double val);
extern void unknown_effect_mixed(int i, double d);
extern int get_global_counter(void);

/* Global volatile to prevent optimization */
volatile int global_volatile = 0;

/* Function 1: High integer register pressure around call */
int high_pressure_int_call(int iter) {
    /* Many integer variables that must stay in registers */
    int v1 = iter * 2;
    int v2 = iter + 100;
    int v3 = iter ^ 0xABCD;
    int v4 = iter * iter;
    int v5 = v1 + v2;
    int v6 = v3 - v4;
    int v7 = v5 * v6;
    int v8 = v2 ^ v3;
    int v9 = v4 + v7;
    int v10 = v5 - v8;
    int v11 = v6 * v9;
    int v12 = v7 ^ v10;
    int v13 = v8 + v11;
    int v14 = v9 - v12;
    int v15 = v10 * v13;
    
    /* All these variables are live across the call */
    /* Use them in computation before call */
    int sum_before = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                     v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    unknown_effect_int(sum_before);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all variables after call - they must be restored */
    int sum_after = (v1 * 2) + (v2 / 3) + (v3 ^ 0xFF) + (v4 - 5) +
                    (v5 * v6) + (v7 ^ v8) + (v9 + v10) + (v11 - v12) +
                    (v13 * v14) + (v15 ^ iter);
    
    /* Complex conditional to prevent optimization */
    if (sum_after > 1000) {
        return sum_before + sum_after;
    } else {
        return sum_before - sum_after;
    }
}

/* Function 2: Mixed integer and floating-point pressure */
double high_pressure_mixed_call(double base) {
    /* Mix integer and floating point variables */
    int i1 = (int)base;
    int i2 = i1 * 2;
    int i3 = i2 + 100;
    int i4 = i3 ^ 0x1234;
    int i5 = i4 - i1;
    
    double d1 = base * 1.1;
    double d2 = base / 2.0;
    double d3 = sin(base);
    double d4 = d1 * d2;
    double d5 = d3 + d4;
    double d6 = d2 - d3;
    double d7 = d4 * d5;
    double d8 = d6 / d1;
    
    /* More variables to increase pressure */
    int i6 = i5 * 3;
    double d9 = d7 + d8;
    int i7 = i6 ^ i1;
    double d10 = d9 * 2.0;
    
    /* Computation using all variables */
    double result_before = (d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10) * 
                           (i1 + i2 + i3 + i4 + i5 + i6 + i7);
    
    /* Call that clobbers both integer and FP registers */
    asm volatile("" : : : "memory");
    unknown_effect_mixed(i7, d10);
    asm volatile("" : : : "memory");
    
    /* Use variables after call - requires restoration */
    double result_after = (d1 * i1) + (d2 * i2) + (d3 * i3) + 
                          (d4 * i4) + (d5 * i5) + (d6 * i6) + 
                          (d7 * i7) + d8 + d9 + d10;
    
    /* Control flow to complicate register allocation */
    for (int j = 0; j < 3; j++) {
        result_after += (d1 * j) + (i1 ^ j);
        if (j % 2 == 0) {
            unknown_effect_double(result_after);
        }
    }
    
    return result_before + result_after;
}

/* Function 3: Nested calls in loops with high pressure */
int nested_calls_high_pressure(int start) {
    int total = 0;
    
    /* Outer loop */
    for (int i = 0; i < 10; i++) {
        /* Many live variables in the loop */
        int a = start + i;
        int b = a * 2;
        int c = b + 100;
        int d = c ^ 0xDEAD;
        int e = d - a;
        int f = e * 3;
        int g = f ^ b;
        int h = g + c;
        int j = h - d;
        int k = j * e;
        
        double x = sin(a * 0.1);
        double y = cos(b * 0.2);
        double z = x * y;
        double w = z + x - y;
        
        /* Inner conditional with call */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
            unknown_effect_mixed(k, w);
            asm volatile("" : : : "memory");
            
            /* Use variables after call */
            total += a + b + c + (int)(x * 100) + (int)(y * 100);
        } else if (i % 3 == 1) {
            asm volatile("" : : : "memory");
            unknown_effect_int(f);
            asm volatile("" : : : "memory");
            
            total += d + e + f + (int)(z * 50);
        } else {
            asm volatile("" : : : "memory");
            unknown_effect_double(w);
            asm volatile("" : : : "memory");
            
            total += g + h + j + k;
        }
        
        /* More computation keeping variables live */
        total += (int)((x + y + z + w) * 10);
    }
    
    return total;
}

/* Function 4: Switch statement with multiple call sites */
double switch_with_calls(int mode, double input) {
    double result = input;
    
    /* Many variables live across switch */
    int i1 = (int)(input * 10);
    int i2 = i1 * 2;
    int i3 = i2 + 50;
    int i4 = i3 ^ 0xBEAF;
    
    double d1 = input * 2.0;
    double d2 = sin(input);
    double d3 = d1 * d2;
    double d4 = d3 + input;
    
    switch (mode % 4) {
        case 0:
            asm volatile("" : : : "memory");
            unknown_effect_int(i1);
            result = d1 + i1;
            break;
        case 1:
            asm volatile("" : : : "memory");
            unknown_effect_double(d2);
            result = d2 * i2;
            break;
        case 2:
            asm volatile("" : : : "memory");
            unknown_effect_mixed(i3, d3);
            result = d3 - i3;
            break;
        case 3:
            asm volatile("" : : : "memory");
            unknown_effect_int(i4);
            unknown_effect_double(d4);
            result = d4 / (i4 + 1);
            break;
    }
    
    asm volatile("" : : : "memory");
    
    /* Use all variables after switch - must be restored */
    return result + (i1 + i2 + i3 + i4) * 0.01 + (d1 + d2 + d3 + d4);
}

/* Main function with loops calling high-pressure functions */
int main(void) {
    int total = 0;
    double fp_total = 0.0;
    
    /* Loop to ensure caller-save insertion is triggered multiple times */
    for (int i = 0; i < 100; i++) {
        /* Call different high-pressure functions */
        total += high_pressure_int_call(i);
        
        fp_total += high_pressure_mixed_call(i * 0.1);
        
        if (i % 10 == 0) {
            total += nested_calls_high_pressure(i);
        }
        
        fp_total += switch_with_calls(i, i * 0.05);
    }
    
    /* Use results to prevent dead code elimination */
    global_volatile = total;
    printf("Result: %d, FP: %f\n", total, fp_total);
    
    return total > 0 ? 0 : 1;
}
