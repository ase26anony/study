/* Test program to trigger caller-save insertion logic in reload pass */
/* Specifically targets lines 905-913 in caller-save.cc */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* External function with unknown side effects - prevents inlining */
extern void unknown_effect(int a, double b) __attribute__((noinline));
extern void unknown_effect2(float a, float b, float c) __attribute__((noinline));
extern void unknown_effect3(long a, long b, double c, double d) __attribute__((noinline));

/* Prevent compiler from optimizing away computations */
#define KEEP_ALIVE(var) asm volatile("" : "+r"(var))

/* Function 1: High pressure with mixed integer/float types */
int high_pressure_function1(int* ints, double* doubles) {
    /* Unpack into many scalar variables - all must be kept alive */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] ^ ints[8];
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] + doubles[2];
    double d3 = doubles[3] / doubles[4];
    double d4 = sin(doubles[5]);
    double d5 = cos(doubles[6]);
    
    float f1 = (float)doubles[7];
    float f2 = (float)doubles[8];
    float f3 = (float)doubles[9];
    
    long l1 = (long)ints[9] * 1000;
    long l2 = (long)ints[10] << 4;
    
    /* Force all variables to be live across the call */
    /* Use them in computations that can't be optimized away */
    int temp_int = v1 + v2 - v3;
    double temp_double = d1 * d2 - d3;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call - clobbers call-used registers */
    unknown_effect(temp_int, temp_double);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all variables after call - forcing caller-save restoration */
    v1 = v1 + v4;
    v2 = v2 ^ v5;
    v3 = v3 * temp_int;
    
    d1 = d1 + d4;
    d2 = d2 * d5;
    d3 = d3 - temp_double;
    
    f1 = f1 + f2;
    f2 = f2 * f3;
    
    l1 = l1 | l2;
    
    /* Another call with different signature */
    unknown_effect2(f1, f2, f3);
    
    /* More computations */
    int result = v1 + v2 + v3 + v4 + v5;
    result += (int)(d1 + d2 + d3 + d4 + d5);
    result += (int)(f1 + f2 + f3);
    result += (int)(l1 + l2);
    
    KEEP_ALIVE(result);
    return result;
}

/* Function 2: Different pattern with nested control flow */
double high_pressure_function2(int* ints, double* doubles, int iterations) {
    double total = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many live values inside loop */
        int a1 = ints[0] + i;
        int a2 = ints[1] * i;
        int a3 = ints[2] - i;
        int a4 = ints[3] ^ i;
        int a5 = ints[4] | i;
        
        double b1 = doubles[0] * i;
        double b2 = doubles[1] + i;
        double b3 = doubles[2] / (i + 1);
        double b4 = doubles[3] - i;
        double b5 = doubles[4] * sin(i);
        
        float c1 = (float)doubles[5] + i;
        float c2 = (float)doubles[6] * i;
        float c3 = (float)doubles[7] - i;
        
        long d1 = (long)ints[5] << i;
        long d2 = (long)ints[6] >> (i & 7);
        
        /* Conditional to create complex control flow */
        if (i % 3 == 0) {
            /* Call in one branch */
            unknown_effect3(d1, d2, b1, b2);
            
            a1 = a1 * 2;
            b1 = b1 * 2.0;
        } else if (i % 3 == 1) {
            /* Different call in another branch */
            unknown_effect(a1 + a2, b3 + b4);
            
            a2 = a2 ^ 0xFF;
            b2 = sqrt(b2);
        } else {
            /* Third call pattern */
            unknown_effect2(c1, c2, c3);
            
            a3 = a3 | 0xFFFF;
            b3 = log(fabs(b3) + 1.0);
        }
        
        /* Use all values after call - forcing caller-save */
        int int_sum = a1 + a2 + a3 + a4 + a5;
        double double_sum = b1 + b2 + b3 + b4 + b5;
        float float_sum = c1 + c2 + c3;
        long long_sum = d1 + d2;
        
        total += int_sum + double_sum + float_sum + long_sum;
        
        /* Memory barrier every few iterations */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    KEEP_ALIVE(total);
    return total;
}

/* Function 3: Switch statement with calls at multiple points */
int high_pressure_function3(int* ints, double* doubles, int selector) {
    /* Many live variables */
    int r1 = ints[0];
    int r2 = ints[1];
    int r3 = ints[2];
    int r4 = ints[3];
    int r5 = ints[4];
    int r6 = ints[5];
    int r7 = ints[6];
    int r8 = ints[7];
    
    double s1 = doubles[0];
    double s2 = doubles[1];
    double s3 = doubles[2];
    double s4 = doubles[3];
    double s5 = doubles[4];
    
    float t1 = (float)doubles[5];
    float t2 = (float)doubles[6];
    float t3 = (float)doubles[7];
    
    int result = 0;
    
    switch (selector % 5) {
        case 0:
            unknown_effect(r1 + r2, s1 * s2);
            result = r1 + r3 + r5;
            break;
        case 1:
            unknown_effect2(t1, t2, t3);
            result = r2 + r4 + r6;
            break;
        case 2:
            unknown_effect3(r7, r8, s3, s4);
            result = r3 + r5 + r7;
            break;
        case 3:
            /* Multiple calls in one case */
            unknown_effect(r4, s5);
            asm volatile("" : : : "memory");
            unknown_effect2(t1 * 2.0f, t2, t3);
            result = r4 + r6 + r8;
            break;
        case 4:
            /* Complex computation with call in middle */
            r1 = r1 * r2;
            s1 = s1 / s2;
            unknown_effect(r1, s1);
            r3 = r3 ^ r4;
            s3 = sqrt(s3);
            result = r1 + r3 + (int)s1 + (int)s3;
            break;
    }
    
    /* Use remaining variables */
    result += r5 + r6 + r7 + r8;
    result += (int)(s2 + s4 + s5);
    result += (int)(t1 + t2 + t3);
    
    KEEP_ALIVE(result);
    return result;
}

int main() {
    const int NUM_ITEMS = 20;
    int* int_array = malloc(NUM_ITEMS * sizeof(int));
    double* double_array = malloc(NUM_ITEMS * sizeof(double));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < NUM_ITEMS; i++) {
        int_array[i] = (i * 37 + 123) % 1000;
        double_array[i] = (double)((i * 51 + 456) % 1000) / 10.0;
    }
    
    int total = 0;
    double total_double = 0.0;
    
    /* Loop with multiple high-pressure calls */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the inputs slightly each iteration */
        int_array[0] += iter % 7;
        double_array[0] += (iter % 11) * 0.1;
        
        /* Call all three high-pressure functions */
        total += high_pressure_function1(int_array, double_array);
        total_double += high_pressure_function2(int_array, double_array, 10);
        total += high_pressure_function3(int_array, double_array, iter);
        
        /* Memory barrier every 20 iterations */
        if (iter % 20 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Use results to prevent optimization */
    printf("Result: %d, %.2f\n", total, total_double);
    
    free(int_array);
    free(double_array);
    
    return 0;
}
