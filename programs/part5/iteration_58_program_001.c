/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = 0;
    /* Force side effects and prevent optimization */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
    result = a * b + c * d - e * f + g * h;
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    return result;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int p1, int p2, int p3, int p4, 
                float p5, double p6, long p7, int p8) {
    /* Declare many local variables of mixed types */
    int a = p1 * 2;
    int b = p2 + 3;
    int c = p3 - 4;
    int d = p4 / 2;
    float e = p5 * 1.5f;
    double f = p6 * 2.0;
    long g = p7 + 1000;
    int h = p8 * 3;
    int i = a + b;
    int j = c - d;
    
    /* Force specific register usage for some variables */
    register int forced_reg1 asm("r12") = a * b;
    register int forced_reg2 asm("r13") = c + d;
    
    /* Complex pre-call computations creating dependencies */
    int t1 = a * b + c * d - forced_reg1;
    float t2 = e * 2.0f + (float)b;
    double t3 = f * 3.0 + (double)c;
    long t4 = g * 2L + (long)d;
    int t5 = h * i - j;
    int t6 = forced_reg1 * forced_reg2;
    
    /* Memory barrier making all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - some will go on stack on x86-64 */
    int call_result = helper_function(t1, t2, t3, t4, t5, t6, forced_reg1, forced_reg2);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Post-call computations using all variables */
    int u1 = call_result + a + b;
    float u2 = (float)call_result * e;
    double u3 = (double)call_result / f;
    long u4 = (long)call_result * g;
    int u5 = h * call_result - i;
    int u6 = j * call_result + forced_reg1;
    int u7 = forced_reg2 * call_result / 2;
    
    /* More complex expressions to maintain live ranges */
    int final1 = u1 + u5 + u6 + u7;
    float final2 = u2 + (float)u1;
    double final3 = u3 + (double)u5;
    long final4 = u4 + (long)u6;
    
    /* Final computation using all results */
    int final_result = final1 + (int)final2 + (int)final3 + (int)final4;
    
    return final_result;
}

/* Main function to repeatedly exercise the target */
int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Run many iterations to ensure compiler sees hot code */
    for (int iter = 0; iter < 100000; iter++) {
        /* Generate random inputs */
        int p1 = rand() % 100;
        int p2 = rand() % 100;
        int p3 = rand() % 100;
        int p4 = rand() % 100;
        float p5 = (float)(rand() % 100) / 10.0f;
        double p6 = (double)(rand() % 100) / 10.0;
        long p7 = rand() % 100;
        int p8 = rand() % 100;
        
        /* Call target function */
        int result = target_function(p1, p2, p3, p4, p5, p6, p7, p8);
        
        /* Accumulate result to prevent dead code elimination */
        total += result;
        
        /* Occasionally print to prevent loop optimization */
        if (iter % 10000 == 0) {
            printf("Iteration %d: result = %d, total = %d\n", iter, result, total);
        }
    }
    
    printf("Final total: %d\n", total);
    return total > 0 ? 0 : 1;
}
