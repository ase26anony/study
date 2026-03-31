/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Complex computation to prevent optimization */
    volatile int barrier = 0;
    asm volatile("" : "+r"(barrier) : : "memory");
    int result = (a * b) + (c / (d ? d : 1)) - (e * f) + (g ^ h);
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    /* Declare many local variables of mixed types */
    int v1, v2, v3, v4, v5, v6, v7, v8;
    float f1, f2, f3, f4;
    double d1, d2;
    long l1, l2;
    
    /* Force specific register usage for some variables */
    register int forced_reg1 asm("r12") = a + b;
    register int forced_reg2 asm("r13") = c * d;
    
    /* Complex pre-call computations creating dependencies */
    v1 = a * b + c;
    v2 = d - e / (f ? f : 1);
    v3 = g ^ h;
    v4 = i | j;
    
    f1 = (float)v1 * 1.5f;
    f2 = (float)v2 / 2.0f;
    f3 = f1 + f2;
    f4 = f3 * 3.14f;
    
    d1 = (double)v3 * 2.71828;
    d2 = (double)v4 / 1.41421;
    
    l1 = (long)v1 * v2;
    l2 = (long)v3 * v4;
    
    v5 = (int)f1 + (int)f2;
    v6 = (int)d1 + (int)d2;
    v7 = (int)l1 + (int)l2;
    v8 = forced_reg1 + forced_reg2;
    
    /* Memory barrier to force all variables live across call */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - some will go on stack on x86-64 */
    int call_result = helper_function(v1, v2, v3, v4, v5, v6, v7, v8);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    int result1 = call_result + v1 - v2 + v3 * v4;
    float result2 = f1 + f2 - f3 + f4;
    double result3 = d1 * d2 + (double)call_result;
    long result4 = l1 / (l2 ? l2 : 1) + (long)call_result;
    
    /* More computations to ensure basic block continues after call */
    int final1 = result1 + (int)result2;
    int final2 = (int)result3 + (int)result4;
    int final3 = forced_reg1 - forced_reg2;
    
    /* Use all results to prevent elimination */
    return final1 + final2 + final3 + v5 + v6 + v7 + v8;
}

/* Another helper to increase register pressure */
double __attribute__((noinline, noclone))
helper_double(double a, double b, double c, double d, 
              float e, float f, int g, int h) {
    volatile double barrier = 0.0;
    asm volatile("" : "+r"(barrier) : : "memory");
    double result = (a * b) + (c / d) + (double)(e * f) + (double)(g * h);
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

/* Function with mixed-type arguments and returns */
int __attribute__((noinline))
mixed_function(int a, float b, double c, long d, int e, float f, double g, long h) {
    /* Many live variables */
    int i1 = a * 2;
    float f1 = b * 3.14f;
    double d1 = c * 2.71828;
    long l1 = d + 1000;
    
    int i2 = e / 2;
    float f2 = f * 1.414f;
    double d2 = g / 3.14159;
    long l2 = h - 500;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call with mixed arguments */
    double call_result = helper_double(d1, d2, (double)f1, (double)f2, 
                                       b, f, i1, i2);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Post-call computations */
    int result_i = i1 + i2 + (int)call_result;
    float result_f = f1 + f2 + (float)call_result;
    double result_d = d1 + d2 + call_result;
    long result_l = l1 + l2 + (long)call_result;
    
    /* Use all results */
    return result_i + (int)result_f + (int)result_d + (int)result_l;
}

int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Repeatedly call target functions with different inputs */
    for (int i = 0; i < 100000; i++) {
        /* Generate random inputs */
        int a = rand() % 100 + 1;
        int b = rand() % 100 + 1;
        int c = rand() % 100 + 1;
        int d = rand() % 100 + 1;
        int e = rand() % 100 + 1;
        int f = rand() % 100 + 1;
        int g = rand() % 100 + 1;
        int h = rand() % 100 + 1;
        int i = rand() % 100 + 1;
        int j = rand() % 100 + 1;
        
        /* Call both functions to increase coverage chances */
        total += target_function(a, b, c, d, e, f, g, h, i, j);
        
        /* Call mixed function with different types */
        total += mixed_function(a, (float)b, (double)c, (long)d,
                                e, (float)f, (double)g, (long)h);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", total);
    
    return 0;
}
