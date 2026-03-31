/* Main test file to trigger caller-save insertion during reload */
#include <stdio.h>
#include <math.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* External functions that cannot be inlined */
extern void unknown_effect_int(int x);
extern void unknown_effect_double(double x);
extern void unknown_effect_mixed(int a, double b, int c, double d);

/* Prevent inlining of these functions */
#define NOINLINE __attribute__((noinline))

/* Function 1: High pressure with integer live values */
NOINLINE static int high_pressure_int_call(int iter) {
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
    
    /* Use them in computations */
    v1 = v1 + v2 * 3;
    v2 = v2 ^ v3;
    v3 = v3 | v4;
    v4 = v4 & v5;
    v5 = v5 << 2;
    v6 = v6 >> 1;
    v7 = v7 + v8 - v9;
    v8 = v8 * v10 / 2;
    v9 = v9 % 7;
    v10 = v10 ^ v11;
    v11 = v11 | v12;
    v12 = v12 & v13;
    v13 = v13 << 1;
    v14 = v14 >> 2;
    v15 = v15 + v1;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Call external function - all variables must be saved */
    unknown_effect_int(v1);
    
    /* More computations after call */
    v1 = v1 + v2;
    v2 = v2 + v3;
    v3 = v3 + v4;
    v4 = v4 + v5;
    v5 = v5 + v6;
    v6 = v6 + v7;
    v7 = v7 + v8;
    v8 = v8 + v9;
    v9 = v9 + v10;
    v10 = v10 + v11;
    v11 = v11 + v12;
    v12 = v12 + v13;
    v13 = v13 + v14;
    v14 = v14 + v15;
    v15 = v15 + v1;
    
    /* Another call with different arguments */
    unknown_effect_int(v15);
    
    /* Final computation and return */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
           v9 + v10 + v11 + v12 + v13 + v14 + v15;
}

/* Function 2: Mixed integer and floating point */
NOINLINE static double high_pressure_mixed_call(int seed) {
    /* Mix integer and floating point variables */
    int i1 = seed;
    int i2 = seed * 2;
    int i3 = seed * 3;
    int i4 = seed * 4;
    double d1 = seed * 1.1;
    double d2 = seed * 2.2;
    double d3 = seed * 3.3;
    double d4 = seed * 4.4;
    double d5 = seed * 5.5;
    double d6 = seed * 6.6;
    double d7 = seed * 7.7;
    double d8 = seed * 8.8;
    
    /* Complex mixed computations */
    d1 = d1 * d2 + i1;
    d2 = d2 / d3 - i2;
    d3 = d3 + d4 * i3;
    d4 = d4 - d5 / i4;
    i1 = i1 + (int)d1;
    i2 = i2 ^ (int)d2;
    i3 = i3 | (int)d3;
    i4 = i4 & (int)d4;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call with mixed arguments - requires saving both int and FP regs */
    unknown_effect_mixed(i1, d1, i2, d2);
    
    /* More mixed computations */
    d5 = d5 + sin(d1) * cos(d2);
    d6 = d6 + exp(d3) * log(fabs(d4) + 1.0);
    d7 = d7 + pow(d1, 2.0) + pow(d2, 3.0);
    d8 = d8 + sqrt(d3 * d3 + d4 * d4);
    i1 = i1 + (int)(d5 * 100);
    i2 = i2 ^ (int)(d6 * 100);
    i3 = i3 | (int)(d7 * 100);
    i4 = i4 & (int)(d8 * 100);
    
    /* Another call */
    unknown_effect_mixed(i3, d7, i4, d8);
    
    /* Final result */
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + i1 + i2 + i3 + i4;
}

/* Function 3: Nested calls in control flow */
NOINLINE static int nested_calls_in_loop(int start, int end) {
    int result = 0;
    
    /* Loop with calls inside - creates multiple insertion points */
    for (int i = start; i < end; i++) {
        int a = i * 3;
        int b = i * 5;
        int c = i * 7;
        double x = i * 1.5;
        double y = i * 2.5;
        double z = i * 3.5;
        
        /* Conditional with calls */
        if (i % 3 == 0) {
            a = a * 2;
            x = x * 1.5;
            unknown_effect_int(a);
            b = b + a;
            y = y + x;
        } else if (i % 3 == 1) {
            b = b * 3;
            y = y * 2.0;
            unknown_effect_double(y);
            c = c + b;
            z = z + y;
        } else {
            c = c * 4;
            z = z * 2.5;
            unknown_effect_mixed(a, x, b, y);
            a = a + c;
            x = x + z;
        }
        
        /* More variables live across calls */
        int d = a + b + c;
        double w = x + y + z;
        
        /* Switch statement with calls */
        switch (i % 4) {
            case 0:
                d = d * 2;
                unknown_effect_int(d);
                w = w * 1.1;
                break;
            case 1:
                d = d + 100;
                w = w + 10.5;
                unknown_effect_double(w);
                break;
            case 2:
                d = d ^ 0xFF;
                w = w / 2.0;
                unknown_effect_mixed(d, w, a, x);
                break;
            case 3:
                d = d | 0xAA;
                w = w * w;
                unknown_effect_int(d);
                break;
        }
        
        result += d + (int)w;
    }
    
    return result;
}

/* Function 4: Deep expression with calls in the middle */
NOINLINE static double complex_expression_with_call(double base) {
    /* Create many intermediate values */
    double t1 = sin(base);
    double t2 = cos(base);
    double t3 = tan(base);
    double t4 = exp(base);
    double t5 = log(fabs(base) + 1.0);
    double t6 = sqrt(fabs(base));
    double t7 = pow(base, 2.0);
    double t8 = pow(base, 3.0);
    
    /* Part 1 of computation */
    double sum1 = t1 * t2 + t3 * t4;
    double sum2 = t5 * t6 + t7 * t8;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call in the middle of expression */
    unknown_effect_double(sum1);
    
    /* Part 2 - uses results from before call */
    double prod1 = sum1 * t1;
    double prod2 = sum2 * t2;
    
    /* Another call */
    unknown_effect_double(prod1);
    
    /* Final computation */
    double result = (prod1 + prod2) * (t3 + t4) / (t5 + t6 + 0.001);
    
    /* One more call */
    unknown_effect_double(result);
    
    return result;
}

int main(void) {
    int total = 0;
    double total_d = 0.0;
    
    printf("Starting caller-save stress test...\n");
    
    /* Test 1: Integer pressure */
    for (int i = 0; i < 100; i++) {
        total += high_pressure_int_call(i);
    }
    
    /* Test 2: Mixed pressure */
    for (int i = 0; i < 50; i++) {
        total_d += high_pressure_mixed_call(i);
    }
    
    /* Test 3: Nested calls */
    total += nested_calls_in_loop(0, 100);
    
    /* Test 4: Complex expressions */
    for (int i = 1; i <= 20; i++) {
        total_d += complex_expression_with_call(i * 0.1);
    }
    
    /* Use results to prevent optimization */
    global_counter = total + (int)total_d;
    
    printf("Result: %d (int), %.2f (double)\n", total, total_d);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
