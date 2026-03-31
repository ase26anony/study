/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, long c, double d, float e, 
                int f, long g, double h, float i, int j) {
    /* Complex computation to prevent optimization */
    volatile int result = 0;
    result += a * b;
    result += (int)(c % 100);
    result += (int)(d * 100.0);
    result += (int)(e * 100.0f);
    result += f;
    result += (int)(g % 100);
    result += (int)(h * 50.0);
    result += (int)(i * 50.0f);
    result += j;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int input1, int input2, long input3, double input4, float input5) {
    /* Declare many local variables of mixed types */
    int var1 = input1 * 2;
    int var2 = input2 + 5;
    long var3 = input3 * 3L;
    double var4 = input4 * 1.5;
    float var5 = input5 * 2.0f;
    int var6 = var1 + var2;
    long var7 = var3 + 1000L;
    double var8 = var4 / 2.0;
    float var9 = var5 + 3.14f;
    int var10 = var6 * 3;
    
    /* Register variable to force specific register usage (x86-64: r12 is call-clobbered) */
    register int forced_reg asm("r12") = var1 * var2 + 123;
    
    /* Complex pre-call computations creating web of dependencies */
    int t1 = var1 * var2 + var6;
    long t2 = var3 - var7 / 3L;
    double t3 = var4 * var8 - 1.0;
    float t4 = var5 + var9 * 2.0f;
    int t5 = var10 + forced_reg;
    
    /* Memory barrier before call - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - forces some to stack on x86-64 */
    int result = helper_function(
        t1,                    /* arg1: integer */
        t2 & 0x7FFFFFFF,       /* arg2: integer */
        t2,                    /* arg3: long */
        t3,                    /* arg4: double */
        t4,                    /* arg5: float */
        t5,                    /* arg6: integer */
        var7,                  /* arg7: long - will go on stack */
        var8,                  /* arg8: double - will go on stack */
        var9,                  /* arg9: float - will go on stack */
        forced_reg             /* arg10: integer - will go on stack */
    );
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Post-call computations using all variables - keeping them live */
    int post1 = result + var1 - var2;
    long post2 = var3 + (long)post1 * 2L;
    double post3 = var4 * (double)post2;
    float post4 = var5 + (float)post3;
    
    /* More complex computations */
    int final1 = post1 * 2 + (int)post2 % 100;
    double final2 = post3 / 2.0 + (double)post4;
    float final3 = post4 * 3.0f + (float)final1;
    
    /* Use forced_reg after call - forces save/restore if in call-clobbered reg */
    int final_result = final1 + (int)final2 + (int)final3 + forced_reg;
    
    /* Use all variables one more time to ensure they stay live */
    final_result += var6 + (int)(var7 % 100) + (int)var8 + (int)var9 + var10;
    
    return final_result;
}

/* Another helper to increase call complexity */
double __attribute__((noinline, noclone))
second_helper(double a, double b, double c, double d, double e,
              double f, double g, double h, double i, double j) {
    volatile double sum = 0.0;
    sum += a + b + c + d + e + f + g + h + i + j;
    asm volatile("" : : : "memory");
    return sum;
}

/* Function with multiple calls to increase pressure */
int __attribute__((noinline))
multi_call_function(int base) {
    /* Many variables */
    double d1 = base * 1.1;
    double d2 = base * 2.2;
    double d3 = base * 3.3;
    double d4 = base * 4.4;
    double d5 = base * 5.5;
    double d6 = base * 6.6;
    double d7 = base * 7.7;
    double d8 = base * 8.8;
    double d9 = base * 9.9;
    double d10 = base * 10.1;
    
    /* First call */
    asm volatile("" : : : "memory");
    double sum1 = second_helper(d1, d2, d3, d4, d5, d6, d7, d8, d9, d10);
    asm volatile("" : : : "memory");
    
    /* Computation between calls */
    d1 += sum1;
    d2 -= sum1;
    d3 *= 1.1 + sum1;
    
    /* Second call */
    asm volatile("" : : : "memory");
    double sum2 = second_helper(d1, d2, d3, d4, d5, d6, d7, d8, d9, d10);
    asm volatile("" : : : "memory");
    
    /* More computations */
    int result = (int)(d1 + d2 + d3 + d4 + d5 + sum1 + sum2);
    
    /* Third call to target_function */
    asm volatile("" : : : "memory");
    result += target_function(result, result+1, result+2, (double)result, (float)result);
    asm volatile("" : : : "memory");
    
    return result;
}

int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Loop to repeatedly exercise the target functions */
    for (int i = 0; i < 1000; i++) {
        /* Varying inputs to prevent constant propagation */
        int input1 = rand() % 100;
        int input2 = rand() % 100;
        long input3 = rand() % 1000;
        double input4 = (double)(rand() % 100) / 10.0;
        float input5 = (float)(rand() % 100) / 10.0f;
        
        /* Call target function */
        int result = target_function(input1, input2, input3, input4, input5);
        total += result;
        
        /* Also call multi-call function */
        if (i % 3 == 0) {
            total += multi_call_function(input1);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", total);
    
    return 0;
}
