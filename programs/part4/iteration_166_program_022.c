/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fdump-rtl-reload -fdump-rtl-all caller_save_test.c -o caller_save_test */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int, double) __attribute__((noinline));
extern void unknown_effect2(float, int, double) __attribute__((noinline));
extern void unknown_effect3(double, double, int) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* External function definitions (will be in separate file or marked noinline) */
void unknown_effect(int a, double b) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_counter += a + (int)b;
}

void unknown_effect2(float a, int b, double c) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
    global_counter += b + (int)(a * c);
}

void unknown_effect3(double a, double b, int c) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
    global_counter += c + (int)(a + b);
}

/* High pressure function with many live values across call */
double high_pressure_call(int *ints, double *doubles) {
    /* Unpack into many scalar variables - all must be kept live */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + v1;
    int v3 = ints[2] - v2;
    int v4 = ints[3] * v3;
    int v5 = ints[4] / (v4 ? v4 : 1);
    int v6 = ints[5] ^ v5;
    int v7 = ints[6] | v6;
    int v8 = ints[7] & v7;
    int v9 = ints[8] << 2;
    int v10 = ints[9] >> 1;
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] + d1;
    double d3 = doubles[2] - d2;
    double d4 = doubles[3] * d3;
    double d5 = doubles[4] / (d4 + 1.0);
    double d6 = doubles[5] * sin(d5);
    double d7 = doubles[6] + cos(d6);
    double d8 = doubles[7] * exp(d7);
    double d9 = doubles[8] / (d8 + 2.0);
    double d10 = doubles[9] * tan(d9);
    
    /* Mix integer and floating point computations */
    double mix1 = v1 * d1;
    double mix2 = v2 + d2;
    float mix3 = v3 * d3;
    double mix4 = v4 / (d4 + 1.0);
    
    /* Call external function - all above variables must be preserved */
    /* This creates massive register pressure */
    unknown_effect(v1, d1);
    
    /* Use all variables after call to keep them live */
    v2 = v2 + v1;
    v3 = v3 * v2;
    v4 = v4 ^ v3;
    v5 = v5 | v4;
    v6 = v6 & v5;
    v7 = v7 << 1;
    v8 = v8 >> 2;
    v9 = v9 + v8;
    v10 = v10 - v9;
    
    d2 = d2 + d1 * 2.0;
    d3 = d3 - d2 / 3.0;
    d4 = d4 * sin(d3);
    d5 = d5 + cos(d4);
    d6 = d6 * exp(d5);
    d7 = d7 / (d6 + 1.0);
    d8 = d8 + tan(d7);
    d9 = d9 * atan(d8);
    d10 = d10 + log(d9 + 1.0);
    
    mix1 = mix1 * 2.0;
    mix2 = mix2 + v2;
    mix3 = mix3 * d3;
    mix4 = mix4 / (v4 + 1.0);
    
    /* Another call with different signature */
    unknown_effect2(mix3, v5, d5);
    
    /* More computations */
    v6 = v6 + (int)mix1;
    v7 = v7 * (int)mix2;
    v8 = v8 ^ (int)mix4;
    
    d6 = d6 + v6;
    d7 = d7 * v7;
    d8 = d8 / (v8 + 1.0);
    
    /* Third call */
    unknown_effect3(d6, d7, v8);
    
    /* Final computation using all variables */
    double result = (v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10) * 
                   (d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10) *
                   (mix1 + mix2 + mix3 + mix4);
    
    return result;
}

/* Second high-pressure function with different pattern */
float high_pressure_call2(int *ints, float *floats, double *doubles) {
    /* Different mix of variables */
    int a1 = ints[0];
    int a2 = ints[1] * a1;
    int a3 = ints[2] + a2;
    int a4 = ints[3] - a3;
    int a5 = ints[4] * a4;
    int a6 = ints[5] / (a5 ? a5 : 1);
    
    float f1 = floats[0];
    float f2 = floats[1] * f1;
    float f3 = floats[2] + f2;
    float f4 = floats[3] - f3;
    float f5 = floats[4] * f4;
    float f6 = floats[5] / (f5 + 1.0f);
    
    double b1 = doubles[0];
    double b2 = doubles[1] * b1;
    double b3 = doubles[2] + b2;
    double b4 = doubles[3] - b3;
    double b5 = doubles[4] * b4;
    double b6 = doubles[5] / (b5 + 1.0);
    
    /* Complex computation mixing types */
    double t1 = a1 * f1 + b1;
    float t2 = a2 * b2 + f2;
    int t3 = (int)(f3 * b3) + a3;
    double t4 = a4 / (f4 + 1.0) * b4;
    
    /* Call in loop - creates multiple insertion points */
    for (int i = 0; i < 3; i++) {
        unknown_effect(a1 + i, b1);
        a1 = a1 * 2;
        b1 = b1 / 2.0;
        
        unknown_effect2(f1, a2, b2);
        f1 = f1 + 1.0f;
        a2 = a2 - 1;
        
        unknown_effect3(b3, b4, a3);
        b3 = b3 * 1.5;
        a3 = a3 >> 1;
    }
    
    /* Use all variables after loop */
    a4 = a4 + a1 + a2 + a3;
    a5 = a5 * a4;
    a6 = a6 ^ a5;
    
    f4 = f4 + f1 + f2 + f3;
    f5 = f5 * f4;
    f6 = f6 / (f5 + 1.0f);
    
    b4 = b4 + b1 + b2 + b3;
    b5 = b5 * b4;
    b6 = b6 / (b5 + 1.0);
    
    t1 = t1 * 2.0;
    t2 = t2 + 1.0f;
    t3 = t3 * 3;
    t4 = t4 / 4.0;
    
    /* Conditional call site */
    if (a4 > 0) {
        unknown_effect(a4, b4);
    } else {
        unknown_effect2(f4, a5, b5);
    }
    
    /* Switch with multiple call sites */
    switch (a6 & 3) {
        case 0:
            unknown_effect(a6, b6);
            break;
        case 1:
            unknown_effect2(f6, a6, t1);
            break;
        case 2:
            unknown_effect3(t1, t4, t3);
            break;
        default:
            unknown_effect(t3, t4);
            break;
    }
    
    return (f1 + f2 + f3 + f4 + f5 + f6) * (a1 + a2 + a3 + a4 + a5 + a6);
}

/* Third function with nested loops */
void high_pressure_call3(int iterations) {
    /* Many local variables */
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0, acc4 = 0.0, acc5 = 0.0;
    float facc1 = 0.0f, facc2 = 0.0f, facc3 = 0.0f, facc4 = 0.0f;
    int iacc1 = 0, iacc2 = 0, iacc3 = 0, iacc4 = 0, iacc5 = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Update accumulators - all live across call */
        acc1 += sin(i * 0.1);
        acc2 += cos(i * 0.2);
        acc3 += tan(acc1);
        acc4 += exp(acc2);
        acc5 += log(fabs(acc3) + 1.0);
        
        facc1 += acc1 * 0.5f;
        facc2 += acc2 * 0.3f;
        facc3 += acc3 * 0.7f;
        facc4 += acc4 * 0.9f;
        
        iacc1 += (int)acc1;
        iacc2 += (int)acc2;
        iacc3 += (int)acc3;
        iacc4 += (int)acc4;
        iacc5 += (int)acc5;
        
        /* Call with many live values - high register pressure */
        if (i % 2 == 0) {
            unknown_effect(iacc1, acc1);
        } else {
            unknown_effect2(facc1, iacc2, acc2);
        }
        
        /* More computations keeping variables live */
        acc1 = acc1 * 1.01;
        acc2 = acc2 / 1.01;
        acc3 = acc3 + 0.5;
        acc4 = acc4 - 0.3;
        acc5 = acc5 * 0.99;
        
        /* Another call in the loop */
        if (i % 3 == 0) {
            unknown_effect3(acc3, acc4, iacc3);
        }
    }
    
    /* Final call with all accumulators */
    unknown_effect(iacc1 + iacc2 + iacc3 + iacc4 + iacc5, 
                   acc1 + acc2 + acc3 + acc4 + acc5);
}

int main() {
    /* Initialize test data */
    int int_data[20];
    double double_data[20];
    float float_data[20];
    
    for (int i = 0; i < 20; i++) {
        int_data[i] = rand() % 100;
        double_data[i] = (rand() % 100) / 10.0;
        float_data[i] = (rand() % 100) / 5.0f;
    }
    
    double total = 0.0;
    
    /* Loop with calls to trigger caller-save insertion */
    for (int i = 0; i < 100; i++) {
        /* Modify inputs slightly each iteration */
        int_data[0] = i;
        double_data[0] = i * 0.5;
        
        /* Call first high-pressure function */
        total += high_pressure_call(int_data, double_data);
        
        /* Call second high-pressure function every 10 iterations */
        if (i % 10 == 0) {
            total += high_pressure_call2(int_data, float_data, double_data);
        }
        
        /* Call third function every 20 iterations */
        if (i % 20 == 0) {
            high_pressure_call3(50);
        }
    }
    
    printf("Result: %f\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
