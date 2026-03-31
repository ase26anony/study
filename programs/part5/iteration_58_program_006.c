/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
register_pressure_helper(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    /* Complex computation to prevent optimization */
    volatile int result = 0;
    result += a * b;
    result -= c / (d ? d : 1);
    result += e * f;
    result ^= g & h;
    result |= i << j;
    
    /* Force memory side effect */
    static int counter = 0;
    counter++;
    
    return result + counter;
}

/* Function designed to maximize register pressure across a call */
long __attribute__((noinline))
high_register_pressure_function(int input1, int input2, float input3, double input4) {
    /* Declare many variables of mixed types to consume registers */
    int a = input1 + 1;
    int b = input2 * 2;
    int c = input1 - input2;
    int d = input1 * input2;
    int e = input1 / (input2 ? input2 : 1);
    int f = input1 & 0xFF;
    int g = input2 | 0x55;
    int h = input1 ^ input2;
    
    /* Floating point variables - use different register classes */
    float f1 = input3 * 2.0f;
    float f2 = input3 + 1.5f;
    float f3 = input3 - 0.5f;
    
    double d1 = input4 * 3.14159;
    double d2 = input4 / 2.71828;
    double d3 = input4 + 1.61803;
    
    /* Long variables */
    long l1 = (long)a * b;
    long l2 = (long)c * d;
    long l3 = (long)e * f;
    
    /* Register suggestion for specific register (x86-64: r12 is call-clobbered) */
    register int forced_reg asm("r12") = a + b + c;
    
    /* Complex pre-call computations creating web of dependencies */
    int t1 = a * b + c - d;
    int t2 = e / (f ? f : 1) + g * h;
    float t3 = f1 * f2 - f3;
    double t4 = d1 / d2 + d3;
    long t5 = l1 - l2 + l3;
    
    /* Use all variables before call to make them live */
    int pre_sum = t1 + t2 + (int)t3 + (int)t4 + t5 + forced_reg;
    
    /* Compiler barrier - makes all variables appear live across call */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - forces some to stack on x86-64 */
    int helper_result = register_pressure_helper(
        t1, t2, pre_sum, 
        (int)t3, (int)t4, t5,
        a, b, c, d
    );
    
    /* Another compiler barrier */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    int post1 = helper_result * a + b;
    int post2 = c * d - e;
    float post3 = f1 + f2 * f3;
    double post4 = d1 - d2 / d3;
    long post5 = l1 * l2 - l3;
    
    /* Use forced_reg after call - forces save/restore if in call-clobbered reg */
    forced_reg = forced_reg + helper_result;
    
    /* More computations ensuring all variables are used */
    int final1 = post1 + post2 + (int)post3;
    double final2 = post4 + (double)post5;
    long final3 = (long)final1 * (long)final2;
    
    /* Mix all results */
    long final_result = final3 + forced_reg + (long)(post3 * 100.0f);
    
    /* Ensure all variables contribute to result to prevent DCE */
    final_result += (long)(d1 * 0.01);
    final_result += (long)(f1 * 10.0f);
    final_result += l1 + l2 + l3;
    
    return final_result;
}

/* Main function to repeatedly exercise the target function */
int main() {
    srand(time(NULL));
    long total_result = 0;
    
    /* Loop to create multiple compilation units/executions */
    for (int i = 0; i < 1000; i++) {
        /* Varying inputs to prevent constant propagation */
        int in1 = rand() % 100 + 1;
        int in2 = rand() % 100 + 1;
        float in3 = (float)(rand() % 100) / 10.0f;
        double in4 = (double)(rand() % 100) / 5.0;
        
        /* Call the high-pressure function */
        long result = high_register_pressure_function(in1, in2, in3, in4);
        
        /* Accumulate result to prevent dead code elimination */
        total_result += result;
        
        /* Occasionally print to prevent optimization */
        if (i % 100 == 0) {
            printf("Iteration %d: result = %ld\n", i, result);
        }
    }
    
    printf("Final total: %ld\n", total_result);
    return 0;
}
