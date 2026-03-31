/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
__attribute__((noinline, noclone))
double helper_func(double a, double b, double c, double d, 
                   double e, double f, double g, double h) {
    /* Complex computation to prevent optimization */
    volatile double result = 0.0;
    result = a * b + c * d - e / f + g - h;
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

/* Target function with high register pressure across a call */
__attribute__((noinline, noclone))
double target_function(double input) {
    /* Declare many local variables of mixed types */
    double a = input * 1.1;
    double b = input * 2.2;
    double c = input * 3.3;
    double d = input * 4.4;
    double e = input * 5.5;
    double f = input * 6.6;
    double g = input * 7.7;
    double h = input * 8.8;
    double i = input * 9.9;
    double j = input * 10.1;
    
    /* Integer variables to increase register pressure */
    int k = (int)input * 11;
    int l = (int)input * 12;
    long m = (long)input * 13;
    long n = (long)input * 14;
    
    /* Float variables */
    float o = (float)input * 15.5f;
    float p = (float)input * 16.6f;
    
    /* Register suggestion for specific register pressure */
    register int forced_reg asm("r12") = (int)input * 17;
    
    /* Complex pre-call computations creating dependencies */
    double t1 = a * b + c;
    double t2 = d - e / f;
    double t3 = g * h + i;
    double t4 = j - a + b;
    
    int t5 = k * l + (int)t1;
    long t6 = m - n * (long)t2;
    float t7 = o * p + (float)t3;
    
    /* Memory barrier to force all variables live across call */
    asm volatile("" : : : "memory");
    
    /* Function call with many arguments - forces register/stack pressure */
    double helper_result = helper_func(t1, t2, t3, t4, 
                                       (double)t5, (double)t6, 
                                       (double)t7, (double)forced_reg);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    double result = helper_result;
    result += a * 0.1;
    result -= b * 0.2;
    result *= c + 1.0;
    result /= d - 0.5;
    result += e * f;
    result -= g / h;
    result += i - j;
    
    result += (double)k * 0.3;
    result -= (double)l * 0.4;
    result += (double)m * 0.0001;
    result -= (double)n * 0.0002;
    result += (double)o * 1.1;
    result -= (double)p * 1.2;
    result += (double)forced_reg * 0.01;
    
    /* More arithmetic to ensure basic block continues after call */
    double final_result = result;
    final_result = final_result * final_result;
    final_result = sqrt(final_result + 1.0);
    final_result = log(final_result + 2.0);
    
    return final_result;
}

/* Main function to exercise the target */
int main() {
    srand(time(NULL));
    double total = 0.0;
    
    /* Loop to repeatedly call the target function */
    for (int iter = 0; iter < 100000; iter++) {
        /* Varying input to prevent constant propagation */
        double input = (double)rand() / RAND_MAX * 100.0;
        
        /* Call target function */
        double result = target_function(input);
        
        /* Accumulate result to prevent dead code elimination */
        total += result;
        
        /* Occasionally print to prevent optimization */
        if (iter % 10000 == 0) {
            printf("Iteration %d: result = %f, total = %f\n", 
                   iter, result, total);
        }
    }
    
    printf("Final total: %f\n", total);
    return 0;
}
