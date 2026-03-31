/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fno-inline -fdump-rtl-reload caller-save-test.c external.c */

#include <stdio.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int a, double b);
extern void another_effect(float f, long l);
extern void third_effect(double d1, double d2, int i);

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Barrier to prevent reordering and keep values live */
#define BARRIER() asm volatile("" : : : "memory")

/* Function 1: High pressure with mixed integer/float variables */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Unpack into many scalar variables - all must be kept live */
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
    
    float f1 = (float)doubles[7];
    float f2 = (float)doubles[8];
    float f3 = f1 * f2;
    
    long l1 = (long)ints[9] * 1000;
    long l2 = (long)ints[10] << 4;
    
    /* Use all variables in computation before call */
    int sum_int = v1 + v2 + v3 + v4 + v5;
    double sum_double = d1 + d2 + d3 + d4 + d5;
    float prod_float = f1 * f2 * f3;
    long sum_long = l1 + l2;
    
    /* Force all values to be live across the call */
    BARRIER();
    
    /* External call that clobbers call-used registers */
    unknown_effect(sum_int, sum_double);
    
    BARRIER();
    
    /* Use all variables again after call - forces caller-save restoration */
    v1 = v1 * 2 + global_counter;
    v2 = v2 / 3 + global_counter;
    v3 = v3 ^ 0xFF;
    v4 = v4 << 2;
    v5 = v5 >> 1;
    
    d1 = d1 * 2.0 + global_counter;
    d2 = d2 / 3.0 + global_counter;
    d3 = d3 * d3;
    d4 = sin(d4);
    d5 = cos(d5);
    
    f1 = f1 + 1.0f;
    f2 = f2 - 1.0f;
    f3 = f3 * 2.0f;
    
    l1 = l1 | 0xFFFF;
    l2 = l2 & 0xFFFFFF;
    
    /* Complex computation mixing all variables */
    double result = (double)v1 + (double)v2 + d1 + d2 + (double)f1 + (double)l1;
    result = result * (double)v3 - (double)v4 / d3 + (double)f2 * (double)l2;
    
    return (int)result + global_counter;
}

/* Function 2: Different pattern with nested control flow */
double high_pressure_call_2(int *ints, double *doubles, int selector) {
    /* Even more variables */
    int a1 = ints[0], a2 = ints[1], a3 = ints[2], a4 = ints[3], a5 = ints[4];
    int b1 = ints[5], b2 = ints[6], b3 = ints[7], b4 = ints[8], b5 = ints[9];
    
    double x1 = doubles[0], x2 = doubles[1], x3 = doubles[2];
    double y1 = doubles[3], y2 = doubles[4], y3 = doubles[5];
    double z1 = doubles[6], z2 = doubles[7], z3 = doubles[8];
    
    /* Pre-call computations */
    int int_sum = a1 + a2 + a3 + a4 + a5;
    double double_prod = x1 * x2 * x3;
    float f1 = (float)y1, f2 = (float)y2, f3 = (float)y3;
    
    /* Call inside conditional */
    if (selector > 0) {
        BARRIER();
        another_effect(f1, (long)int_sum);
        BARRIER();
    } else {
        BARRIER();
        third_effect(double_prod, y1, int_sum);
        BARRIER();
    }
    
    /* Post-call computations using all variables */
    a1 = a1 * selector;
    a2 = a2 + selector;
    x1 = x1 / (selector + 1.0);
    x2 = x2 * selector;
    
    /* Another call in a loop */
    for (int i = 0; i < 3; i++) {
        double temp = (double)a1 + x1 + (double)b1 + z1;
        BARRIER();
        unknown_effect(i, temp);
        BARRIER();
        a1++;
        x1 += 0.5;
    }
    
    /* Mix all results */
    double final = (double)(a1 * a2 * a3) + x1 * x2 * x3 + 
                   (double)(b1 + b2 + b3) + y1 + y2 + y3 +
                   z1 * z2 * z3;
    
    return final;
}

/* Function 3: Loop with multiple call sites */
void high_pressure_loop(int *data, double *coeffs, int n) {
    for (int i = 0; i < n; i++) {
        /* Create many live values */
        int base = data[i];
        double scale = coeffs[i % 10];
        
        int v1 = base * 2;
        int v2 = base + i;
        int v3 = base ^ 0xAA;
        int v4 = base << 3;
        int v5 = base >> 1;
        
        double d1 = scale * 1.1;
        double d2 = scale / 1.2;
        double d3 = sin(scale);
        double d4 = cos(scale);
        double d5 = scale * scale;
        
        float f1 = (float)scale * 0.5f;
        float f2 = (float)d1 * 2.0f;
        
        /* Use them */
        int int_sum = v1 + v2 + v3 + v4 + v5;
        double double_sum = d1 + d2 + d3 + d4 + d5;
        float float_prod = f1 * f2;
        
        /* Call that clobbers registers */
        BARRIER();
        unknown_effect(int_sum, double_sum);
        BARRIER();
        
        /* More computations */
        v1 = v1 * (int)double_sum;
        v2 = v2 + (int)float_prod;
        d1 = d1 * (double)v1;
        d2 = d2 / (double)v2;
        
        /* Another call */
        BARRIER();
        another_effect(f1, (long)v1);
        BARRIER();
        
        /* Store result */
        data[i] = v1 + v2 + (int)d1 + (int)d2;
    }
}

/* Main driver with multiple call patterns */
int main() {
    /* Create test data */
    int int_data[100];
    double double_data[100];
    
    for (int i = 0; i < 100; i++) {
        int_data[i] = i * 3 + 7;
        double_data[i] = (double)i * 0.5 + 1.0;
    }
    
    int result1 = 0;
    double result2 = 0.0;
    
    /* Loop to increase register pressure */
    for (int iter = 0; iter < 100; iter++) {
        /* Mix different high-pressure functions */
        result1 += high_pressure_call_1(int_data + iter, double_data + iter);
        
        if (iter % 3 == 0) {
            result2 += high_pressure_call_2(int_data, double_data, iter);
        }
        
        /* Every 10 iterations, do the loop version */
        if (iter % 10 == 0) {
            high_pressure_loop(int_data, double_data, 20);
        }
        
        global_counter++;
    }
    
    printf("Results: %d, %f\n", result1, result2);
    return 0;
}
