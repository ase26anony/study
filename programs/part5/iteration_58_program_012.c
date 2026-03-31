/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
__attribute__((noinline, noclone))
double helper_func(double a, double b, double c, double d, 
                   double e, double f, double g, double h) {
    /* Complex computation with side effects */
    volatile double side_effect = 0;
    for (int i = 0; i < 10; i++) {
        side_effect += a * b * c;
    }
    
    /* Use all arguments in computation */
    double result = (a + b) * (c - d) / (e + 1.0) + f * g - h;
    
    /* Memory side effect */
    static double accumulator = 0;
    accumulator += result;
    
    return result + accumulator * 0.001;
}

/* Target function with high register pressure across a call */
__attribute__((noinline, optimize("O2")))
double target_function(double input1, double input2, double input3) {
    /* Declare many local variables of mixed types */
    double a = input1 * 2.0;
    double b = input2 / 3.0;
    double c = input3 + 1.5;
    double d = input1 - input2;
    double e = input2 * input3;
    double f = input3 / input1;
    
    /* Integer variables */
    long g = (long)(input1 * 1000);
    long h = (long)(input2 * 1000);
    int i = (int)(input3 * 100);
    int j = i * 2;
    
    /* Float variables */
    float k = (float)input1;
    float l = (float)input2;
    float m = (float)input3;
    
    /* Register suggestion for specific variable */
    register long forced_reg asm("r12") = g + h;
    
    /* Complex pre-call computations creating dependencies */
    double t1 = a * b + c * d;
    double t2 = e - f / (a + 1.0);
    long t3 = g * h - (long)i;
    float t4 = k * l + m;
    double t5 = t1 * t2 + t3;
    
    /* Use the register-suggested variable */
    t5 += (double)forced_reg;
    
    /* Memory barrier before call - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - forces register/stack pressure */
    double call_result = helper_func(t1, t2, t3, t4, t5, 
                                     (double)j, (double)k, (double)l);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Post-call computations using all variables */
    double r1 = call_result * a + b;
    double r2 = c - d * call_result;
    long r3 = g + h * (long)call_result;
    float r4 = k * l * m + (float)call_result;
    
    /* Use the register-suggested variable again */
    forced_reg += (long)(r1 * r2);
    
    /* More complex computations ensuring all values are used */
    double final_result = r1 + r2 + r3 + r4 + forced_reg + 
                         i + j + t1 + t2 + t3 + t4 + t5;
    
    /* Ensure the call is not at block end - add more computations */
    final_result *= 1.0001;
    final_result += 0.00001 * (a + b + c + d + e + f);
    
    return final_result;
}

/* Another helper to increase complexity */
__attribute__((noinline, noclone))
double secondary_helper(double x, double y, int z, long w) {
    volatile double v = x;
    for (int i = 0; i < 5; i++) {
        v = v * y + z - w;
    }
    return v;
}

/* Main test driver */
int main() {
    srand(time(NULL));
    double total = 0.0;
    
    /* Test with many different inputs */
    for (int iter = 0; iter < 1000; iter++) {
        double in1 = (double)rand() / RAND_MAX * 100.0;
        double in2 = (double)rand() / RAND_MAX * 100.0;
        double in3 = (double)rand() / RAND_MAX * 100.0;
        
        /* Call target function multiple times */
        double result = target_function(in1, in2, in3);
        
        /* Additional computation to prevent optimization */
        result += secondary_helper(in1, in2, iter, (long)result);
        
        total += result;
        
        /* Occasionally call helper directly to keep it alive */
        if (iter % 100 == 0) {
            total += helper_func(in1, in2, in3, in1, in2, in3, in1, in2);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", total);
    
    return 0;
}
