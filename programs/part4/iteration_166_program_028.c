#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// External function with unknown side effects - prevents optimization
extern void unknown_effect(int a, double b) __attribute__((noinline, noipa));
extern void unknown_effect2(float a, long b) __attribute__((noinline, noipa));
extern void unknown_effect3(short a, char b, double c) __attribute__((noinline, noipa));

// Global volatile to prevent dead code elimination
volatile int global_counter = 0;

// External function definitions (will be in separate file)
void unknown_effect(int a, double b) {
    // Use asm to prevent optimization
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_counter += a;
}

void unknown_effect2(float a, long b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_counter += (int)b;
}

void unknown_effect3(short a, char b, double c) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
    global_counter += a + b;
}

// High pressure function with many live values across call
int high_pressure_call1(int *ints, double *doubles) {
    // Unpack into many scalar variables - all must be kept live
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[0];
    int v3 = ints[2] - ints[1];
    int v4 = ints[3] | ints[2];
    int v5 = ints[4] & ints[3];
    int v6 = ints[5] ^ ints[4];
    int v7 = ints[6] << 2;
    int v8 = ints[7] >> 1;
    int v9 = ints[8] * 3;
    int v10 = ints[9] + 7;
    
    double d1 = doubles[0] * 2.0;
    double d2 = doubles[1] + doubles[0];
    double d3 = doubles[2] - doubles[1];
    double d4 = doubles[3] * doubles[2];
    double d5 = doubles[4] / doubles[3];
    double d6 = sin(doubles[5]);
    double d7 = cos(doubles[6]);
    double d8 = doubles[7] * doubles[8];
    double d9 = doubles[8] + doubles[9];
    double d10 = doubles[9] * 3.14159;
    
    // Mix integer and floating point computations
    float f1 = (float)v1 * (float)d1;
    float f2 = (float)v2 + (float)d2;
    float f3 = (float)v3 * (float)d3;
    float f4 = (float)v4 / (float)d4;
    float f5 = (float)v5 + (float)d5;
    
    // All these variables are now live across the call
    // Call external function - forces caller-save for all live values
    unknown_effect(v1 + v2, d1 + d2);
    
    // Use all variables after call - prevents them from being dead
    int sum_int = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    double sum_double = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    float sum_float = f1 + f2 + f3 + f4 + f5;
    
    // More computations mixing values
    v1 = v1 * 2 + (int)d1;
    v2 = v2 / 3 + (int)d2;
    d1 = d1 * 2.0 + v1;
    d2 = d2 / 3.0 + v2;
    
    // Second call with different signature
    unknown_effect2(f1 + f2, (long)v3 * v4);
    
    // More computations
    v3 = v3 ^ v4 | v5;
    v4 = v4 & v6 << 1;
    d3 = d3 * d4 - d5;
    d4 = d4 / d6 + d7;
    
    // Third call with three arguments
    unknown_effect3((short)v7, (char)v8, d8 + d9);
    
    // Final mixing
    int result = sum_int + (int)sum_double + (int)sum_float + v1 + v2 + v3 + v4;
    result += (int)(d1 + d2 + d3 + d4);
    
    return result;
}

// Second high pressure function with different pattern
double high_pressure_call2(int *ints, float *floats) {
    // Different set of live variables
    int a1 = ints[0] * 3;
    int a2 = ints[1] - ints[0];
    int a3 = ints[2] | ints[1];
    int a4 = ints[3] & ints[2];
    int a5 = ints[4] ^ ints[3];
    int a6 = ints[5] << 3;
    int a7 = ints[6] >> 2;
    int a8 = ints[7] * 5;
    int a9 = ints[8] + 11;
    int a10 = ints[9] - 13;
    int a11 = ints[10] * 7;
    int a12 = ints[11] / 2;
    int a13 = ints[12] % 3;
    int a14 = ints[13] | 0xFF;
    int a15 = ints[14] & 0x0F;
    
    float b1 = floats[0] * 1.5f;
    float b2 = floats[1] + floats[0];
    float b3 = floats[2] - floats[1];
    float b4 = floats[3] * floats[2];
    float b5 = floats[4] / floats[3];
    float b6 = floats[5] + 2.0f;
    float b7 = floats[6] - 1.0f;
    float b8 = floats[7] * 3.0f;
    
    double c1 = (double)a1 * 1.1;
    double c2 = (double)a2 * 2.2;
    double c3 = (double)a3 * 3.3;
    double c4 = (double)a4 * 4.4;
    double c5 = (double)a5 * 5.5;
    
    // Complex expression with many intermediates
    double temp1 = c1 * c2 + c3 - c4;
    double temp2 = c5 * c1 - c2 + c3;
    float temp3 = b1 * b2 + b3 - b4;
    float temp4 = b5 * b6 + b7 - b8;
    
    // Call in conditional context
    if (a1 > a2) {
        unknown_effect(a1 + a2, temp1 + temp2);
    } else {
        unknown_effect2(temp3 + temp4, (long)a3 * a4);
    }
    
    // Use all variables in loop
    double sum = 0.0;
    for (int i = 0; i < 3; i++) {
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
        sum += a11 + a12 + a13 + a14 + a15;
        sum += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8;
        sum += c1 + c2 + c3 + c4 + c5;
        
        // Nested call inside loop
        if (i % 2 == 0) {
            unknown_effect3((short)a6, (char)a7, sum);
        }
    }
    
    return sum;
}

// Third function with switch statement
int high_pressure_call3(int *ints, double *doubles, int selector) {
    int r1 = ints[0];
    int r2 = ints[1];
    int r3 = ints[2];
    int r4 = ints[3];
    int r5 = ints[4];
    int r6 = ints[5];
    int r7 = ints[6];
    int r8 = ints[7];
    int r9 = ints[8];
    int r10 = ints[9];
    
    double s1 = doubles[0];
    double s2 = doubles[1];
    double s3 = doubles[2];
    double s4 = doubles[3];
    double s5 = doubles[4];
    double s6 = doubles[5];
    double s7 = doubles[6];
    double s8 = doubles[7];
    double s9 = doubles[8];
    double s10 = doubles[9];
    
    // Switch with calls at different cases
    int result = 0;
    switch (selector % 4) {
        case 0:
            unknown_effect(r1 + r2, s1 + s2);
            result = r1 + r2 + (int)(s1 + s2);
            break;
        case 1:
            unknown_effect2((float)s3, (long)r3 * r4);
            result = r3 + r4 + (int)(s3 * s4);
            break;
        case 2:
            unknown_effect3((short)r5, (char)r6, s5 + s6);
            result = r5 + r6 + (int)(s5 + s6);
            break;
        case 3:
            // Multiple calls in one case
            unknown_effect(r7, s7);
            unknown_effect2((float)s8, (long)r8);
            result = r7 + r8 + (int)(s7 + s8);
            break;
    }
    
    // Use remaining variables
    result += r9 + r10 + (int)(s9 + s10);
    
    return result;
}

int main() {
    // Initialize test data
    int int_data[20];
    double double_data[20];
    float float_data[20];
    
    for (int i = 0; i < 20; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 1.5 + 0.3;
        float_data[i] = i * 0.7f + 0.1f;
    }
    
    int total = 0;
    double total_double = 0.0;
    
    // Loop with multiple high-pressure calls
    for (int iter = 0; iter < 100; iter++) {
        // Modify inputs slightly each iteration
        int_data[iter % 20] += iter;
        double_data[iter % 20] += iter * 0.01;
        float_data[iter % 20] += iter * 0.005f;
        
        // Call first high-pressure function
        total += high_pressure_call1(int_data, double_data);
        
        // Call second high-pressure function
        total_double += high_pressure_call2(int_data, float_data);
        
        // Call third high-pressure function with switch
        total += high_pressure_call3(int_data, double_data, iter);
        
        // Memory barrier to prevent reordering
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d, %f\n", total, total_double);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
