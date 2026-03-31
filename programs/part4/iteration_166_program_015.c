/* caller-save-test.c - Main test program to force caller-save insertion */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions with side effects to prevent optimization */
extern void unknown_effect1(int, double);
extern void unknown_effect2(float, int, double);
extern void unknown_effect3(double, double, int);
extern int unknown_computation(int, double, float);

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Non-inlinable function with many live values across call */
__attribute__((noinline)) 
int high_pressure_function1(int a, int b, int c, int d, int e, 
                           double f, double g, double h, float i, float j) {
    /* Create many intermediate values that must stay in registers */
    int v1 = a * b + c;
    int v2 = d ^ e | c;
    int v3 = (a << 2) + (b >> 1);
    int v4 = v1 * v2 - v3;
    
    double v5 = f * g + h;
    double v6 = sin(f) * cos(g);
    double v7 = v5 * 2.5 - v6;
    
    float v8 = i * j + 3.14f;
    float v9 = v8 * 2.0f - i / j;
    
    /* Mix integer and floating point computations */
    int v10 = (int)(v5 * 100.0) + v4;
    double v11 = (double)v2 / (double)v1 + v7;
    float v12 = (float)v3 * 0.5f + v9;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Function call that clobbers call-used registers */
    unknown_effect1(v1, v5);
    
    /* More computations using values that were live across the call */
    int v13 = v4 * 2 + v10;
    double v14 = v11 * v7 - v5;
    float v15 = v12 + v8 * v9;
    
    /* Another call with different arguments */
    asm volatile("" : : : "memory");
    unknown_effect2(v15, v13, v14);
    
    /* Complex conditional to create control flow */
    if (v13 > 1000) {
        v14 = sqrt(v14) + v11;
        v15 = v15 * 2.0f;
    } else {
        v14 = log(fabs(v14) + 1.0);
        v15 = v15 / 2.0f;
    }
    
    /* Final computation and return */
    int result = (int)(v14 * 1000.0) + v13 + (int)v15;
    asm volatile("" : : : "memory");
    return result;
}

/* Second high-pressure function with different register usage pattern */
__attribute__((noinline))
double high_pressure_function2(int x1, int x2, int x3, int x4, int x5,
                              int x6, int x7, int x8, int x9, int x10,
                              double y1, double y2, double y3, double y4, double y5) {
    /* Even more variables to increase register pressure */
    int sum_int = 0;
    double sum_double = 0.0;
    
    /* Loop inside function to create multiple basic blocks */
    for (int i = 0; i < 3; i++) {
        int t1 = x1 + x2 * i;
        int t2 = x3 ^ x4 | x5;
        int t3 = x6 * x7 - x8;
        int t4 = x9 << i;
        int t5 = x10 >> (i + 1);
        
        double d1 = y1 * i + y2;
        double d2 = sin(y3) * cos(y4);
        double d3 = y5 * exp(-i * 0.1);
        
        /* All these values must be preserved across the call */
        asm volatile("" : : : "memory");
        unknown_effect3(d1, d2, t1);
        
        /* Use the values after the call */
        sum_int += t1 + t2 + t3 + t4 + t5;
        sum_double += d1 * d2 + d3;
        
        /* Conditional call inside loop */
        if (i % 2 == 0) {
            asm volatile("" : : : "memory");
            unknown_effect1(t2, d3);
        }
    }
    
    /* Switch statement to create complex control flow */
    switch (sum_int % 4) {
        case 0:
            sum_double = sum_double * 2.0;
            break;
        case 1:
            sum_double = sqrt(sum_double);
            break;
        case 2:
            sum_double = log(sum_double + 1.0);
            break;
        case 3:
            sum_double = sin(sum_double);
            break;
    }
    
    asm volatile("" : : : "memory");
    return sum_double;
}

/* Third function with mixed types and nested loops */
__attribute__((noinline))
float high_pressure_function3(float a, float b, float c, float d,
                             int e, int f, int g, int h,
                             double i, double j, double k) {
    float acc_f = 0.0f;
    double acc_d = 0.0;
    int acc_i = 0;
    
    /* Nested loops to increase register pressure */
    for (int outer = 0; outer < 2; outer++) {
        float f1 = a * outer + b;
        float f2 = c / (outer + 1) + d;
        int i1 = e + f * outer;
        int i2 = g ^ h;
        double d1 = i * sin(j * outer);
        double d2 = k * cos(j);
        
        for (int inner = 0; inner < 2; inner++) {
            /* Many live values across this call */
            float f3 = f1 * inner + f2;
            int i3 = i1 * i2 + inner;
            double d3 = d1 * d2 + inner * 0.5;
            
            /* Call with many arguments - will use multiple call-used regs */
            asm volatile("" : : : "memory");
            int comp = unknown_computation(i3, d3, f3);
            
            /* Use results */
            acc_f += f3 * comp;
            acc_d += d3 * comp;
            acc_i += i3 + comp;
            
            /* Another call in the inner loop */
            if (inner == 0) {
                asm volatile("" : : : "memory");
                unknown_effect2(f3, i3, d3);
            }
        }
    }
    
    asm volatile("" : : : "memory");
    return acc_f + (float)acc_d + (float)acc_i;
}

/* Main function with loop calling high-pressure functions */
int main() {
    int result1 = 0;
    double result2 = 0.0;
    float result3 = 0.0f;
    
    /* Loop to ensure caller-save logic is triggered multiple times */
    for (int iter = 0; iter < 100; iter++) {
        /* Call first high-pressure function */
        int r1 = high_pressure_function1(
            iter, iter+1, iter+2, iter+3, iter+4,
            1.1 * iter, 2.2 * iter, 3.3 * iter,
            4.4f * iter, 5.5f * iter
        );
        result1 += r1;
        
        /* Call second high-pressure function */
        double r2 = high_pressure_function2(
            iter, iter*2, iter*3, iter*4, iter*5,
            iter*6, iter*7, iter*8, iter*9, iter*10,
            1.5 * iter, 2.5 * iter, 3.5 * iter, 4.5 * iter, 5.5 * iter
        );
        result2 += r2;
        
        /* Call third high-pressure function */
        float r3 = high_pressure_function3(
            1.1f * iter, 2.2f * iter, 3.3f * iter, 4.4f * iter,
            iter, iter+10, iter+20, iter+30,
            5.5 * iter, 6.6 * iter, 7.7 * iter
        );
        result3 += r3;
        
        /* Update global to prevent optimization */
        global_counter++;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %f, %f\n", result1, result2, result3);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
