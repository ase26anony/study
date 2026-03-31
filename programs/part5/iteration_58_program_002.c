/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Complex computation to prevent optimization */
    volatile int result = 0;
    result += a * b;
    result -= c / (d ? d : 1);
    result += e * f;
    result ^= g << (h & 3);
    
    /* Force side effects */
    static int counter = 0;
    counter++;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return result + counter;
}

/* Target function with high register pressure */
int __attribute__((noinline))
target_function(int input1, int input2, int input3, int input4) {
    /* Declare many local variables with mixed types */
    int a = input1 * 2;
    int b = input2 + 5;
    int c = input3 - 3;
    int d = input4 / 2;
    
    /* Floating point variables increase register pressure */
    float e = input1 * 1.5f;
    float f = input2 * 0.75f;
    double g = input3 * 2.71828;
    double h = input4 * 3.14159;
    
    /* Long variables for more registers */
    long i = (long)input1 * input2;
    long j = (long)input3 * input4;
    
    /* Suggest specific register usage (x86-64 specific) */
    register int forced_reg asm("r12") = input1 + input2 + input3 + input4;
    
    /* Complex pre-call computations creating dependencies */
    int t1 = a * b + c;
    int t2 = d - (input2 ? input2 : 1);
    float t3 = e * f + 2.0f;
    double t4 = g / (h + 1.0);
    long t5 = i ^ j;
    int t6 = forced_reg * 3;
    
    /* Memory barrier before call - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Function call with many arguments - forces register/stack pressure */
    int helper_result = helper_function(t1, t2, (int)t3, (int)t4, 
                                       (int)(t5 & 0xFFFFFFFF), t6, a, b);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    int result = helper_result;
    result += c * d;
    result += (int)(e * 10.0f);
    result += (int)(g / 2.0);
    result += (int)(h * 100.0);
    result += (int)(i >> 8);
    result += (int)(j & 0xFF);
    result += forced_reg;
    
    /* Use all variables in final computation to keep them live */
    volatile int final_check = 0;
    final_check += a + b + c + d;
    final_check += (int)e + (int)f;
    final_check += (int)g + (int)h;
    final_check += (int)i + (int)j;
    
    return result + final_check;
}

/* Another helper to increase complexity */
double __attribute__((noinline, noclone))
second_helper(double x, double y, int z) {
    volatile double result = x * y + z;
    asm volatile("" : : : "memory");
    return result;
}

/* Main test driver */
int main() {
    srand(time(NULL));
    int total_result = 0;
    
    /* Test with various inputs to exercise different paths */
    for (int iteration = 0; iteration < 1000; iteration++) {
        /* Generate random inputs */
        int inputs[4];
        for (int i = 0; i < 4; i++) {
            inputs[i] = rand() % 100 + 1;
        }
        
        /* Call target function multiple times */
        int result = target_function(inputs[0], inputs[1], inputs[2], inputs[3]);
        
        /* Mix with another call to increase pressure */
        double extra = second_helper(result * 0.5, result * 1.5, iteration);
        
        /* Accumulate result to prevent elimination */
        total_result += result + (int)extra;
        
        /* Occasionally add memory barrier */
        if (iteration % 100 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Final result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
