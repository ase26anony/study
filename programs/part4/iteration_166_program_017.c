#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// External function with unknown side effects - prevents optimization
extern void unknown_effect(int, double) __attribute__((noinline));
extern void another_effect(float, int, double) __attribute__((noinline));
extern void mixed_effect(long, float, double, int) __attribute__((noinline));

// Global volatile to prevent dead code elimination
volatile int global_counter = 0;

// External function definitions (will be in separate file)
void unknown_effect(int a, double b) {
    // Use asm to prevent optimization
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_counter += a + (int)b;
}

void another_effect(float f, int i, double d) {
    asm volatile("" : : "r"(f), "r"(i), "r"(d) : "memory");
    global_counter += (int)f + i + (int)d;
}

void mixed_effect(long l, float f, double d, int i) {
    asm volatile("" : : "r"(l), "r"(f), "r"(d), "r"(i) : "memory");
    global_counter += (int)l + (int)f + (int)d + i;
}

// Function with extreme register pressure around a call
int high_pressure_call_1(int *ints, double *doubles) {
    // Declare many variables of mixed types - all must be live across call
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] / (ints[8] + 1);
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] + doubles[2];
    double d3 = doubles[3] - doubles[4];
    double d4 = doubles[5] * doubles[6];
    double d5 = doubles[7] / (doubles[8] + 1.0);
    
    float f1 = (float)d1 * 2.0f;
    float f2 = (float)d2 + 1.5f;
    float f3 = (float)d3 - 0.5f;
    
    long l1 = (long)v1 * v2;
    long l2 = (long)v3 * v4;
    
    // Complex computation mixing all variables
    double complex1 = d1 * v1 + d2 * v2 - d3 * v3;
    float complex2 = f1 * v4 + f2 * v5 - f3 * v1;
    
    // Memory barrier to prevent reordering
    asm volatile("" : : : "memory");
    
    // External call - all above variables must be preserved
    unknown_effect(v1 + v2, d1 + d2);
    
    // More computations after call using all variables
    v1 = v1 + (int)(d1 * 100.0);
    v2 = v2 * (int)(d2 * 50.0);
    v3 = v3 - (int)(d3 * 25.0);
    v4 = v4 / ((int)d4 + 1);
    v5 = v5 ^ (int)d5;
    
    d1 = d1 + (double)v1;
    d2 = d2 * (double)v2;
    d3 = d3 - (double)v3;
    d4 = d4 / ((double)v4 + 1.0);
    d5 = d5 * (double)v5;
    
    f1 = f1 + (float)v1;
    f2 = f2 * (float)v2;
    f3 = f3 - (float)v3;
    
    // Another memory barrier
    asm volatile("" : : : "memory");
    
    // Return checksum using all variables
    return v1 + v2 + v3 + v4 + v5 + 
           (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
           (int)f1 + (int)f2 + (int)f3 +
           (int)l1 % 256 + (int)l2 % 256 +
           (int)complex1 % 256 + (int)complex2 % 256;
}

// Second high-pressure function with different pattern
double high_pressure_call_2(float *floats, int *ints, double *doubles) {
    // Even more variables
    int a1 = ints[0], a2 = ints[1], a3 = ints[2], a4 = ints[3], a5 = ints[4];
    int a6 = ints[5], a7 = ints[6], a8 = ints[7], a9 = ints[8], a10 = ints[9];
    
    float b1 = floats[0], b2 = floats[1], b3 = floats[2], b4 = floats[3];
    float b5 = floats[4], b6 = floats[5], b7 = floats[6], b8 = floats[7];
    
    double c1 = doubles[0], c2 = doubles[1], c3 = doubles[2], c4 = doubles[3];
    double c5 = doubles[4], c6 = doubles[5], c7 = doubles[6], c8 = doubles[7];
    
    // Pre-call computations
    int sum_i = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    float prod_f = b1 * b2 * b3 * b4 * b5 * b6 * b7 * b8;
    double sum_d = c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8;
    
    // Nested control flow with call inside
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            another_effect(b1 + i, a1 * i, c1 * i);
        } else if (i == 1) {
            // Modify variables in loop
            a1 += i; a2 -= i; a3 *= i;
            b1 += i; b2 -= i; b3 *= i;
            c1 += i; c2 -= i; c3 *= i;
            another_effect(b1, a1, c1);
        } else {
            // Different call pattern
            mixed_effect(a1 + a2, b1 + b2, c1 + c2, a3 + a4);
        }
        
        // More computations between calls
        sum_i += a1 + a2;
        prod_f *= b1 + b2;
        sum_d += c1 + c2;
    }
    
    // Complex return value using all variables
    return (double)sum_i + (double)prod_f + sum_d +
           (double)a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           (double)b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 +
           c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8;
}

// Third function with switch statement and calls
int high_pressure_call_3(int mode, int *ints, double *doubles) {
    int r1 = ints[0], r2 = ints[1], r3 = ints[2], r4 = ints[3];
    int r5 = ints[4], r6 = ints[5], r7 = ints[6], r8 = ints[7];
    int r9 = ints[8], r10 = ints[9], r11 = ints[10], r12 = ints[11];
    
    double s1 = doubles[0], s2 = doubles[1], s3 = doubles[2], s4 = doubles[3];
    double s5 = doubles[4], s6 = doubles[5], s7 = doubles[6], s8 = doubles[7];
    double s9 = doubles[8], s10 = doubles[9];
    
    float t1 = (float)s1, t2 = (float)s2, t3 = (float)s3, t4 = (float)s4;
    
    // Switch with calls in different cases
    switch (mode % 4) {
        case 0:
            unknown_effect(r1 + r2, s1 + s2);
            r1 = (int)(s1 * 100.0);
            r2 = (int)(s2 * 50.0);
            break;
        case 1:
            another_effect(t1 + t2, r3 + r4, s3 + s4);
            r3 = (int)(s3 * 75.0);
            r4 = (int)(s4 * 25.0);
            break;
        case 2:
            mixed_effect(r5 + r6, t3 + t4, s5 + s6, r7 + r8);
            r5 = (int)(s5 * 60.0);
            r6 = (int)(s6 * 40.0);
            break;
        case 3:
            unknown_effect(r9 + r10, s7 + s8);
            another_effect(t1, r11, s9);
            r9 = (int)(s7 * 90.0);
            r10 = (int)(s8 * 10.0);
            break;
    }
    
    // All variables still in use
    int sum_int = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
    double sum_double = s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10;
    float sum_float = t1 + t2 + t3 + t4;
    
    return sum_int + (int)sum_double + (int)sum_float;
}

int main() {
    // Initialize test data
    int int_data[20];
    float float_data[20];
    double double_data[20];
    
    for (int i = 0; i < 20; i++) {
        int_data[i] = (i * 37) % 100 + 1;  // Non-zero, non-constant values
        float_data[i] = (float)(i * 19) / 7.0f;
        double_data[i] = (double)(i * 23) / 11.0;
    }
    
    int total = 0;
    
    // Loop with multiple high-pressure calls
    for (int iter = 0; iter < 100; iter++) {
        // Vary inputs slightly each iteration
        int_data[0] += iter % 7;
        float_data[0] += (float)(iter % 5) * 0.1f;
        double_data[0] += (double)(iter % 3) * 0.01;
        
        // Call all three high-pressure functions
        total += high_pressure_call_1(int_data, double_data);
        
        double result2 = high_pressure_call_2(float_data, int_data, double_data);
        total += (int)result2;
        
        total += high_pressure_call_3(iter, int_data, double_data);
        
        // Prevent loop unrolling
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d (global: %d)\n", total, global_counter);
    return total != 0 ? 0 : 1;
}
