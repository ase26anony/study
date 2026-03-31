/* Test program to trigger caller-save insertion during reload */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int, double) __attribute__((noinline));
extern void another_effect(float, long) __attribute__((noinline));
extern void third_effect(double, double, int) __attribute__((noinline));

/* Global volatile to prevent call elimination */
volatile int global_counter = 0;

/* External function definitions (will be in separate file or marked noinline) */
void unknown_effect(int a, double b) {
    /* Minimal side effect that can't be optimized away */
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_counter += a;
}

void another_effect(float f, long l) {
    asm volatile("" : : "r"(f), "r"(l) : "memory");
    global_counter += (int)f;
}

void third_effect(double d1, double d2, int i) {
    asm volatile("" : : "r"(d1), "r"(d2), "r"(i) : "memory");
    global_counter += i;
}

/* Function with extreme register pressure around calls */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Unpack into many scalar variables - all must be kept live */
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
    double d4 = doubles[3] * 0.5;
    double d5 = doubles[4] / 2.0;
    double d6 = sin(doubles[5]);
    double d7 = cos(doubles[6]);
    double d8 = doubles[7] * doubles[0];
    double d9 = doubles[8] + 3.14159;
    double d10 = doubles[9] - 2.71828;
    
    /* Mix computations to force register allocation */
    double mix1 = d1 * v1 + d2 * v2;
    double mix2 = d3 * v3 - d4 * v4;
    int mix3 = v5 * (int)d5 + v6 * (int)d6;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Call with many live values - will need caller-save */
    unknown_effect(v7, d7);
    
    /* Use all variables after call to keep them live */
    v1 += (int)(mix1 * 0.5);
    v2 -= (int)(mix2 * 0.3);
    v3 ^= mix3;
    d1 = d8 * v8 + d9;
    d2 = d10 * v9 - d1;
    
    /* Another call with different arguments */
    another_effect((float)d2, (long)v10);
    
    /* More computations mixing all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10);
    result += (int)mix1 + (int)mix2 + mix3;
    
    return result;
}

/* Second high-pressure function with different pattern */
double high_pressure_call_2(float *floats, long *longs) {
    /* Create many floating point live values */
    float f1 = floats[0] * 1.1f;
    float f2 = floats[1] + 2.2f;
    float f3 = floats[2] - 3.3f;
    float f4 = floats[3] / 4.4f;
    float f5 = floats[4] * 5.5f;
    float f6 = floats[5] + 6.6f;
    float f7 = floats[6] - 7.7f;
    float f8 = floats[7] / 8.8f;
    
    long l1 = longs[0] * 11;
    long l2 = longs[1] + 22;
    long l3 = longs[2] - 33;
    long l4 = longs[3] | 0xFF;
    long l5 = longs[4] & 0xFFFF;
    long l6 = longs[5] ^ 0xAAAA;
    long l7 = longs[6] << 2;
    long l8 = longs[7] >> 1;
    
    /* Complex floating point computation */
    double d1 = (double)f1 * l1;
    double d2 = (double)f2 * l2;
    double d3 = (double)f3 * l3;
    double d4 = (double)f4 * l4;
    
    /* Call in conditional context */
    if (l1 > 100) {
        third_effect(d1, d2, (int)l1);
    } else {
        another_effect(f5, l5);
    }
    
    /* Keep all variables live */
    f1 += (float)d3;
    f2 -= (float)d4;
    l1 += (long)(f3 * 100);
    l2 -= (long)(f4 * 50);
    
    /* Nested loop with call inside */
    for (int i = 0; i < 3; i++) {
        double temp = (double)f5 * (i + 1);
        unknown_effect((int)temp, (double)l6);
        f5 += 1.0f;
        l6 += i;
    }
    
    /* Switch statement with calls at different cases */
    switch ((int)l7 % 4) {
        case 0:
            third_effect(d1, d2, 100);
            break;
        case 1:
            another_effect(f6, l7);
            break;
        case 2:
            unknown_effect((int)f7, (double)l8);
            break;
        default:
            third_effect(d3, d4, 200);
    }
    
    /* Final computation using all variables */
    double result = d1 + d2 + d3 + d4;
    result += f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
    result += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
    
    return result;
}

/* Third function with loop containing calls */
int high_pressure_loop(int iterations, int *data, double *coeffs) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create live values inside loop */
        int a = data[i * 4];
        int b = data[i * 4 + 1];
        int c = data[i * 4 + 2];
        int d = data[i * 4 + 3];
        
        double x = coeffs[i * 3];
        double y = coeffs[i * 3 + 1];
        double z = coeffs[i * 3 + 2];
        
        /* Complex computation forcing register pressure */
        double t1 = x * a + y * b;
        double t2 = y * c + z * d;
        int t3 = a * b + c * d;
        double t4 = sin(x) * cos(y);
        
        /* Call with many live values - triggers caller-save */
        if (i % 2 == 0) {
            unknown_effect(t3, t1);
        } else {
            another_effect((float)t2, (long)t3);
        }
        
        /* Use results after call */
        sum += (int)(t1 + t2 + t4) + t3;
        a += (int)t1;
        b += (int)t2;
        
        /* Another call in the loop */
        if (i % 3 == 0) {
            third_effect(t1, t2, t3);
        }
        
        /* More computations to keep values live */
        x += t4;
        y -= t1 * 0.5;
        z *= 1.1;
    }
    
    return sum;
}

int main() {
    /* Initialize test data */
    int int_data[40];
    double double_data[30];
    float float_data[24];
    long long_data[24];
    
    for (int i = 0; i < 40; i++) {
        int_data[i] = rand() % 1000;
    }
    for (int i = 0; i < 30; i++) {
        double_data[i] = (rand() % 1000) / 10.0;
    }
    for (int i = 0; i < 24; i++) {
        float_data[i] = (rand() % 1000) / 10.0f;
        long_data[i] = rand() % 10000;
    }
    
    int total = 0;
    double total_d = 0.0;
    
    /* Loop with high pressure calls */
    for (int i = 0; i < 100; i++) {
        /* Modify data slightly each iteration */
        int_data[0] += i;
        double_data[0] += i * 0.1;
        
        /* Call first high-pressure function */
        total += high_pressure_call_1(int_data, double_data);
        
        /* Call second high-pressure function */
        total_d += high_pressure_call_2(float_data, long_data);
        
        /* Call loop-based high-pressure function */
        total += high_pressure_loop(10, int_data, double_data);
    }
    
    printf("Result: %d, %.2f\n", total, total_d);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
