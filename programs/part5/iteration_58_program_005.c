/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, double c, float d, long e, int f, double g, float h) {
    /* Force side effects */
    volatile int result = 0;
    result += a * b;
    result += (int)(c * d);
    result += e % 100;
    result += f;
    result += (int)(g * h * 100);
    
    /* Memory barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Declare many local variables of mixed types */
    int var1 = a * 2;
    int var2 = b + 3;
    double var3 = c * 1.5;
    float var4 = d / 2.0f;
    long var5 = e * 100L;
    int var6 = f - 10;
    double var7 = g * 3.14159;
    float var8 = h * 0.5f;
    int var9 = a + b + c;
    long var10 = d * e * f;
    
    /* Register suggestion for specific register (x86-64: r12 is call-clobbered) */
    register int forced_reg asm("r12") = var1 * var2;
    
    /* Complex pre-call computations creating web of dependencies */
    double t1 = var3 * var4 + var7;
    float t2 = var4 - var8 / var3;
    long t3 = var5 + var10 * 2;
    int t4 = var1 * var6 - var2;
    double t5 = var7 * 2.0 + var3;
    float t6 = var8 + var4 * 3.0f;
    
    /* Memory barrier before call - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Function call with many arguments - some will go on stack on x86-64 */
    int helper_result = helper_function(
        var1, var2, t1, t2, t3, t4, t5, t6
    );
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Post-call computations using all variables - keeping them live */
    int result1 = helper_result + forced_reg;
    double result2 = var3 + var7 - t1;
    float result3 = var4 * var8 + t2;
    long result4 = var5 + var10 + t3;
    int result5 = var6 * var9 + t4;
    double result6 = t5 * 2.0;
    float result7 = t6 * 1.5f;
    
    /* More complex dependent operations */
    int final_result = result1 + (int)result2 + (int)result3 + 
                      (int)(result4 % 1000) + result5 + 
                      (int)result6 + (int)result7;
    
    /* Use all variables one more time to ensure liveness */
    final_result += var1 + var2 + (int)var3 + (int)var4 + 
                   (int)(var5 % 100) + var6 + (int)var7 + 
                   (int)var8 + var9 + (int)(var10 % 100);
    
    return final_result;
}

/* Another helper to increase complexity */
double __attribute__((noinline, noclone))
second_helper(double a, double b, double c, double d, double e, double f) {
    volatile double result = a + b - c * d / e + f;
    asm volatile("" : : : "memory");
    return result;
}

/* Function with multiple calls in same basic block */
int __attribute__((noinline))
multi_call_function(int a, int b, int c, int d) {
    int x1 = a * b;
    int x2 = c + d;
    double x3 = a * 1.234;
    double x4 = b * 5.678;
    float x5 = c * 0.123f;
    float x6 = d * 0.456f;
    long x7 = a * b * c;
    long x8 = d * 1000L;
    
    /* First call */
    asm volatile("" : : : "memory");
    int res1 = helper_function(x1, x2, x3, x4, x7, x8, x3 + x4, x5 + x6);
    asm volatile("" : : : "memory");
    
    /* Straight-line code between calls (no jumps) */
    double mid1 = x3 * x4 + res1;
    float mid2 = x5 - x6 * res1;
    
    /* Second call in same basic block */
    asm volatile("" : : : "memory");
    double res2 = second_helper(mid1, mid2, x3, x4, x5, x6);
    asm volatile("" : : : "memory");
    
    /* More computations after second call */
    int final = res1 + (int)res2 + x1 + x2 + (int)(x7 % 100) + (int)(x8 % 100);
    
    return final;
}

int main() {
    srand(time(NULL));
    int total_result = 0;
    
    /* Create varied inputs */
    int inputs[100];
    for (int i = 0; i < 100; i++) {
        inputs[i] = rand() % 1000;
    }
    
    /* Repeatedly exercise the target functions */
    for (int i = 0; i < 50; i++) {
        /* Call target_function with many live values */
        total_result += target_function(
            inputs[i % 100],
            inputs[(i + 1) % 100],
            inputs[(i + 2) % 100],
            inputs[(i + 3) % 100],
            inputs[(i + 4) % 100],
            inputs[(i + 5) % 100],
            inputs[(i + 6) % 100],
            inputs[(i + 7) % 100]
        );
        
        /* Also call multi_call_function */
        total_result += multi_call_function(
            inputs[(i + 8) % 100],
            inputs[(i + 9) % 100],
            inputs[(i + 10) % 100],
            inputs[(i + 11) % 100]
        );
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    
    return 0;
}
