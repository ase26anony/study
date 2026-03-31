/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = 0;
    /* Force side effects */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e));
    result = a + b * c - d / (e + 1) + f - g * h + i - j;
    return result;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int p1, int p2, int p3, int p4, int p5, 
                float p6, double p7, long p8) {
    /* Declare many local variables of mixed types */
    int a = p1 * 2;
    int b = p2 + 3;
    int c = p3 - 4;
    int d = p4 / 5;
    int e = p5 % 6;
    float f = p6 * 2.5f;
    double g = p7 / 3.14;
    long h = p8 + 1000;
    
    /* Additional variables to increase pressure */
    int i = a + b;
    int j = c - d;
    float k = f + 1.0f;
    double l = g * 2.0;
    long m = h << 2;
    int n = e ^ 0xFF;
    
    /* Register variable to force specific register usage */
    register int forced_reg asm("r12") = a * b + c;
    
    /* Complex pre-call computations creating dependencies */
    int t1 = a * b + c - d;
    int t2 = e + (forced_reg >> 3);
    float t3 = f * k + 2.0f;
    double t4 = g / l - 1.5;
    long t5 = h + m * 2;
    int t6 = i - j + n;
    
    /* Memory barrier to force all variables live across call */
    asm volatile("" : : : "memory");
    
    /* Function call with many arguments - some will go on stack on x86-64 */
    int ret = helper_function(t1, t2, (int)t3, (int)t4, (int)t5, 
                              t6, a, b, c, d);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Post-call computations using all variables */
    int u1 = ret + a + b;
    int u2 = c * d - e;
    float u3 = f + k * 2.0f;
    double u4 = g * l / 2.0;
    long u5 = h - m;
    int u6 = i + j - n;
    
    /* Use forced_reg after call - must be saved/restored */
    int final1 = u1 + u2 + forced_reg;
    float final2 = u3 + (float)u4;
    long final3 = u5 * u6;
    
    /* Complex final computation to use all values */
    int result = final1 + (int)final2 + (int)final3;
    
    /* Ensure all variables appear used */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                      "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
                      "r"(k), "r"(l), "r"(m), "r"(n));
    
    return result;
}

/* Main function to repeatedly exercise the target */
int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Loop to increase coverage chances */
    for (int iter = 0; iter < 10000; iter++) {
        /* Generate random inputs */
        int p1 = rand() % 100;
        int p2 = rand() % 100;
        int p3 = rand() % 100;
        int p4 = rand() % 100;
        int p5 = rand() % 100;
        float p6 = (float)(rand() % 100) / 10.0f;
        double p7 = (double)(rand() % 100) / 10.0;
        long p8 = rand() % 100;
        
        /* Call target function */
        int result = target_function(p1, p2, p3, p4, p5, p6, p7, p8);
        
        /* Accumulate to prevent dead code elimination */
        total += result;
        
        /* Occasionally print to prevent optimization */
        if (iter % 1000 == 0) {
            printf("Iteration %d: result = %d\n", iter, result);
        }
    }
    
    printf("Final total: %d\n", total);
    return total != 0 ? 0 : 1;
}
