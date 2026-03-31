/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Complex computation to prevent optimization */
    volatile int result = 0;
    result = a * b + c * d - e * f + g * h;
    result = result ^ (a + b) ^ (c + d) ^ (e + f) ^ (g + h);
    return result;
}

/* Another helper to force register pressure */
double __attribute__((noinline, noclone))
helper_double(double a, double b, double c, double d) {
    volatile double result = 0.0;
    result = a * b + c / d;
    result = result * 2.0 - 1.0;
    return result;
}

/* The target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int p1, int p2, int p3, int p4, 
                double p5, double p6, float p7, float p8) {
    
    /* Declare many local variables of mixed types */
    int a = p1 * 2;
    int b = p2 + 3;
    int c = p3 - 4;
    int d = p4 / 2;
    long e = (long)p1 * p2;
    long f = (long)p3 * p4;
    float g = p7 * 2.0f;
    float h = p8 / 3.0f;
    double i = p5 + 1.0;
    double j = p6 - 2.0;
    
    /* Register variable to force specific register usage */
    register int forced_reg asm("r12") = a + b + c + d;
    
    /* Complex pre-call computations creating web of dependencies */
    int t1 = a * b + c - d;
    int t2 = (e % 100) + (f % 50);
    float t3 = g * h + p7;
    double t4 = i * j - p5;
    long t5 = e * f / 1000;
    
    /* Use all variables to keep them live */
    forced_reg += t1 + t2 + t3 + t4 + t5;
    
    /* Memory barrier to prevent optimizations */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - forces register/stack pressure */
    int helper_result = helper_function(t1, t2, forced_reg, a, b, c, d, t1 ^ t2);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Post-call computations using all variables */
    double double_result = helper_double(i, j, p5, p6);
    
    /* More complex computations keeping all variables live */
    int final_int = helper_result + a - b + c * d;
    final_int += (int)(g * 100.0f) + (int)(h * 50.0f);
    final_int += (int)(double_result * 10.0);
    final_int += (int)(t4 / 2.0);
    final_int += forced_reg;
    
    /* Use all remaining variables */
    long final_long = e + f + t5;
    float final_float = g + h + t3 + p7 + p8;
    double final_double = i + j + p5 + p6 + double_result + t4;
    
    /* Combine everything */
    int result = final_int + (int)final_long + (int)final_float + (int)final_double;
    
    return result;
}

/* Test driver */
int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Run many iterations to ensure the function is compiled and executed */
    for (int iter = 0; iter < 100000; iter++) {
        /* Generate random inputs */
        int p1 = rand() % 100;
        int p2 = rand() % 100;
        int p3 = rand() % 100;
        int p4 = rand() % 100;
        double p5 = (double)(rand() % 100) / 10.0;
        double p6 = (double)(rand() % 100) / 10.0;
        float p7 = (float)(rand() % 100) / 10.0f;
        float p8 = (float)(rand() % 100) / 10.0f;
        
        /* Call the target function */
        int result = target_function(p1, p2, p3, p4, p5, p6, p7, p8);
        
        /* Accumulate to prevent dead code elimination */
        total += result;
        
        /* Occasionally print to prevent optimization */
        if (iter % 10000 == 0) {
            printf("Iteration %d: result = %d\n", iter, result);
        }
    }
    
    printf("Final total: %d\n", total);
    return 0;
}
