/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Complex computation to prevent optimization */
    volatile int barrier = 0;
    barrier = a + b - c * d + e / (f ? f : 1) + g - h;
    asm volatile("" : : : "memory");
    return barrier + 12345;
}

/* Another helper to force register usage */
double __attribute__((noinline, noclone))
helper_double(double a, double b, double c, float d, float e) {
    volatile double result = a * b + c - d / e;
    asm volatile("" : : : "memory");
    return result * 2.0;
}

/* The target function with high register pressure across a call */
long __attribute__((noinline, noclone))
target_function(int input1, int input2, double input3, float input4) {
    /* Declare many local variables of mixed types */
    int a = input1 * 2;
    int b = input2 + 5;
    int c = input1 - input2;
    int d = input1 * input2;
    int e = input2 / (input1 ? input1 : 1) + 1;
    int f = input1 + input2 * 3;
    int g = input2 - input1 * 2;
    int h = input1 * input1 - input2;
    
    float fa = input4 * 2.0f;
    float fb = input4 + 3.14f;
    float fc = input4 - 1.5f;
    
    double da = input3 * 1.5;
    double db = input3 + 2.71828;
    double dc = input3 / 3.0;
    
    long la = (long)input1 * 1000;
    long lb = (long)input2 * 2000;
    
    /* Use register suggestion for specific variable */
    register int forced_reg asm("r12") = a * b + c;
    
    /* Complex pre-call computations creating web of dependencies */
    int t1 = a * b + c - d;
    int t2 = e + f - g * h;
    float ft1 = fa * fb - fc;
    double dt1 = da + db * dc;
    
    /* Memory barrier to force all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - some will go on stack on x86-64 */
    int call_result = helper_function(t1, t2, forced_reg, d, e, f, g, h);
    
    /* Another memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    int post1 = call_result + a - b * c;
    int post2 = d + e - f * g + h;
    
    /* Use forced_reg after call (must be saved/restored) */
    int post3 = forced_reg * 2 + post1;
    
    float post4 = ft1 + fa - fb * fc;
    double post5 = dt1 + da - db * dc;
    
    /* Another function call with floating point args */
    double double_result = helper_double(post5, da, db, post4, fa);
    
    /* More computations to ensure BB doesn't end at call */
    long final1 = la + lb * post3;
    long final2 = (long)post1 * post2 + (long)post3;
    double final3 = double_result + post5 + dt1;
    
    /* Final result using all computed values */
    long final_result = final1 + final2 + (long)final3 + call_result;
    
    return final_result;
}

/* Main function to repeatedly exercise the target */
int main() {
    srand(time(NULL));
    long total = 0;
    const int iterations = 100000;
    
    for (int i = 0; i < iterations; i++) {
        /* Generate varying inputs to prevent constant propagation */
        int in1 = rand() % 100 + 1;
        int in2 = rand() % 100 + 1;
        double in3 = (rand() % 100) / 10.0 + 1.0;
        float in4 = (rand() % 100) / 10.0f + 1.0f;
        
        /* Call target function */
        long result = target_function(in1, in2, in3, in4);
        
        /* Accumulate result to prevent dead code elimination */
        total += result;
        
        /* Occasionally print to prevent optimization */
        if (i % 10000 == 0) {
            printf("Iteration %d: result = %ld\n", i, result);
        }
    }
    
    printf("Final total: %ld\n", total);
    return 0;
}
