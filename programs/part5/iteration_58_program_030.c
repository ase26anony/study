/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, long c, float d, double e, 
                int f, long g, float h, double i, int j) {
    /* Complex computation with side effects */
    volatile int side_effect = 0;
    side_effect = a + b + (int)c + (int)d + (int)e + f + (int)g + (int)h + (int)i + j;
    
    /* Use all arguments in computation */
    double result = (a * b) + (c / 2.0) + (d * 3.14f) + (e / 1.618) 
                    + (f * g) + (h * 2.71f) + (i * 3.14159) + j;
    
    /* Memory barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    return (int)result + side_effect;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline, noclone))
target_function(int input1, int input2, long input3, 
                float input4, double input5, int input6) {
    
    /* Declare many local variables with mixed types */
    int a = input1 * 2;
    int b = input2 + 3;
    long c = input3 - 100;
    float d = input4 * 1.5f;
    double e = input5 / 2.0;
    int f = input6 * 3;
    long g = c * 2;
    float h = d + 10.0f;
    double i = e * 3.14159;
    int j = a + b;
    
    /* Suggest specific register for a variable (x86-64 specific) */
    register int forced_reg asm("r12") = a * b + 100;
    
    /* Complex pre-call computations creating web of dependencies */
    int t1 = a * b + c % 256;
    long t2 = g - (long)(d * 10.0f);
    float t3 = h * 2.0f - d;
    double t4 = i / 1.618 + e;
    int t5 = f * j + forced_reg;
    
    /* Memory barrier before call - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - some will go on stack on x86-64 */
    int result = helper_function(t1, t2, (long)t3, (float)t4, t5,
                                 a, b, d, e, f);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Post-call computations using all variables */
    int post1 = result + a - b;
    long post2 = c + g + (long)result;
    float post3 = d * h + (float)result;
    double post4 = e * i + (double)result;
    int post5 = f * j * result;
    
    /* More complex dependent operations */
    int final1 = post1 * 2 + post5 % 100;
    long final2 = post2 / 3 + (long)post3;
    float final3 = post3 * 1.5f + (float)post4;
    double final4 = post4 / 2.0 + (double)post1;
    
    /* Use forced_reg across the call */
    int final5 = forced_reg * result + post1;
    
    /* Final computation using all values */
    int final_result = (int)(final1 + final2 + final3 + final4 + final5);
    
    /* Ensure all variables are used to prevent optimization */
    volatile int sink __attribute__((unused));
    sink = a + b + (int)c + (int)d + (int)e + f + (int)g + (int)h + (int)i + j;
    
    return final_result;
}

/* Main function to repeatedly exercise the target */
int main() {
    srand(time(NULL));
    int total = 0;
    const int iterations = 10000;
    
    for (int i = 0; i < iterations; i++) {
        /* Generate random inputs */
        int input1 = rand() % 100;
        int input2 = rand() % 100;
        long input3 = rand() % 1000;
        float input4 = (rand() % 100) / 10.0f;
        double input5 = (rand() % 100) / 5.0;
        int input6 = rand() % 100;
        
        /* Call target function */
        int result = target_function(input1, input2, input3, 
                                     input4, input5, input6);
        
        /* Accumulate result to prevent dead code elimination */
        total += result;
        
        /* Occasionally print to prevent optimization */
        if (i % 1000 == 0) {
            printf("Iteration %d: result = %d\n", i, result);
        }
    }
    
    printf("Final total: %d\n", total);
    return total > 0 ? 0 : 1;
}
