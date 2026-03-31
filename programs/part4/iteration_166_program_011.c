/* Test program to trigger caller-save insertion during reload */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* External function with unknown side effects - prevents inlining */
extern void unknown_effect(int a, double b) __attribute__((noinline));
extern void another_effect(float f, long l) __attribute__((noinline));
extern void mixed_effect(int i1, int i2, double d1, double d2) __attribute__((noinline));

/* Function 1: High register pressure with integer and floating point */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Unpack into many scalar variables to create register pressure */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] ^ ints[8];
    int v6 = ints[9] | ints[10];
    int v7 = ints[11] & ints[12];
    int v8 = ints[13] << 2;
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] + doubles[2];
    double d3 = doubles[3] - doubles[4];
    double d4 = doubles[5] * doubles[6];
    double d5 = doubles[7] / (doubles[8] + 1.0);
    double d6 = sin(doubles[9]);
    double d7 = cos(doubles[10]);
    double d8 = exp(doubles[11] * 0.5);
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Call external function - all above variables must be preserved */
    unknown_effect(v1 + v2, d1 * d2);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all variables in complex computation to keep them live */
    int result = v1 + v3 - v5;
    result += (v2 * v4) >> 1;
    result ^= v6 & v7;
    result |= v8;
    
    double dresult = d1 + d3 - d5;
    dresult *= d2 * d4;
    dresult += sin(d6) * cos(d7);
    dresult /= d8 + 1.0;
    
    /* Mix integer and floating point results */
    return result + (int)(dresult * 1000.0);
}

/* Function 2: Different pattern with nested control flow */
float high_pressure_call_2(int *ints, float *floats) {
    int a = ints[0];
    int b = ints[1];
    int c = ints[2];
    int d = ints[3];
    int e = ints[4];
    int f = ints[5];
    int g = ints[6];
    int h = ints[7];
    
    float fa = floats[0];
    float fb = floats[1];
    float fc = floats[2];
    float fd = floats[3];
    float fe = floats[4];
    float ff = floats[5];
    float fg = floats[6];
    float fh = floats[7];
    
    /* Complex conditional with calls inside */
    float result = 0.0f;
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            asm volatile("" : : : "memory");
            another_effect(fa, (long)a * b);
            asm volatile("" : : : "memory");
            result += fa * fb + a - b;
        } else if (i == 1) {
            asm volatile("" : : : "memory");
            another_effect(fc, (long)c * d);
            asm volatile("" : : : "memory");
            result += fc * fd + c - d;
        } else {
            asm volatile("" : : : "memory");
            another_effect(fe, (long)e * f);
            asm volatile("" : : : "memory");
            result += fe * ff + e - f;
        }
        
        /* Use remaining variables */
        result += (fg * fh) / (g + h + 1);
    }
    
    return result;
}

/* Function 3: Mixed types with switch statement */
double high_pressure_call_3(int *ints, double *doubles, int selector) {
    /* Create many live values */
    int i1 = ints[0] * 2;
    int i2 = ints[1] + 1;
    int i3 = ints[2] - 1;
    int i4 = ints[3] | 0xFF;
    int i5 = ints[4] & 0x7F;
    int i6 = ints[5] ^ ints[6];
    int i7 = ints[7] << 1;
    int i8 = ints[8] >> 2;
    
    double d1 = doubles[0] * 2.0;
    double d2 = doubles[1] + 1.0;
    double d3 = doubles[2] - 1.0;
    double d4 = doubles[3] / 2.0;
    double d5 = doubles[4] * doubles[5];
    double d6 = sqrt(doubles[6]);
    double d7 = log(doubles[7] + 1.0);
    double d8 = pow(doubles[8], 1.5);
    
    double result = 0.0;
    
    /* Switch with calls in different cases */
    switch (selector % 4) {
        case 0:
            asm volatile("" : : : "memory");
            mixed_effect(i1, i2, d1, d2);
            asm volatile("" : : : "memory");
            result = d1 + d2 + i1 + i2;
            break;
        case 1:
            asm volatile("" : : : "memory");
            mixed_effect(i3, i4, d3, d4);
            asm volatile("" : : : "memory");
            result = d3 + d4 + i3 + i4;
            break;
        case 2:
            asm volatile("" : : : "memory");
            mixed_effect(i5, i6, d5, d6);
            asm volatile("" : : : "memory");
            result = d5 + d6 + i5 + i6;
            break;
        case 3:
            asm volatile("" : : : "memory");
            mixed_effect(i7, i8, d7, d8);
            asm volatile("" : : : "memory");
            result = d7 + d8 + i7 + i8;
            break;
    }
    
    /* Use all variables in final computation */
    result += (i1 * i3 * i5 * i7) / 1000000.0;
    result += (d2 * d4 * d6 * d8) / 1000.0;
    
    return result;
}

/* Main function with loop calling high-pressure functions */
int main() {
    const int N = 100;
    int int_data[20];
    double double_data[20];
    float float_data[20];
    
    /* Initialize test data */
    for (int i = 0; i < 20; i++) {
        int_data[i] = rand() % 1000;
        double_data[i] = (rand() % 1000) / 10.0;
        float_data[i] = (rand() % 1000) / 10.0f;
    }
    
    int total = 0;
    
    /* Loop to ensure caller-save insertion happens multiple times */
    for (int iter = 0; iter < N; iter++) {
        /* Modify data slightly each iteration */
        int_data[iter % 20] += iter;
        double_data[iter % 20] += iter * 0.1;
        float_data[iter % 20] += iter * 0.1f;
        
        /* Call all high-pressure functions */
        int r1 = high_pressure_call_1(int_data, double_data);
        float r2 = high_pressure_call_2(int_data, float_data);
        double r3 = high_pressure_call_3(int_data, double_data, iter);
        
        total += r1 + (int)r2 + (int)r3;
        
        /* Prevent loop unrolling */
        if (iter % 10 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
