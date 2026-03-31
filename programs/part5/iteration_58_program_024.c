/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Complex computation to prevent optimization */
    volatile int result = a * b + c * d - e * f + g * h;
    /* Memory barrier to ensure side effects */
    asm volatile("" : : : "memory");
    return result;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    /* Declare many local variables with mixed types */
    int var1, var2, var3, var4, var5, var6, var7, var8;
    float f1, f2, f3;
    double d1, d2;
    long l1, l2;
    
    /* Register suggestion for specific variable */
    register int forced_reg asm("r12") = a + b;
    
    /* Complex pre-call computations creating web of dependencies */
    var1 = a * b + c;
    var2 = d - e / (f + 1);
    var3 = g * h + i * j;
    var4 = (a + c) * (d + f);
    var5 = b * e - g * h;
    var6 = i * j + a * c;
    var7 = d * f - e * g;
    var8 = h * i + j * b;
    
    f1 = (float)var1 / (var2 + 1.0f);
    f2 = (float)var3 * 0.5f + f1;
    f3 = f1 * f2 - (float)var4;
    
    d1 = (double)var5 * 1.5 + (double)var6 * 0.25;
    d2 = d1 / (double)(var7 + 1) + (double)var8;
    
    l1 = (long)var1 * var2 + (long)var3 * var4;
    l2 = l1 - (long)var5 * var6;
    
    /* Memory barrier before call - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - forces register/stack pressure */
    int helper_result = helper_function(
        var1, var2, var3, var4, 
        var5, var6, var7, var8
    );
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    int result1 = helper_result + var1 * var2 - var3;
    float result2 = f1 + f2 * f3 + (float)helper_result;
    double result3 = d1 * 2.0 - d2 + (double)helper_result;
    long result4 = l1 / (l2 + 1) + (long)helper_result;
    
    /* Use the register-suggested variable */
    forced_reg += helper_result;
    
    /* More computations to ensure straight-line flow after call */
    int final1 = result1 + (int)result2 + var4;
    double final2 = result3 + (double)result4 + d2;
    float final3 = (float)final1 * 0.3f + (float)final2;
    
    /* Use all variables one more time */
    return final1 + (int)final2 + (int)final3 + forced_reg + 
           (int)f1 + (int)d1 + (int)l1 + var5 + var6 + var7 + var8;
}

/* Main function to repeatedly exercise the target */
int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Loop to create multiple compilation contexts */
    for (int iter = 0; iter < 100000; iter++) {
        /* Generate random inputs to prevent constant propagation */
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
        
        /* Call target function and accumulate result */
        total += target_function(a, b, c, d, e, f, g, h, i, j);
        
        /* Prevent loop optimization */
        if (iter % 1000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
