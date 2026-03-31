/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Complex computation to prevent simplification */
    volatile int result = 0;
    result += a * b;
    result -= c / (d ? d : 1);
    result += e * f;
    result ^= g << 2;
    result |= h;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another helper to increase register pressure */
double __attribute__((noinline, noclone))
helper_double(double a, double b, double c, float d, float e) {
    volatile double result = a;
    result += b * c;
    result -= (double)d * e;
    asm volatile("" : : : "memory");
    return result;
}

/* The target function with high register pressure across a call */
long __attribute__((noinline))
target_function(int p1, int p2, double p3, float p4, long p5) {
    /* Declare many local variables of mixed types */
    int a = p1 * 2;
    int b = p2 + 3;
    double c = p3 * 1.5;
    float d = p4 / 2.0f;
    long e = p5 - 100;
    int f = a ^ b;
    double g = c + 3.14159;
    float h = d * 2.0f;
    int i = b << 2;
    long j = e / 3;
    double k = g * 2.0;
    float l = h + 1.0f;
    
    /* Force specific register usage (x86-64 specific) */
    register int forced_reg asm("r12") = a * b + 12345;
    
    /* Complex pre-call computation creating web of dependencies */
    int t1 = a * b + f;
    double t2 = c - d / g;
    long t3 = e ^ j;
    float t4 = h * l - d;
    int t5 = i + forced_reg;
    
    /* Memory barrier before call - makes all vars appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - some will go on stack on x86-64 */
    int call_result = helper_function(t1, t2, t3, t4, t5, 
                                      a, b, forced_reg);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computation using all variables */
    double double_res = helper_double(c, g, k, h, l);
    
    long final_result = call_result;
    final_result += (long)(a * b);
    final_result -= (long)(c * 100.0);
    final_result ^= e;
    final_result |= j;
    final_result += (long)(double_res * 10.0);
    final_result += forced_reg;  /* Use forced register variable */
    
    /* More arithmetic to ensure straight-line code after call */
    int x1 = final_result & 0xFF;
    double x2 = double_res * x1;
    float x3 = (float)x2 / l;
    long x4 = final_result ^ (long)x3;
    
    /* Final computation - no early return */
    long result = x4 + (long)(x2 * 1000.0) - (long)(x3 * 100.0f);
    result += forced_reg * 2;  /* Keep forced_reg live */
    
    return result;
}

/* Test driver */
int main() {
    srand(time(NULL));
    long total = 0;
    const int iterations = 10000;
    
    for (int i = 0; i < iterations; i++) {
        /* Generate random inputs */
        int p1 = rand() % 1000;
        int p2 = rand() % 1000;
        double p3 = (double)(rand() % 1000) / 10.0;
        float p4 = (float)(rand() % 1000) / 10.0f;
        long p5 = rand() % 10000;
        
        /* Call target function repeatedly */
        long result = target_function(p1, p2, p3, p4, p5);
        
        /* Accumulate result to prevent dead code elimination */
        total += result;
        
        /* Occasionally call helper directly to keep it alive */
        if (i % 100 == 0) {
            total += helper_function(p1, p2, p1, p2, p1, p2, p1, p2);
        }
    }
    
    printf("Final result: %ld\n", total);
    return 0;
}
