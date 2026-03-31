/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, float c, double d, long e, int f, float g, double h) {
    /* Complex computation to prevent optimization */
    volatile int result = 0;
    result += a * b;
    result += (int)(c * 100.0f);
    result += (int)(d * 100.0);
    result += (int)(e % 1000);
    result += f;
    result += (int)(g * 50.0f);
    result += (int)(h * 50.0);
    
    /* Memory barrier to ensure side effects */
    asm volatile("" : : : "memory");
    return result;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Declare many local variables of mixed types */
    int v1 = a + b;
    int v2 = b * c;
    float v3 = (float)c / 2.0f;
    double v4 = (double)d * 1.5;
    long v5 = (long)e * 3L;
    int v6 = f - g;
    float v7 = (float)h * 0.75f;
    double v8 = (double)a * 2.5;
    int v9 = b + c + d;
    int v10 = e * f;
    
    /* Register suggestion for specific variable */
    register int critical_var asm("r12") = v1 * 2;
    
    /* Complex pre-call computations creating dependencies */
    int t1 = v1 * v2 + v6;
    float t2 = v3 * 2.0f - v7;
    double t3 = v4 / 2.0 + v8;
    long t4 = v5 + (long)v9;
    int t5 = v10 - v1;
    float t6 = v7 + v3;
    double t7 = v8 - v4;
    
    /* Memory barrier before call - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - some will go on stack on x86-64 */
    int helper_result = helper_function(
        t1,                    /* arg1 - in register */
        t5,                    /* arg2 - in register */
        t2,                    /* arg3 - in register */
        t3,                    /* arg4 - in register */
        t4,                    /* arg5 - in register */
        critical_var,          /* arg6 - in register */
        t6,                    /* arg7 - on stack */
        t7                     /* arg8 - on stack */
    );
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    int r1 = helper_result + v1 + v2;
    float r2 = (float)helper_result * v3 + v7;
    double r3 = (double)helper_result / v4 + v8;
    long r4 = (long)helper_result * v5 + (long)v9;
    int r5 = critical_var * 3 + v6;
    float r6 = v3 * v7 + (float)r1;
    double r7 = v4 * v8 + (double)r2;
    int r8 = v9 * v10 + r5;
    
    /* Final computation using all results */
    int final_result = r1 + (int)r2 + (int)r3 + (int)r4 + r5 + (int)r6 + (int)r7 + r8;
    
    /* Use critical_var one more time */
    final_result += critical_var;
    
    return final_result;
}

/* Main function to repeatedly exercise the target */
int main() {
    srand(time(NULL));
    int total_result = 0;
    
    /* Create varied inputs to prevent constant propagation */
    int inputs[8];
    for (int i = 0; i < 8; i++) {
        inputs[i] = rand() % 100 + 1;
    }
    
    /* Loop to increase coverage likelihood */
    for (int iteration = 0; iteration < 1000; iteration++) {
        /* Modify inputs slightly each iteration */
        for (int i = 0; i < 8; i++) {
            inputs[i] = (inputs[i] * 13 + 17) % 100 + 1;
        }
        
        /* Call target function with current inputs */
        total_result += target_function(
            inputs[0], inputs[1], inputs[2], inputs[3],
            inputs[4], inputs[5], inputs[6], inputs[7]
        );
        
        /* Prevent loop optimization */
        if (iteration % 100 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    
    return 0;
}
