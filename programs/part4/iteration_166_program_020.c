/* caller-save-test.c - Test program to trigger caller-save insertion logic */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int, double) __attribute__((noinline));
extern void unknown_effect2(float, int, double) __attribute__((noinline));
extern void unknown_effect3(void) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Function 1: High register pressure with mixed types */
double high_pressure_function1(int iterations) {
    /* Declare many variables to create register pressure */
    int v1 = iterations * 2;
    int v2 = iterations + 100;
    int v3 = iterations - 50;
    int v4 = iterations * 3;
    int v5 = iterations / 2;
    
    double d1 = iterations * 1.5;
    double d2 = iterations * 2.5;
    double d3 = iterations * 3.5;
    double d4 = iterations * 4.5;
    double d5 = iterations * 5.5;
    
    float f1 = iterations * 0.1f;
    float f2 = iterations * 0.2f;
    float f3 = iterations * 0.3f;
    
    /* Use all variables in computations before call */
    v1 = v1 * v2 + v3;
    v2 = v2 - v4 * v5;
    v3 = v3 + v1 / (v5 + 1);
    
    d1 = d1 * d2 + sin(d3);
    d2 = d2 - d4 * cos(d5);
    d3 = d3 + d1 / (d5 + 1.0);
    
    f1 = f1 * f2 + sinf(f3);
    f2 = f2 - f1 * 0.5f;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call - clobbers call-used registers */
    unknown_effect(v1 + v2, d1 + d2);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use variables after call - they must be restored */
    v4 = v1 * v3 + v2 * v5;
    v5 = v2 - v3 + v4;
    
    d4 = d1 * d3 + d2 * d5;
    d5 = d2 - d3 + d4;
    
    f3 = f1 * 2.0f + f2 * 3.0f;
    
    /* More computations mixing types */
    double result = (v1 + v2 + v3 + v4 + v5) * 
                   (d1 + d2 + d3 + d4 + d5) * 
                   (f1 + f2 + f3);
    
    /* Another call with different arguments */
    unknown_effect2(f3, v4, d5);
    
    return result;
}

/* Function 2: Different pattern with nested control flow */
int high_pressure_function2(int base) {
    /* Even more variables */
    int a1 = base, a2 = base + 1, a3 = base + 2, a4 = base + 3;
    int a5 = base + 4, a6 = base + 5, a7 = base + 6, a8 = base + 7;
    
    double b1 = base * 0.1, b2 = base * 0.2, b3 = base * 0.3, b4 = base * 0.4;
    double b5 = base * 0.5, b6 = base * 0.6, b7 = base * 0.7, b8 = base * 0.8;
    
    /* Complex conditional with calls inside */
    int result = 0;
    for (int i = 0; i < 100; i++) {
        /* Update variables in loop */
        a1 = a1 * a2 + i;
        a2 = a2 - a3 * a4;
        a3 = a3 + a1 / (a5 + 1);
        a4 = a4 * a6 - a7;
        
        b1 = b1 * b2 + sin(b3 + i);
        b2 = b2 - b4 * cos(b5);
        b3 = b3 + b1 / (b6 + 1.0);
        b4 = b4 * b7 - b8;
        
        /* Call inside loop - high pressure */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
            unknown_effect(a1 + a3, b1 + b3);
            asm volatile("" : : : "memory");
            
            a5 = a1 * a3 + a2 * a4;
            b5 = b1 * b3 + b2 * b4;
        } else if (i % 3 == 1) {
            asm volatile("" : : : "memory");
            unknown_effect2(b1, a2, b3);
            asm volatile("" : : : "memory");
            
            a6 = a2 * a4 - a3 * a5;
            b6 = b2 * b4 - b3 * b5;
        } else {
            asm volatile("" : : : "memory");
            unknown_effect3();
            asm volatile("" : : : "memory");
            
            a7 = a3 * a5 + a4 * a6;
            b7 = b3 * b5 + b4 * b6;
        }
        
        /* More computations */
        a8 = a1 + a2 + a3 + a4 + a5 + a6 + a7;
        b8 = b1 + b2 + b3 + b4 + b5 + b6 + b7;
        
        result += (int)(a8 * b8);
    }
    
    return result;
}

/* Function 3: Switch statement with calls */
double high_pressure_function3(int mode) {
    int x1 = mode * 10, x2 = mode * 20, x3 = mode * 30;
    int x4 = mode * 40, x5 = mode * 50, x6 = mode * 60;
    
    double y1 = mode * 0.01, y2 = mode * 0.02, y3 = mode * 0.03;
    double y4 = mode * 0.04, y5 = mode * 0.05, y6 = mode * 0.06;
    
    float z1 = mode * 0.001f, z2 = mode * 0.002f, z3 = mode * 0.003f;
    
    double total = 0.0;
    
    switch (mode % 4) {
        case 0:
            x1 = x1 * x2 + x3;
            y1 = y1 * y2 + sin(y3);
            asm volatile("" : : : "memory");
            unknown_effect(x1, y1);
            asm volatile("" : : : "memory");
            total = x1 * y1 * z1;
            break;
            
        case 1:
            x2 = x2 - x4 * x5;
            y2 = y2 - y4 * cos(y5);
            asm volatile("" : : : "memory");
            unknown_effect2(z2, x2, y2);
            asm volatile("" : : : "memory");
            total = x2 * y2 * z2;
            break;
            
        case 2:
            x3 = x3 + x1 / (x6 + 1);
            y3 = y3 + y1 / (y6 + 1.0);
            asm volatile("" : : : "memory");
            unknown_effect3();
            asm volatile("" : : : "memory");
            total = x3 * y3 * z3;
            break;
            
        case 3:
            x4 = x1 * x3 + x2 * x5;
            y4 = y1 * y3 + y2 * y5;
            z3 = z1 * 2.0f + z2 * 3.0f;
            asm volatile("" : : : "memory");
            unknown_effect(x4, y4);
            asm volatile("" : : : "memory");
            total = x4 * y4 * z3;
            break;
    }
    
    /* Use all variables after switch */
    x5 = x1 + x2 + x3 + x4;
    y5 = y1 + y2 + y3 + y4;
    z1 = z1 + z2 + z3;
    
    return total + x5 + y5 + z1;
}

int main(void) {
    double total = 0.0;
    int int_total = 0;
    
    printf("Starting caller-save test...\n");
    
    /* Loop to ensure multiple invocations */
    for (int i = 0; i < 100; i++) {
        /* Call all high-pressure functions */
        total += high_pressure_function1(i);
        int_total += high_pressure_function2(i);
        total += high_pressure_function3(i);
        
        /* Update global to prevent optimization */
        global_counter++;
    }
    
    printf("Results: total = %f, int_total = %d, global = %d\n", 
           total, int_total, global_counter);
    
    /* Use results to prevent dead code elimination */
    if (total > 0 && int_total > 0) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}
