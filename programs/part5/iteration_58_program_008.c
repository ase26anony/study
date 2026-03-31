/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = 0;
    /* Force side effects and prevent optimization */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e), 
                       "+r"(f), "+r"(g), "+r"(h), "+r"(i), "+r"(j));
    result = a + b - c + d - e + f - g + h - i + j;
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
high_register_pressure(int seed) {
    /* Declare many variables of mixed types to increase register pressure */
    int a = seed * 1;
    int b = seed * 2;
    int c = seed * 3;
    int d = seed * 4;
    int e = seed * 5;
    int f = seed * 6;
    int g = seed * 7;
    int h = seed * 8;
    int i = seed * 9;
    int j = seed * 10;
    
    /* Use register suggestion for specific variables */
    register int r12_var asm("r12") = seed * 11;
    register int r13_var asm("r13") = seed * 12;
    
    /* Floating point variables to use FP registers */
    float fa = seed * 1.5f;
    float fb = seed * 2.5f;
    double da = seed * 3.14159;
    double db = seed * 2.71828;
    
    /* Complex pre-call computations creating web of dependencies */
    int t1 = a * b + c - d;
    int t2 = e / (f + 1) + g;
    int t3 = h ^ i ^ j;
    float ft1 = fa * fb + (float)c;
    double dt1 = da / (db + 1.0);
    
    /* Use all variables before the call */
    t1 += r12_var;
    t2 -= r13_var;
    ft1 += (float)r12_var;
    dt1 *= (double)r13_var;
    
    /* Memory barrier to force all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - some will go on stack on x86-64 */
    int helper_result = helper_function(t1, t2, t3, a, b, c, d, e, f, g);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Post-call computations using all variables */
    int post1 = helper_result + h * i - j;
    int post2 = r12_var + r13_var + t1;
    float post3 = ft1 * 2.0f + (float)helper_result;
    double post4 = dt1 * 3.0 + (double)helper_result;
    
    /* More complex computations to ensure basic block continues after call */
    int result1 = post1 * post2 + a - b;
    float result2 = post3 * fa - fb;
    double result3 = post4 + da - db;
    
    /* Use all variables one more time */
    result1 += c + d - e + f - g + h - i + j;
    result2 += (float)(r12_var * r13_var);
    result3 += (double)(t2 * t3);
    
    /* Final computation that depends on everything */
    int final_result = result1 + (int)result2 + (int)result3;
    
    /* Ensure the call is not at the end of basic block */
    final_result = final_result * 2 - seed;
    
    return final_result;
}

/* Main function to repeatedly exercise the target */
int main() {
    srand(time(NULL));
    int total = 0;
    const int iterations = 100000;
    
    for (int k = 0; k < iterations; k++) {
        /* Use different inputs each iteration */
        int input = rand() % 100 + 1;
        
        /* Call the high-pressure function */
        int result = high_register_pressure(input);
        
        /* Accumulate to prevent dead code elimination */
        total += result;
        
        /* Occasionally print to prevent optimization */
        if (k % 10000 == 0) {
            printf("Iteration %d: result = %d\n", k, result);
        }
    }
    
    printf("Final total: %d\n", total);
    return total > 0 ? 0 : 1;
}
