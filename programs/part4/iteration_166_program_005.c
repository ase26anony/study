/* caller-save-test.c - Main test program to trigger caller-save insertion */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions with side effects to prevent optimization */
extern void unknown_effect1(int, double);
extern void unknown_effect2(float, int, double);
extern void unknown_effect3(void);

/* Global volatile to ensure side effects are visible */
volatile int global_counter = 0;

/* Non-inlinable external functions defined in separate file */
__attribute__((noinline)) 
void unknown_effect1(int a, double b) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_counter += a + (int)b;
}

__attribute__((noinline))
void unknown_effect2(float f, int i, double d) {
    /* Complex enough to not be optimized away */
    asm volatile("" : : "r"(f), "r"(i), "r"(d) : "memory");
    global_counter += (int)(f * i + d);
}

__attribute__((noinline))
void unknown_effect3(void) {
    /* Memory barrier to force register preservation */
    asm volatile("" : : : "memory");
    global_counter++;
}

/* High pressure function with many live integer variables across call */
__attribute__((noinline))
int high_pressure_int_call(int iter) {
    /* Declare many integer variables that will need registers */
    int v1 = iter * 1;
    int v2 = iter * 2;
    int v3 = iter * 3;
    int v4 = iter * 4;
    int v5 = iter * 5;
    int v6 = iter * 6;
    int v7 = iter * 7;
    int v8 = iter * 8;
    int v9 = iter * 9;
    int v10 = iter * 10;
    int v11 = iter * 11;
    int v12 = iter * 12;
    int v13 = iter * 13;
    int v14 = iter * 14;
    int v15 = iter * 15;
    
    /* Use all variables in computation before call */
    int sum1 = v1 + v2 + v3 + v4 + v5;
    int sum2 = v6 + v7 + v8 + v9 + v10;
    int sum3 = v11 + v12 + v13 + v14 + v15;
    
    /* Complex expression that keeps values live */
    int pre_call_result = (sum1 * sum2) / (sum3 + 1) + 
                         (v1 ^ v2) | (v3 & v4) << 2;
    
    /* Call external function - all above variables must be preserved */
    unknown_effect1(pre_call_result, (double)sum1 / sum2);
    
    /* Use variables after call in different computation */
    int post_call_result = (v15 - v14) * (v13 - v12) +
                          (v11 * v10) / (v9 + 1) +
                          (v8 | v7) & (v6 ^ v5);
    
    /* Another call with different arguments */
    unknown_effect2((float)post_call_result, v1, (double)v2);
    
    /* Final computation using all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + pre_call_result + post_call_result;
}

/* High pressure function with mixed int/float/double variables */
__attribute__((noinline))
double high_pressure_mixed_call(int seed) {
    /* Mixed types to use different register classes */
    int i1 = seed + 1;
    int i2 = seed + 2;
    int i3 = seed + 3;
    int i4 = seed + 4;
    int i5 = seed + 5;
    
    float f1 = seed * 1.1f;
    float f2 = seed * 2.2f;
    float f3 = seed * 3.3f;
    float f4 = seed * 4.4f;
    
    double d1 = seed * 1.111;
    double d2 = seed * 2.222;
    double d3 = seed * 3.333;
    double d4 = seed * 4.444;
    double d5 = seed * 5.555;
    
    /* Complex floating point computations */
    double fp_sum1 = d1 * d2 + d3 / d4 - d5;
    float fp_sum2 = f1 * f2 + f3 - f4;
    int int_sum = i1 * i2 + i3 - i4 * i5;
    
    /* Mix types in computation */
    double mixed1 = fp_sum1 * int_sum + fp_sum2;
    
    /* Call with mixed arguments - forces preservation of all reg types */
    unknown_effect2(f1, i1, d1);
    
    /* More computations keeping values live */
    double mixed2 = (d2 * i2) + (f2 * i3) - (d3 / i4);
    
    /* Another call */
    unknown_effect1(i5, d4);
    
    /* Use all variables in final result */
    return mixed1 + mixed2 + f3 + f4 + d5 + i1 + i2 + i3 + i4 + i5;
}

/* Function with calls in complex control flow */
__attribute__((noinline))
int high_pressure_control_flow(int n) {
    int result = 0;
    
    /* Loop with call inside - creates multiple insertion points */
    for (int i = 0; i < n; i++) {
        /* Many live variables inside loop */
        int a = i * 2;
        int b = i * 3;
        int c = i * 4;
        int d = i * 5;
        int e = i * 6;
        int f = i * 7;
        int g = i * 8;
        int h = i * 9;
        
        double x = i * 1.5;
        double y = i * 2.5;
        double z = i * 3.5;
        
        /* Conditional with call inside */
        if (i % 3 == 0) {
            /* Call in one branch */
            unknown_effect1(a + b, x + y);
            result += a * b;
        } else if (i % 3 == 1) {
            /* Different call in another branch */
            unknown_effect2((float)x, c, y);
            result += c * d;
        } else {
            /* Third call pattern */
            unknown_effect3();
            result += e * f;
        }
        
        /* Use variables after conditional call */
        int temp = (a * c) + (b * d) - (e * f) + (g * h);
        double ftemp = (x * y) / (z + 1.0);
        
        /* Another call with mixed arguments */
        unknown_effect2((float)ftemp, temp, z);
        
        /* Accumulate result using all variables */
        result += a + b + c + d + e + f + g + h + (int)(x + y + z);
    }
    
    return result;
}

/* Function with nested loops and calls */
__attribute__((noinline))
double nested_loop_pressure(int outer, int inner) {
    double total = 0.0;
    
    for (int i = 0; i < outer; i++) {
        /* Outer loop variables */
        int o1 = i * 11;
        int o2 = i * 12;
        int o3 = i * 13;
        double od1 = i * 1.11;
        double od2 = i * 2.22;
        
        for (int j = 0; j < inner; j++) {
            /* Inner loop variables - high pressure */
            int i1 = j * 2;
            int i2 = j * 3;
            int i3 = j * 4;
            int i4 = j * 5;
            int i5 = j * 6;
            int i6 = j * 7;
            int i7 = j * 8;
            
            float f1 = j * 0.1f;
            float f2 = j * 0.2f;
            float f3 = j * 0.3f;
            
            double d1 = j * 0.01;
            double d2 = j * 0.02;
            double d3 = j * 0.03;
            double d4 = j * 0.04;
            
            /* Computation before call */
            double pre = (i1 * d1) + (i2 * d2) - (i3 * d3) + (i4 * d4);
            float fpre = (f1 * i5) + (f2 * i6) - (f3 * i7);
            
            /* Call with many live values */
            unknown_effect1(i1 + i2, d1 + d2);
            
            /* Computation after call */
            double post = (d3 * i3) + (d4 * i4) - (d1 * i1);
            
            /* Use outer loop variables too */
            total += pre + post + fpre + o1 + o2 + o3 + od1 + od2;
            
            /* Another call with different arguments */
            if (j % 5 == 0) {
                unknown_effect2(f1, i5, d3);
            }
        }
        
        /* Call at outer loop level */
        unknown_effect3();
    }
    
    return total;
}

/* Main test driver */
int main(void) {
    int total = 0;
    double dtotal = 0.0;
    
    printf("Starting caller-save test...\n");
    
    /* Test 1: Integer pressure in loop */
    for (int i = 0; i < 100; i++) {
        total += high_pressure_int_call(i);
    }
    
    /* Test 2: Mixed type pressure */
    for (int i = 0; i < 50; i++) {
        dtotal += high_pressure_mixed_call(i);
    }
    
    /* Test 3: Control flow with calls */
    total += high_pressure_control_flow(100);
    
    /* Test 4: Nested loops */
    dtotal += nested_loop_pressure(10, 20);
    
    printf("Integer total: %d\n", total);
    printf("Double total: %f\n", dtotal);
    printf("Global counter: %d\n", global_counter);
    
    /* Use results to prevent optimization */
    if (total > 1000000 || dtotal > 1000000.0) {
        printf("Results are large enough\n");
    }
    
    return 0;
}
