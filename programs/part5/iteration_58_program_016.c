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
    result = result ^ (result >> 16);
    result = result * 1103515245 + 12345;
    return result & 0x7FFFFFFF;
}

/* Another helper to force register pressure */
double __attribute__((noinline, noclone))
helper_double(double a, double b, double c, double d) {
    volatile double result = a * b + c / d;
    result = result * 1.23456789;
    return result;
}

/* The target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int input1, int input2, int input3, int input4,
                double input5, double input6, float input7, float input8) {
    
    /* Declare many local variables of mixed types */
    int a = input1 * 2;
    int b = input2 + 3;
    int c = input3 - 5;
    int d = input4 * 7;
    int e = a + b;
    int f = c - d;
    int g = input1 * input2;
    int h = input3 * input4;
    
    double x = input5 * 1.5;
    double y = input6 / 2.0;
    double z = x + y;
    
    float p = input7 * 3.14f;
    float q = input8 * 2.71f;
    float r = p - q;
    
    long l1 = (long)a * b;
    long l2 = (long)c * d;
    long l3 = l1 + l2;
    
    /* Use register suggestion for specific variable */
    register int critical_var asm("r12") = a * b + c * d;
    
    /* Complex pre-call computation creating web of dependencies */
    int t1 = a * b + c;
    int t2 = d - e / (f ? f : 1);
    int t3 = g * h + critical_var;
    double t4 = x * y + z;
    float t5 = p / (q ? q : 1.0f) + r;
    long t6 = l1 - l2 + l3;
    
    /* Memory barrier to force all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - some will go on stack on x86-64 */
    int helper_result = helper_function(t1, t2, t3, a, b, c, d, e);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Post-call computation using all variables */
    int u1 = helper_result + t1;
    int u2 = u1 * t2 - t3;
    double u3 = t4 + helper_double(x, y, z, input5);
    float u4 = t5 * 2.0f + p - q;
    long u5 = t6 * 3 + l1;
    
    /* Use critical_var after call (forces save/restore if in call-clobbered reg) */
    int final1 = u1 + u2 + critical_var;
    double final2 = u3 * 2.0;
    float final3 = u4 / 1.5f;
    long final4 = u5 >> 2;
    
    /* More complex mixing */
    int result = final1 + (int)final2 + (int)final3 + (int)final4;
    
    /* Ensure all variables are used to prevent optimization */
    volatile int dummy = a + b + c + d + e + f + g + h;
    dummy += (int)x + (int)y + (int)z;
    dummy += (int)p + (int)q + (int)r;
    dummy += (int)l1 + (int)l2 + (int)l3;
    
    return result ^ dummy;
}

/* Main function to repeatedly exercise the target */
int main() {
    srand(time(NULL));
    int total_result = 0;
    
    /* Create varying inputs to prevent constant propagation */
    int inputs[100];
    for (int i = 0; i < 100; i++) {
        inputs[i] = rand() % 1000;
    }
    
    /* Loop to repeatedly call the target function */
    for (int i = 0; i < 100; i++) {
        int idx = i % 100;
        total_result += target_function(
            inputs[idx],
            inputs[(idx + 1) % 100],
            inputs[(idx + 2) % 100],
            inputs[(idx + 3) % 100],
            (double)inputs[(idx + 4) % 100] / 10.0,
            (double)inputs[(idx + 5) % 100] / 20.0,
            (float)inputs[(idx + 6) % 100] / 5.0f,
            (float)inputs[(idx + 7) % 100] / 7.0f
        );
        
        /* Prevent loop unrolling that might eliminate calls */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Final result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
