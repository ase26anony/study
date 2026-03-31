/* Main test file to trigger caller-save insertion during reload */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions with side effects to prevent optimization */
extern void unknown_effect1(int, double);
extern void unknown_effect2(float, long);
extern void unknown_effect3(double, double, int);

/* Global volatile to ensure side effects are preserved */
volatile int global_counter = 0;

/* Non-inlinable external functions */
__attribute__((noinline)) 
void unknown_effect1(int a, double b) {
    global_counter += a + (int)b;
    asm volatile("" : : : "memory");
}

__attribute__((noinline))
void unknown_effect2(float a, long b) {
    global_counter += (int)a + (int)(b % 100);
    asm volatile("" : : : "memory");
}

__attribute__((noinline)) 
void unknown_effect3(double a, double b, int c) {
    global_counter += (int)(a + b) + c;
    asm volatile("" : : : "memory");
}

/* Function with high register pressure around calls */
__attribute__((noinline))
double high_pressure_function1(int iterations) {
    /* Declare many variables of mixed types to create register pressure */
    double v1 = 1.0, v2 = 2.0, v3 = 3.0, v4 = 4.0, v5 = 5.0;
    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f, f4 = 4.5f;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50, i6 = 60;
    long l1 = 100, l2 = 200, l3 = 300;
    
    double result = 0.0;
    
    /* Loop with calls that clobber registers */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex computation using all variables */
        v1 = v1 * 1.1 + sin(v2);
        v2 = v2 * 0.9 + cos(v3);
        v3 = v3 * 1.05 + tan(v4);
        v4 = v4 * 0.95 + exp(v5);
        v5 = v5 * 1.01 + log(v1 + 1.0);
        
        f1 = f1 * 1.1f + (float)sin(v1);
        f2 = f2 * 0.9f + (float)cos(v2);
        f3 = f3 * 1.05f + (float)tan(v3);
        f4 = f4 * 0.95f + (float)exp(v4);
        
        i1 = i1 * 2 + (int)v1;
        i2 = i2 * 3 + (int)v2;
        i3 = i3 * 4 + (int)v3;
        i4 = i4 * 5 + (int)v4;
        i5 = i5 * 6 + (int)v5;
        i6 = i6 * 7 + (int)f1;
        
        l1 = l1 * 2 + (long)v1;
        l2 = l2 * 3 + (long)v2;
        l3 = l3 * 4 + (long)v3;
        
        /* Call with many live values - forces caller-save */
        unknown_effect1(i1 + i2, v1 + v2);
        
        /* More computations between calls */
        v1 = v1 + v3 * v4;
        v2 = v2 + v5 * f1;
        
        /* Another call with different register types */
        unknown_effect2(f3 + f4, l1 + l2);
        
        /* Complex conditional to create control flow */
        if (iter % 3 == 0) {
            v3 = v3 * 2.0 - v4;
            unknown_effect3(v3, v4, i3);
        } else if (iter % 3 == 1) {
            v4 = v4 * 1.5 - v5;
            unknown_effect1(i4, v4);
        } else {
            v5 = v5 * 0.8 + v1;
            unknown_effect2(f2, l3);
        }
        
        /* Use all variables in final result */
        result += v1 + v2 + v3 + v4 + v5 + f1 + f2 + f3 + f4 
                + i1 + i2 + i3 + i4 + i5 + i6 + l1 + l2 + l3;
    }
    
    return result;
}

/* Second high-pressure function with different pattern */
__attribute__((noinline))
double high_pressure_function2(int iterations) {
    /* Different set of variables */
    double a1 = 0.1, a2 = 0.2, a3 = 0.3, a4 = 0.4, a5 = 0.5;
    double b1 = 1.1, b2 = 1.2, b3 = 1.3, b4 = 1.4;
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5, x6 = 6, x7 = 7, x8 = 8;
    float y1 = 0.01f, y2 = 0.02f, y3 = 0.03f, y4 = 0.04f;
    
    double sum = 0.0;
    
    /* Nested loops for more complex control flow */
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 3; j++) {
            /* Heavy computation */
            a1 = a1 * a2 + sin(a3) * cos(a4);
            a2 = a2 * a3 + tan(a4) * exp(a5);
            a3 = a3 * a4 + log(a5 + 1.0) * sqrt(a1);
            a4 = a4 * a5 + pow(a1, 2.0) * pow(a2, 1.5);
            a5 = a5 * a1 + atan(a2) * asin(a3);
            
            b1 = b1 + b2 * b3;
            b2 = b2 + b3 * b4;
            b3 = b3 + b4 * b1;
            b4 = b4 + b1 * b2;
            
            x1 = x1 * 3 + x2;
            x2 = x2 * 5 + x3;
            x3 = x3 * 7 + x4;
            x4 = x4 * 11 + x5;
            x5 = x5 * 13 + x6;
            x6 = x6 * 17 + x7;
            x7 = x7 * 19 + x8;
            x8 = x8 * 23 + x1;
            
            y1 = y1 * 1.1f + (float)a1;
            y2 = y2 * 1.2f + (float)a2;
            y3 = y3 * 1.3f + (float)a3;
            y4 = y4 * 1.4f + (float)a4;
            
            /* Switch statement for varied control flow */
            switch ((i + j) % 4) {
                case 0:
                    unknown_effect1(x1 + x2, a1 + b1);
                    a1 = a1 * 2.0;
                    break;
                case 1:
                    unknown_effect2(y1 + y2, (long)(x3 + x4));
                    a2 = a2 * 1.5;
                    break;
                case 2:
                    unknown_effect3(a3, b3, x5 + x6);
                    a3 = a3 * 0.8;
                    break;
                case 3:
                    unknown_effect1(x7 + x8, a4 + b4);
                    a4 = a4 * 1.2;
                    break;
            }
            
            /* Memory barrier to prevent reordering */
            asm volatile("" : : : "memory");
            
            /* Use variables across calls */
            sum += a1 * x1 + a2 * x2 + a3 * x3 + a4 * x4 + a5 * x5
                 + b1 * y1 + b2 * y2 + b3 * y3 + b4 * y4;
        }
    }
    
    return sum;
}

/* Third function with even more variables */
__attribute__((noinline))
double extreme_pressure_function(int iterations) {
    /* Maximum register pressure - 20+ variables */
    double d[10];
    int i[10];
    float f[10];
    
    /* Initialize arrays */
    for (int idx = 0; idx < 10; idx++) {
        d[idx] = idx * 1.1;
        i[idx] = idx * 100;
        f[idx] = idx * 0.1f;
    }
    
    double total = 0.0;
    
    /* Complex loop with multiple call sites */
    for (int iter = 0; iter < iterations; iter++) {
        /* Update all variables */
        for (int idx = 0; idx < 10; idx++) {
            d[idx] = d[idx] * 1.01 + sin(d[(idx+1)%10]);
            i[idx] = i[idx] * 2 + (int)d[idx] + idx;
            f[idx] = f[idx] * 1.02f + (float)cos(d[idx]);
        }
        
        /* Multiple calls in sequence with live values */
        unknown_effect1(i[0] + i[1], d[0] + d[1]);
        
        /* Computation between calls */
        d[2] = d[2] + d[3] * d[4];
        i[2] = i[2] + i[3] * i[4];
        
        unknown_effect2(f[0] + f[1], (long)(i[5] + i[6]));
        
        d[5] = d[5] + d[6] * d[7];
        i[7] = i[7] + i[8] * i[9];
        
        unknown_effect3(d[8], d[9], i[0] + i[9]);
        
        /* Conditional call insertion */
        if (iter % 2 == 0) {
            unknown_effect1(i[2], d[2]);
        } else {
            unknown_effect2(f[2], (long)i[2]);
        }
        
        /* Final aggregation using all variables */
        for (int idx = 0; idx < 10; idx++) {
            total += d[idx] + i[idx] + f[idx];
        }
    }
    
    return total;
}

int main() {
    int iterations = 100;
    double result1, result2, result3;
    
    printf("Starting caller-save pressure test...\n");
    
    /* Call all high-pressure functions */
    result1 = high_pressure_function1(iterations);
    printf("Function1 result: %f\n", result1);
    
    result2 = high_pressure_function2(iterations);
    printf("Function2 result: %f\n", result2);
    
    result3 = extreme_pressure_function(iterations / 2);
    printf("Function3 result: %f\n", result3);
    
    printf("Global counter: %d\n", global_counter);
    
    /* Verify results aren't optimized away */
    if (result1 != 0.0 || result2 != 0.0 || result3 != 0.0) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}
