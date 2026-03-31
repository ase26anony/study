#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// External function with unknown side effects - prevents optimization
extern void unknown_effect(int, double) __attribute__((noinline));
extern void another_effect(float, int, double) __attribute__((noinline));
extern void mixed_effect(long, float, double, int) __attribute__((noinline));

// Global volatile to prevent dead code elimination
volatile int global_counter = 0;

// External function definitions (will be in separate file or marked noinline)
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

// High pressure function with many live values across call
int high_pressure_call(int *ints, double *doubles) {
    // Unpack into many scalar variables - all must be kept live
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] ^ ints[8];
    int v6 = ints[9] | ints[10];
    int v7 = ints[11] & ints[12];
    int v8 = ints[13] << 2;
    int v9 = ints[14] >> 1;
    int v10 = ints[15] + 42;
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] / 2.71828;
    double d3 = doubles[2] + doubles[3];
    double d4 = doubles[4] - doubles[5];
    double d5 = doubles[6] * doubles[7];
    double d6 = sin(doubles[8]);
    double d7 = cos(doubles[9]);
    double d8 = doubles[10] * doubles[11];
    double d9 = doubles[12] + 1.618;
    double d10 = doubles[13] - 0.577;
    
    // Mix computations to create register pressure
    float f1 = (float)(v1 * d1);
    float f2 = (float)(v2 + d2);
    float f3 = (float)(v3 * d3);
    float f4 = (float)(v4 / d4);
    float f5 = (float)(v5 + d5);
    
    // Memory barrier to prevent reordering
    asm volatile("" : : : "memory");
    
    // Call with many live values - forces caller-save insertion
    unknown_effect(v1 + v2, d1 * d2);
    
    // Use all variables after call to keep them live
    int sum1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    double sum2 = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    float sum3 = f1 + f2 + f3 + f4 + f5;
    
    // Another call with different arguments
    another_effect(sum3, sum1, sum2);
    
    // More computations mixing values
    long l1 = (long)v1 * (long)v2;
    long l2 = (long)v3 * (long)v4;
    
    // Third call with mixed types
    mixed_effect(l1 + l2, f1 * f2, d1 - d2, v5 ^ v6);
    
    // Final computation using all values
    int result = (int)(sum1 + (int)sum2 + (int)sum3 + (l1 & 0xFFFF) + (l2 & 0xFFFF));
    
    // Memory barrier before return
    asm volatile("" : : : "memory");
    
    return result;
}

// Second high pressure function with different pattern
double high_pressure_call2(float *floats, int *ints, double *doubles) {
    // Create many live values of mixed types
    float f1 = floats[0] * 1.1f;
    float f2 = floats[1] / 2.2f;
    float f3 = floats[2] + floats[3];
    float f4 = floats[4] - floats[5];
    float f5 = floats[6] * floats[7];
    float f6 = floats[8] + floats[9];
    float f7 = floats[10] - floats[11];
    float f8 = floats[12] * floats[13];
    float f9 = floats[14] / floats[15];
    float f10 = floats[16] + 3.3f;
    
    int i1 = ints[0] * 3;
    int i2 = ints[1] + ints[2];
    int i3 = ints[3] - ints[4];
    int i4 = ints[5] | ints[6];
    int i5 = ints[7] & ints[8];
    int i6 = ints[9] ^ ints[10];
    int i7 = ints[11] << 1;
    int i8 = ints[12] >> 2;
    int i9 = ints[13] * ints[14];
    int i10 = ints[15] + 99;
    
    double d1 = doubles[0];
    double d2 = doubles[1];
    double d3 = doubles[2];
    double d4 = doubles[3];
    double d5 = doubles[4];
    
    // Complex computation before call
    double temp1 = f1 * i1 + d1;
    double temp2 = f2 * i2 - d2;
    double temp3 = f3 * i3 * d3;
    double temp4 = f4 / i4 + d4;
    double temp5 = f5 * i5 - d5;
    
    // Call in conditional to create control flow
    if (i1 > 0) {
        unknown_effect(i1 + i2, temp1 + temp2);
    } else {
        another_effect(f1 + f2, i3 + i4, temp3 + temp4);
    }
    
    // Use values after call
    double sum = 0.0;
    for (int j = 0; j < 5; j++) {
        // Loop creates multiple call sites
        switch (j % 3) {
            case 0:
                mixed_effect(i1 + j, f1, d1, i2);
                sum += f1 * j;
                break;
            case 1:
                unknown_effect(i3 + j, d2 * j);
                sum += f2 * j;
                break;
            case 2:
                another_effect(f3, i4 + j, d3);
                sum += f3 * j;
                break;
        }
        
        // Modify live values in loop
        f1 += 0.1f;
        i1 += 1;
        d1 *= 1.01;
    }
    
    // Final computation using all live values
    double result = temp1 + temp2 + temp3 + temp4 + temp5 + sum +
                    f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 +
                    i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 +
                    d1 + d2 + d3 + d4 + d5;
    
    return result;
}

// Third function with nested loops and calls
void high_pressure_call3(int iterations) {
    // Many local variables
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    float f = 1.0f, g = 2.0f, h = 3.0f, i = 4.0f, j = 5.0f;
    double k = 1.0, l = 2.0, m = 3.0, n = 4.0, o = 5.0;
    long p = 100, q = 200, r = 300, s = 400, t = 500;
    
    // Outer loop
    for (int x = 0; x < iterations; x++) {
        // Modify variables
        a += x; b -= x; c *= (x % 10) + 1;
        d ^= x; e |= x;
        
        f *= 1.1f; g /= 1.2f; h += 0.1f * x;
        i -= 0.2f * x; j = sinf(j + 0.1f);
        
        k = cos(k + 0.01); l = tan(l * 1.01);
        m += 0.001 * x; n -= 0.002 * x; o *= 1.001;
        
        p <<= 1; q >>= 1; r += x; s -= x; t ^= x;
        
        // Inner loop with call
        for (int y = 0; y < 3; y++) {
            // Call with current values - all must be preserved
            if ((x + y) % 2 == 0) {
                mixed_effect(p + y, f + y, k + y, a + y);
            } else {
                another_effect(g + y, b + y, l + y);
            }
            
            // More computations
            int tmp1 = a * b + c - d;
            float tmp2 = f * g - h / i;
            double tmp3 = k * l / m + n;
            
            // Another call
            unknown_effect(tmp1 + y, tmp3 + y);
            
            // Update variables
            a = (a + tmp1) % 1000;
            f = fmodf(f + tmp2, 10.0f);
            k = fmod(k + tmp3, 20.0);
        }
        
        // Memory barrier
        asm volatile("" : : : "memory");
    }
    
    // Final call with all accumulated values
    mixed_effect(p + q + r + s + t, 
                 f + g + h + i + j,
                 k + l + m + n + o,
                 a + b + c + d + e);
}

int main() {
    const int N = 100;
    int ints[20];
    double doubles[20];
    float floats[20];
    
    // Initialize test data
    for (int i = 0; i < 20; i++) {
        ints[i] = rand() % 1000;
        doubles[i] = (rand() % 1000) / 10.0;
        floats[i] = (rand() % 1000) / 10.0f;
    }
    
    int total = 0;
    double total_d = 0.0;
    
    // Loop with many calls to trigger caller-save insertion
    for (int i = 0; i < N; i++) {
        // Modify inputs slightly each iteration
        for (int j = 0; j < 20; j++) {
            ints[j] = (ints[j] * 13 + 17) % 1000;
            doubles[j] = fmod(doubles[j] * 1.1, 100.0);
            floats[j] = fmodf(floats[j] * 1.2f, 100.0f);
        }
        
        // Call first high pressure function
        total += high_pressure_call(ints, doubles);
        
        // Call second function
        total_d += high_pressure_call2(floats, ints, doubles);
        
        // Every 10 iterations, call third function
        if (i % 10 == 0) {
            high_pressure_call3(5);
        }
    }
    
    // Use results to prevent optimization
    printf("Result: %d, %.6f, counter: %d\n", 
           total, total_d, global_counter);
    
    return 0;
}
