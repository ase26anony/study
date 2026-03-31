#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// External functions that cannot be inlined
extern void unknown_effect1(int, double) __attribute__((noinline));
extern void unknown_effect2(float, long) __attribute__((noinline));
extern void unknown_effect3(short, char, double) __attribute__((noinline));

// Global volatile to prevent optimization
volatile int global_counter = 0;

// Function 1: High pressure with mixed integer types
int high_pressure_call1(int *ints, double *doubles) {
    // Unpack into many scalar variables
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] ^ ints[8];
    int v6 = ints[9] | ints[10];
    int v7 = ints[11] & ints[12];
    int v8 = ints[13] << 2;
    int v9 = ints[14] >> 1;
    int v10 = ints[15] + 100;
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] / 2.71828;
    double d3 = doubles[2] + doubles[3];
    double d4 = doubles[4] - doubles[5];
    double d5 = doubles[6] * doubles[7];
    float f1 = (float)doubles[8];
    float f2 = (float)doubles[9];
    
    // All these variables are live across the call
    // Force them to stay in registers
    asm volatile("" : : : "memory");
    
    // External call - clobbers call-used registers
    unknown_effect1(v1 + v2, d1 * d2);
    
    // Use all variables after call to keep them live
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)(d1 + d2 + d3 + d4 + d5);
    result += (int)(f1 * f2);
    
    // Another memory barrier
    asm volatile("" : : : "memory");
    
    return result;
}

// Function 2: Different pattern with nested control flow
double high_pressure_call2(int *ints, double *doubles, int selector) {
    // Create many live values
    double accum = 0.0;
    double temp[15];
    
    for (int i = 0; i < 15; i++) {
        temp[i] = ints[i] * doubles[i % 10];
    }
    
    // Complex conditional with calls at different points
    if (selector > 0) {
        double a = temp[0] * temp[1];
        double b = temp[2] / temp[3];
        double c = temp[4] + temp[5];
        double d = temp[6] - temp[7];
        double e = temp[8] * temp[9];
        double f = temp[10] + temp[11];
        double g = temp[12] - temp[13];
        double h = temp[14] * 2.0;
        
        // Memory barrier
        asm volatile("" : : : "memory");
        
        // Call in conditional branch
        unknown_effect2((float)a, (long)b);
        
        // Use values after call
        accum = a + b + c + d + e + f + g + h;
        
        if (selector > 10) {
            double i = temp[0] * 3.0;
            double j = temp[1] / 4.0;
            
            // Another call in nested conditional
            unknown_effect3((short)i, (char)j, accum);
            
            accum += i + j;
        }
    } else {
        // Alternative path with different live values
        for (int i = 0; i < 8; i++) {
            accum += temp[i] * (i + 1);
        }
        
        // Call in else branch
        unknown_effect1((int)accum, accum * 2.0);
    }
    
    return accum;
}

// Function 3: Loop with multiple calls
int high_pressure_call3(int *ints, double *doubles, int iterations) {
    int total = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        // Create fresh live values each iteration
        int v1 = ints[0] + iter;
        int v2 = ints[1] - iter;
        int v3 = ints[2] * iter;
        int v4 = ints[3] ^ iter;
        int v5 = ints[4] | iter;
        int v6 = ints[5] & iter;
        
        double d1 = doubles[0] * iter;
        double d2 = doubles[1] / (iter + 1);
        double d3 = doubles[2] + iter;
        double d4 = doubles[3] - iter;
        double d5 = doubles[4] * (iter * 0.5);
        double d6 = doubles[5] / (iter * 0.25 + 1);
        
        float f1 = (float)d1;
        float f2 = (float)d2;
        float f3 = (float)d3;
        float f4 = (float)d4;
        
        // Memory barrier
        asm volatile("" : : : "memory");
        
        // Call inside loop - forces repeated caller-save insertion
        if (iter % 3 == 0) {
            unknown_effect1(v1 + v2, d1 + d2);
        } else if (iter % 3 == 1) {
            unknown_effect2(f1 + f2, v3 + v4);
        } else {
            unknown_effect3((short)v5, (char)v6, d3 + d4);
        }
        
        // Use all values after call
        total += v1 + v2 + v3 + v4 + v5 + v6;
        total += (int)(d1 + d2 + d3 + d4 + d5 + d6);
        total += (int)(f1 + f2 + f3 + f4);
        
        // Another barrier
        asm volatile("" : : : "memory");
    }
    
    return total;
}

// Main function with multiple call sites
int main() {
    // Initialize test data
    int ints[20];
    double doubles[20];
    
    for (int i = 0; i < 20; i++) {
        ints[i] = rand() % 1000;
        doubles[i] = (rand() % 1000) / 10.0;
    }
    
    int result1 = 0;
    double result2 = 0.0;
    int result3 = 0;
    
    // Loop to increase pressure
    for (int outer = 0; outer < 100; outer++) {
        // Call all three high-pressure functions
        result1 += high_pressure_call1(ints, doubles);
        result2 += high_pressure_call2(ints, doubles, outer % 20);
        result3 += high_pressure_call3(ints, doubles, 5);
        
        // Modify inputs slightly
        ints[outer % 20] += 1;
        doubles[outer % 20] *= 1.01;
    }
    
    printf("Results: %d, %f, %d\n", result1, result2, result3);
    return 0;
}
