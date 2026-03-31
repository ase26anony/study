/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Complex computation to prevent optimization */
    volatile int result = 0;
    result = a * b + c * d - e * f + g / (h ? h : 1);
    asm volatile("" : : : "memory");  /* Memory barrier */
    return result;
}

/* Another helper to increase register pressure */
double __attribute__((noinline, noclone))
helper_double(double a, double b, double c, double d) {
    volatile double result = a * b + c / d;
    asm volatile("" : : : "memory");
    return result;
}

/* The target function with high register pressure across a call */
int __attribute__((noinline, noclone))
target_function(int p1, int p2, double p3, float p4, long p5) {
    /* Declare many local variables of mixed types */
    int a = p1 + 1;
    int b = p2 * 2;
    double c = p3 * 3.14;
    float d = p4 * 2.0f;
    long e = p5 + 1000;
    int f = a * b;
    double g = c + 1.618;
    float h = d * 3.0f;
    long i = e - 500;
    int j = b / (a ? a : 1);
    double k = g * 2.0;
    float l = h + 1.0f;
    
    /* Use register suggestion for specific variable */
    register int r12_var asm("r12") = a * 2 + b;
    
    /* Complex pre-call computations creating dependencies */
    int t1 = a * b + c + d + e;
    double t2 = c * d - e / (f ? f : 1) + g;
    float t3 = d * e + f * g + h;
    long t4 = e * f - g + h * i;
    int t5 = f * g + h - i * j;
    double t6 = g * h + i - j * k;
    
    /* Memory barrier to force all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - forces some to stack on x86-64 */
    int helper_result = helper_function(
        t1, t2, t3, t4, t5, t6, 
        r12_var, j  /* 8 arguments - more than 6 for x86-64 */
    );
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    /* This ensures the call is not at block end */
    double double_result = helper_double(c, g, k, 2.0);
    float float_combo = d + h + l + helper_result;
    int int_combo = a + b + f + j + helper_result;
    long long_combo = e + i + t4 + helper_result;
    
    /* More arithmetic to extend basic block after call */
    int combo1 = int_combo * 2 - float_combo;
    double combo2 = double_result * 3.14159 / (combo1 ? combo1 : 1);
    long combo3 = long_combo + combo1 + combo2;
    
    /* Use all variables in final computation */
    volatile int final_result = 0;
    final_result = a + b + c + d + e + f + g + h + i + j + k + l +
                   t1 + t2 + t3 + t4 + t5 + t6 +
                   helper_result + combo1 + combo2 + combo3 +
                   r12_var;
    
    return final_result;
}

/* Test driver */
int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Repeated calls with different inputs */
    for (int i = 0; i < 1000; i++) {
        int p1 = rand() % 100;
        int p2 = rand() % 100;
        double p3 = (rand() % 100) / 10.0;
        float p4 = (rand() % 100) / 10.0f;
        long p5 = rand() % 1000;
        
        total += target_function(p1, p2, p3, p4, p5);
        
        /* Prevent loop optimization */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return total > 0 ? 0 : 1;
}
