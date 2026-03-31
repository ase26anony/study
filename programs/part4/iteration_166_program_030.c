/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fdump-rtl-reload -fdump-rtl-all caller_save_test.c external_effects.c */

#include <stdio.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect_int(int x);
extern void unknown_effect_double(double x);
extern void unknown_effect_mixed(int a, double b);
extern int get_global_counter(void);

/* Global volatile to prevent optimization */
volatile int global_volatile = 0;

/* Function 1: High integer register pressure around call */
int high_pressure_int_call(int iter) {
    /* Create many integer variables that must stay in registers */
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
    
    /* Use all variables in computation before call */
    int sum_before = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    sum_before += v11 + v12 + v13 + v14 + v15;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    unknown_effect_int(sum_before);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex computation using all variables after call */
    int result = v1 * v2 - v3 + v4 / (v5 + 1);
    result += (v6 << 2) | (v7 & 0xFF);
    result += v8 * v9 - v10 * v11;
    result += v12 % (v13 + 1) + v14 - v15;
    
    /* Use in conditional to prevent dead code elimination */
    if (result > 1000) {
        result = result / 2 + sum_before;
    } else {
        result = result * 3 - sum_before;
    }
    
    return result;
}

/* Function 2: Mixed integer and floating point pressure */
double high_pressure_mixed_call(int seed) {
    /* Integer variables */
    int i1 = seed + 1;
    int i2 = seed + 2;
    int i3 = seed + 3;
    int i4 = seed + 4;
    int i5 = seed + 5;
    
    /* Floating point variables */
    double d1 = seed * 1.1;
    double d2 = seed * 2.2;
    double d3 = seed * 3.3;
    double d4 = seed * 4.4;
    double d5 = seed * 5.5;
    double d6 = seed * 6.6;
    double d7 = seed * 7.7;
    double d8 = seed * 8.8;
    
    /* Complex mixed computation before call */
    double mixed_sum = d1 * i1 + d2 / i2 - d3 * i3;
    mixed_sum += sin(d4) * cos(d5) + tan(d6);
    
    /* Use in loop to increase pressure */
    for (int j = 0; j < 3; j++) {
        mixed_sum += (i4 + j) * (d7 / (j + 1));
        mixed_sum -= (i5 - j) * (d8 * (j + 1));
    }
    
    asm volatile("" : : : "memory");
    
    /* Call that uses both integer and floating point arguments */
    unknown_effect_mixed(i1 + i2, d1 + d2);
    
    asm volatile("" : : : "memory");
    
    /* More computation after call, using all variables */
    double result = 0.0;
    result += i1 * d1 + i2 * d2 - i3 * d3;
    result += pow(d4, d5) / (i4 + 1);
    result += log(fabs(d6) + 1.0) * i5;
    result += d7 * d8 * (i1 + i2 + i3);
    
    /* Conditional with floating point */
    if (result > 100.0) {
        result = sqrt(result) + mixed_sum;
    } else {
        result = result * result - mixed_sum;
    }
    
    return result;
}

/* Function 3: Nested calls in control flow */
int nested_calls_pressure(int base) {
    int result = base;
    
    /* Loop with multiple calls */
    for (int i = 0; i < 10; i++) {
        /* Create live variables across call */
        int live1 = result + i * 2;
        int live2 = result - i * 3;
        double live3 = result * 1.5 + i;
        double live4 = result * 2.5 - i;
        
        /* Complex condition */
        if (i % 3 == 0) {
            /* Call in one branch */
            asm volatile("" : : : "memory");
            unknown_effect_int(live1);
            asm volatile("" : : : "memory");
            
            result += live1 * 2;
            result += (int)(live3 * 10);
        } else if (i % 3 == 1) {
            /* Different call in another branch */
            asm volatile("" : : : "memory");
            unknown_effect_double(live3);
            asm volatile("" : : : "memory");
            
            result -= live2 / 2;
            result += (int)(live4 * 5);
        } else {
            /* Mixed call in third branch */
            asm volatile("" : : : "memory");
            unknown_effect_mixed(live1 + live2, live3 + live4);
            asm volatile("" : : : "memory");
            
            result = result * 3 - live1 + live2;
        }
        
        /* Use variables after call in all branches */
        live3 = live3 * 1.1;
        live4 = live4 / 1.1;
        result += (int)(live3 + live4);
    }
    
    return result;
}

/* Function 4: Switch statement with calls */
double switch_calls_pressure(int selector) {
    double accumulator = 0.0;
    
    /* Many live variables */
    double d1 = selector * 1.1;
    double d2 = selector * 2.2;
    double d3 = selector * 3.3;
    int i1 = selector + 100;
    int i2 = selector + 200;
    int i3 = selector + 300;
    
    switch (selector % 5) {
        case 0:
            accumulator = d1 * d2;
            asm volatile("" : : : "memory");
            unknown_effect_double(d1);
            asm volatile("" : : : "memory");
            accumulator += d3 * i1;
            break;
            
        case 1:
            accumulator = d2 / d3;
            asm volatile("" : : : "memory");
            unknown_effect_int(i2);
            asm volatile("" : : : "memory");
            accumulator -= i2 * d1;
            break;
            
        case 2:
            accumulator = sin(d1) + cos(d2);
            asm volatile("" : : : "memory");
            unknown_effect_mixed(i3, d3);
            asm volatile("" : : : "memory");
            accumulator *= 1.5;
            break;
            
        case 3:
            accumulator = tan(d3) * log(fabs(d1) + 1.0);
            /* Two calls in sequence */
            asm volatile("" : : : "memory");
            unknown_effect_int(i1);
            asm volatile("" : : : "memory");
            unknown_effect_double(d2);
            asm volatile("" : : : "memory");
            accumulator /= 2.0;
            break;
            
        case 4:
            accumulator = pow(d1, 2.0) + pow(d2, 3.0);
            asm volatile("" : : : "memory");
            unknown_effect_mixed(i1 + i2, d1 + d2 + d3);
            asm volatile("" : : : "memory");
            accumulator = sqrt(accumulator);
            break;
    }
    
    /* Use all variables after switch */
    accumulator += i1 * d1 + i2 * d2 + i3 * d3;
    return accumulator;
}

int main(void) {
    int total = 0;
    double total_double = 0.0;
    
    /* Loop to ensure caller-save insertion is triggered multiple times */
    for (int i = 0; i < 100; i++) {
        /* Call different high-pressure functions */
        int res1 = high_pressure_int_call(i);
        double res2 = high_pressure_mixed_call(i);
        int res3 = nested_calls_pressure(i);
        double res4 = switch_calls_pressure(i);
        
        total += res1 + res3;
        total_double += res2 + res4;
        
        /* Prevent loop unrolling */
        if (i % 10 == 0) {
            global_volatile = i;
        }
    }
    
    /* Use results to prevent optimization */
    printf("Integer total: %d\n", total);
    printf("Double total: %f\n", total_double);
    
    return total > 0 ? 0 : 1;
}
