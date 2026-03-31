/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Force side effects to prevent optimization */
    asm volatile("" : : : "memory");
    return (a * b) + (c - d) + (e / (f ? f : 1)) + (g * h);
}

/* Another helper to force register usage */
double __attribute__((noinline, noclone))
helper_double(double a, double b, double c, double d) {
    asm volatile("" : : : "memory");
    return a * b + c / d;
}

/* Target function with high register pressure across a call */
long __attribute__((noinline))
target_function(int a, int b, int c, int d, 
                float e, float f, float g, float h,
                double i, double j, double k, double l) {
    
    /* Declare many local variables with mixed types */
    int t1, t2, t3, t4, t5, t6;
    float f1, f2, f3, f4;
    double d1, d2, d3, d4;
    long result = 0;
    
    /* Use register suggestion for specific variables */
    register int reg_var1 asm("r12") = a * 2;
    register int reg_var2 asm("r13") = b + 3;
    
    /* Complex pre-call computations creating web of dependencies */
    t1 = a * b + c;
    t2 = d - c + b;
    t3 = t1 * t2 / (a ? a : 1);
    t4 = reg_var1 + reg_var2 * 2;
    
    f1 = e * f + g;
    f2 = h / (f ? f : 1.0f) + e;
    f3 = f1 * f2 - g;
    f4 = h + f1 / f2;
    
    d1 = i * j + k;
    d2 = l / (j ? j : 1.0) + i;
    d3 = d1 * d2 - k;
    d4 = l + d1 / d2;
    
    /* Make all variables appear live across the call */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - some will go on stack on x86-64 */
    int helper_result = helper_function(t1, t2, t3, t4, 
                                       (int)f1, (int)f2, 
                                       (int)d1, (int)d2);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Post-call computations using all variables */
    t5 = helper_result + t1 * t2;
    t6 = t3 + t4 - helper_result;
    
    float f5 = f3 * f4 + (float)helper_result;
    float f6 = f1 / f2 + f3 - f4;
    
    double d5 = d3 * d4 + helper_double(d1, d2, d3, d4);
    double d6 = d1 / d2 + d3 - d4;
    
    /* Use the register-suggested variables */
    result = (long)t5 * t6 + (long)f5 * (long)f6 + (long)d5 * (long)d6;
    result += reg_var1 * reg_var2;
    
    /* More computations to ensure call isn't at block end */
    result = result * 2 - 1;
    result = result + (long)(f1 * f2 * f3 * f4);
    result = result ^ (long)(d1 * d2 * d3 * d4);
    
    return result;
}

/* Test driver */
int main() {
    srand(time(NULL));
    long total = 0;
    const int iterations = 100000;
    
    for (int i = 0; i < iterations; i++) {
        /* Generate random inputs */
        int a = rand() % 100 + 1;
        int b = rand() % 100 + 1;
        int c = rand() % 100 + 1;
        int d = rand() % 100 + 1;
        
        float e = (float)(rand() % 100) / 10.0f;
        float f = (float)(rand() % 100) / 10.0f;
        float g = (float)(rand() % 100) / 10.0f;
        float h = (float)(rand() % 100) / 10.0f;
        
        double i = (double)(rand() % 100) / 10.0;
        double j = (double)(rand() % 100) / 10.0;
        double k = (double)(rand() % 100) / 10.0;
        double l = (double)(rand() % 100) / 10.0;
        
        /* Call target function repeatedly */
        total += target_function(a, b, c, d, e, f, g, h, i, j, k, l);
        
        /* Prevent loop optimization */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %ld\n", total);
    return 0;
}
