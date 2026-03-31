/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-optimize-sibling-calls caller_save_test.c external_effects.c -o caller_save_test */

#include <stdio.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect_int(int x);
extern void unknown_effect_double(double x);
extern void unknown_effect_mixed(int a, double b, int c, double d);
extern int get_global_counter(void);

/* Global volatile to prevent optimization */
volatile int global_volatile = 0;

/* Function with extreme register pressure around calls */
/* 15+ live variables across calls, mixed int/float */
double high_pressure_function_1(int iterations) {
    /* Declare many variables to create register pressure */
    int v1 = iterations * 2;
    int v2 = iterations + 1;
    int v3 = iterations - 5;
    int v4 = iterations * 3;
    int v5 = iterations / 2;
    int v6 = iterations % 7;
    int v7 = iterations << 2;
    int v8 = iterations ^ 0x55;
    int v9 = iterations | 0xAA;
    int v10 = iterations & 0xFF;
    
    double d1 = iterations * 1.5;
    double d2 = iterations * 2.5;
    double d3 = iterations * 3.14159;
    double d4 = iterations / 2.0;
    double d5 = sqrt(iterations + 1.0);
    double d6 = sin(iterations * 0.1);
    double d7 = cos(iterations * 0.2);
    double d8 = d1 * d2 + d3;
    
    /* Use all variables in computation before call */
    int sum_int = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    double sum_double = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call - clobbers call-used registers */
    unknown_effect_mixed(sum_int, sum_double, v1, d1);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Complex computation using all variables after call */
    /* This ensures they must be restored from spills */
    v1 = v1 * 2 + v2;
    v2 = v2 * 3 + v3;
    v3 = v3 * 4 + v4;
    v4 = v4 * 5 + v5;
    v5 = v5 * 6 + v6;
    v6 = v6 * 7 + v7;
    v7 = v7 * 8 + v8;
    v8 = v8 * 9 + v9;
    v9 = v9 * 10 + v10;
    v10 = v10 * 11 + v1;
    
    d1 = d1 * 1.1 + d2;
    d2 = d2 * 1.2 + d3;
    d3 = d3 * 1.3 + d4;
    d4 = d4 * 1.4 + d5;
    d5 = d5 * 1.5 + d6;
    d6 = d6 * 1.6 + d7;
    d7 = d7 * 1.7 + d8;
    d8 = d8 * 1.8 + d1;
    
    /* Another call with different arguments */
    unknown_effect_int(v1 + v2 + v3);
    
    /* More computations */
    double result = d1 + d2 + d3 + d4;
    result += d5 + d6 + d7 + d8;
    result += v1 + v2 + v3 + v4;
    result += v5 + v6 + v7 + v8;
    result += v9 + v10;
    
    /* Final call */
    unknown_effect_double(result);
    
    return result;
}

/* Second high-pressure function with different pattern */
float high_pressure_function_2(int base) {
    /* Different mix of variables */
    int a1 = base * 11;
    int a2 = base * 13;
    int a3 = base * 17;
    int a4 = base * 19;
    int a5 = base * 23;
    
    float f1 = base * 1.11f;
    float f2 = base * 2.22f;
    float f3 = base * 3.33f;
    float f4 = base * 4.44f;
    float f5 = base * 5.55f;
    float f6 = base * 6.66f;
    float f7 = base * 7.77f;
    float f8 = base * 8.88f;
    float f9 = base * 9.99f;
    float f10 = base * 10.10f;
    
    /* Use in conditional to create control flow */
    if (base % 2 == 0) {
        a1 += a2;
        f1 += f2;
        unknown_effect_int(a1);
    } else {
        a3 += a4;
        f3 += f4;
        unknown_effect_int(a3);
    }
    
    /* Loop with call inside - forces caller-save in loop body */
    float accum = 0.0f;
    for (int i = 0; i < 10; i++) {
        /* All variables live across this call */
        a1 += i;
        a2 += i * 2;
        f1 += i * 0.1f;
        f2 += i * 0.2f;
        
        /* Call in loop - register pressure is high here */
        unknown_effect_mixed(a1, f1, a2, f2);
        
        /* More computations */
        accum += f1 + f2 + f3 + f4 + f5;
        accum += a1 + a2 + a3 + a4 + a5;
    }
    
    /* Switch statement with calls at multiple points */
    switch (base % 4) {
        case 0:
            unknown_effect_int(a1 + a2);
            f1 = f1 * 2.0f;
            break;
        case 1:
            unknown_effect_int(a3 + a4);
            f2 = f2 * 3.0f;
            break;
        case 2:
            unknown_effect_int(a5);
            f3 = f3 * 4.0f;
            break;
        default:
            unknown_effect_int(a1 + a3 + a5);
            f4 = f4 * 5.0f;
            break;
    }
    
    return accum + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
}

/* Third function with nested loops and calls */
double high_pressure_function_3(int seed) {
    double total = 0.0;
    
    /* Outer loop */
    for (int i = 0; i < 5; i++) {
        int x1 = seed + i * 10;
        int x2 = seed + i * 20;
        int x3 = seed + i * 30;
        int x4 = seed + i * 40;
        
        double y1 = seed * 0.01 + i;
        double y2 = seed * 0.02 + i;
        double y3 = seed * 0.03 + i;
        double y4 = seed * 0.04 + i;
        
        /* Inner loop with call */
        for (int j = 0; j < 3; j++) {
            /* Modify all variables */
            x1 += j;
            x2 += j * 2;
            x3 += j * 3;
            x4 += j * 4;
            
            y1 += j * 0.1;
            y2 += j * 0.2;
            y3 += j * 0.3;
            y4 += j * 0.4;
            
            /* Call with many live values */
            unknown_effect_mixed(x1, y1, x2, y2);
            
            /* Use results immediately */
            total += x1 + x2 + x3 + x4;
            total += y1 + y2 + y3 + y4;
        }
        
        /* Another call after inner loop */
        unknown_effect_int(x3 + x4);
    }
    
    return total;
}

int main(void) {
    double total_result = 0.0;
    
    /* Create varying register pressure patterns */
    for (int i = 0; i < 100; i++) {
        /* Call different high-pressure functions */
        total_result += high_pressure_function_1(i);
        total_result += high_pressure_function_2(i);
        total_result += high_pressure_function_3(i);
        
        /* Occasionally call external function directly */
        if (i % 7 == 0) {
            unknown_effect_int(i);
        }
        
        /* Reference global volatile to prevent dead code elimination */
        global_volatile = i;
    }
    
    printf("Total result: %f\n", total_result);
    printf("Global volatile: %d\n", global_volatile);
    
    return 0;
}
