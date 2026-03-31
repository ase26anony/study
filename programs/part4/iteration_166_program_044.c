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
    /* Unpack many variables to create register pressure */
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
    double d6 = sin(doubles[8]);
    double d7 = cos(doubles[9]);
    double d8 = doubles[10] * doubles[11];
    double d9 = doubles[12] + 1.0;
    double d10 = doubles[13] - 0.5;
    
    /* Use all variables in computations before call */
    int sum_int = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    double sum_double = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Call external function - many registers live across this call */
    unknown_effect(sum_int, sum_double);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use variables after call in complex ways */
    int result1 = (v1 * v2) / (v3 + 1);
    int result2 = (v4 ^ v5) | (v6 & v7);
    double result3 = (d1 * d2) + (d3 - d4);
    double result4 = sin(d5) * cos(d6);
    
    /* Nested conditional with another call */
    if (result1 > 0) {
        another_effect((float)result3, (long)result2);
        asm volatile("" : : : "memory");
        result1 += v8 * v9;
    } else {
        mixed_effect(v10, result1, d7, d8);
        asm volatile("" : : : "memory");
        result1 -= v10;
    }
    
    /* Loop with call inside */
    for (int i = 0; i < 3; i++) {
        double temp = d9 * i + d10;
        unknown_effect(i, temp);
        asm volatile("" : : : "memory");
        result1 += (int)temp;
    }
    
    return result1 + (int)result4;
}

/* Function 2: Different pattern with more floating point pressure */
double high_pressure_call_2(float *floats, long *longs) {
    float f1 = floats[0] * 1.1f;
    float f2 = floats[1] / 2.2f;
    float f3 = floats[2] + floats[3];
    float f4 = floats[4] - floats[5];
    float f5 = floats[6] * floats[7];
    float f6 = floats[8] + 3.14f;
    float f7 = floats[9] - 1.618f;
    float f8 = floats[10] * 2.0f;
    float f9 = floats[11] / 3.0f;
    float f10 = floats[12] + floats[13];
    
    long l1 = longs[0] * 3;
    long l2 = longs[1] + longs[2];
    long l3 = longs[3] - longs[4];
    long l4 = longs[5] | longs[6];
    long l5 = longs[7] & longs[8];
    long l6 = longs[9] ^ longs[10];
    long l7 = longs[11] << 1;
    long l8 = longs[12] >> 2;
    long l9 = longs[13] + 999;
    long l10 = longs[14] * 2;
    
    /* Complex computation mixing types */
    double mixed1 = (double)f1 * (double)l1;
    double mixed2 = (double)f2 / (double)(l2 + 1);
    double mixed3 = sin((double)f3) * (double)l3;
    double mixed4 = cos((double)f4) + (double)l4;
    
    asm volatile("" : : : "memory");
    
    /* Call with mixed arguments */
    mixed_effect((int)l5, (int)l6, mixed1, mixed2);
    
    asm volatile("" : : : "memory");
    
    /* Switch statement with calls at multiple points */
    int choice = l7 % 4;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            unknown_effect((int)f5, mixed3);
            asm volatile("" : : : "memory");
            result = f6 * l8;
            break;
        case 1:
            another_effect(f7, l9);
            asm volatile("" : : : "memory");
            result = f8 / (l10 + 1);
            break;
        case 2:
            mixed_effect((int)f9, (int)l1, (double)f10, mixed4);
            asm volatile("" : : : "memory");
            result = mixed1 + mixed2;
            break;
        default:
            unknown_effect((int)l2, mixed3);
            asm volatile("" : : : "memory");
            result = mixed3 * mixed4;
            break;
    }
    
    /* Small loop with call */
    for (int i = 0; i < 2; i++) {
        float temp_f = f1 * i + f2;
        long temp_l = l1 * i + l2;
        another_effect(temp_f, temp_l);
        asm volatile("" : : : "memory");
        result += temp_f * temp_l;
    }
    
    return result;
}

/* Function 3: Extreme pressure with many live values */
int extreme_pressure(int a, double b, float c, long d) {
    /* Create many intermediate values */
    int i1 = a * 2;
    int i2 = a + 100;
    int i3 = a - 50;
    int i4 = a * a;
    int i5 = a ^ 0xFFFF;
    int i6 = a | 0xFF00;
    int i7 = a & 0x00FF;
    int i8 = a << 3;
    int i9 = a >> 2;
    int i10 = a * 3;
    int i11 = a + 200;
    int i12 = a - 75;
    int i13 = a * 4;
    int i14 = a ^ 0xAAAA;
    int i15 = a | 0x5555;
    
    double d1 = b * 2.0;
    double d2 = b / 3.0;
    double d3 = b + 1.0;
    double d4 = b - 0.5;
    double d5 = sin(b);
    double d6 = cos(b);
    double d7 = b * b;
    double d8 = sqrt(b + 1.0);
    double d9 = b * 3.14159;
    double d10 = b / 2.71828;
    
    float f1 = c * 1.5f;
    float f2 = c / 2.5f;
    float f3 = c + 0.1f;
    float f4 = c - 0.2f;
    float f5 = c * c;
    
    long l1 = d * 2;
    long l2 = d + 1000;
    long l3 = d - 500;
    long l4 = d | 0xF0F0;
    long l5 = d & 0x0F0F;
    
    /* Use all variables before call */
    int int_sum = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
    int_sum += i11 + i12 + i13 + i14 + i15;
    double double_sum = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    float float_sum = f1 + f2 + f3 + f4 + f5;
    long long_sum = l1 + l2 + l3 + l4 + l5;
    
    asm volatile("" : : : "memory");
    
    /* Multiple calls in sequence */
    unknown_effect(int_sum, double_sum);
    asm volatile("" : : : "memory");
    
    another_effect(float_sum, long_sum);
    asm volatile("" : : : "memory");
    
    mixed_effect(i1, i2, d1, d2);
    asm volatile("" : : : "memory");
    
    /* Complex conditional with nested calls */
    if (int_sum > 1000) {
        for (int j = 0; j < 2; j++) {
            double temp = d3 * j + d4;
            unknown_effect(j * 10, temp);
            asm volatile("" : : : "memory");
            int_sum += (int)temp;
        }
    } else {
        int k = 0;
        while (k < 2) {
            float temp_f = f3 * k + f4;
            long temp_l = l1 * k + l2;
            another_effect(temp_f, temp_l);
            asm volatile("" : : : "memory");
            int_sum -= (int)temp_f;
            k++;
        }
    }
    
    /* Final computation using all variables */
    double final_d = (d5 * d6) + (d7 / d8) - d9 + d10;
    float final_f = (f1 * f2) - (f3 / f4) + f5;
    long final_l = (l1 | l2) & (l3 ^ l4) + l5;
    
    asm volatile("" : : : "memory");
    
    mixed_effect(int_sum, (int)final_f, final_d, (double)final_l);
    
    return int_sum + (int)final_d + (int)final_f + (int)final_l;
}

int main() {
    /* Initialize test data */
    int ints[20];
    double doubles[20];
    float floats[20];
    long longs[20];
    
    for (int i = 0; i < 20; i++) {
        ints[i] = rand() % 1000;
        doubles[i] = (rand() % 1000) / 10.0;
        floats[i] = (rand() % 1000) / 100.0f;
        longs[i] = rand() % 10000;
    }
    
    int total = 0;
    double total_d = 0.0;
    
    /* Loop with calls to trigger caller-save in reload */
    for (int iter = 0; iter < 100; iter++) {
        /* Modify inputs slightly each iteration */
        ints[0] += iter;
        doubles[0] += iter * 0.1;
        
        /* Call high pressure functions */
        int result1 = high_pressure_call_1(ints, doubles);
        double result2 = high_pressure_call_2(floats, longs);
        int result3 = extreme_pressure(iter, iter * 1.5, iter * 0.5f, iter * 10L);
        
        total += result1 + result3;
        total_d += result2;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d, %f\n", total, total_d);
    return 0;
}
