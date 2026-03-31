/* Main test file to trigger caller-save insertion during reload */
#include <stdio.h>
#include <math.h>

/* External functions with noinline to prevent optimization */
extern void unknown_effect1(int, double) __attribute__((noinline));
extern void unknown_effect2(float, long) __attribute__((noinline));
extern void unknown_effect3(double, double, int) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Function 1: High pressure with mixed integer types */
int high_pressure_int_call(int *ints, double *doubles) {
    /* Unpack many variables to create register pressure */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] / (ints[8] + 1);
    int v6 = ints[9] << 2;
    int v7 = ints[10] & 0xFF;
    int v8 = ints[11] | 0x1000;
    int v9 = ints[12] ^ ints[13];
    int v10 = ints[14] % (ints[15] + 1);
    
    /* Floating point variables to use XMM registers */
    double d1 = doubles[0] * 1.5;
    double d2 = doubles[1] + doubles[2];
    double d3 = doubles[3] - doubles[4];
    double d4 = doubles[5] * doubles[6];
    double d5 = doubles[7] / (doubles[8] + 1.0);
    
    /* Mix computations to keep values live */
    int sum1 = v1 + v2 + v3;
    double prod1 = d1 * d2 * d3;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    unknown_effect1(v4, d4);
    
    /* More computations using pre-call values */
    int sum2 = v5 + v6 + v7 + v8;
    double prod2 = d4 * d5;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    /* Second external call */
    unknown_effect2(v9, (long)v10);
    
    /* Final computations mixing all values */
    int final_int = sum1 + sum2 + v9 + v10;
    double final_double = prod1 + prod2 + d1 + d2 + d3 + d4 + d5;
    
    /* Use results to prevent elimination */
    global_counter += final_int;
    return final_int + (int)final_double;
}

/* Function 2: Different pattern with more floating point */
double high_pressure_float_call(float *floats, long *longs) {
    /* Many float/double variables */
    float f1 = floats[0] * 2.0f;
    float f2 = floats[1] + floats[2];
    float f3 = floats[3] - floats[4];
    float f4 = floats[5] * floats[6];
    float f5 = floats[7] / (floats[8] + 1.0f);
    float f6 = floats[9] * 3.14f;
    float f7 = floats[10] + 2.71f;
    float f8 = floats[11] - 1.41f;
    
    /* Double precision variables */
    double d1 = (double)f1 * 1.5;
    double d2 = (double)f2 + 2.5;
    double d3 = (double)f3 * 3.5;
    double d4 = (double)f4 / 4.5;
    double d5 = (double)f5 + 5.5;
    
    /* Long integers */
    long l1 = longs[0] * 2;
    long l2 = longs[1] + longs[2];
    long l3 = longs[3] - longs[4];
    long l4 = longs[5] | 0xFFFF;
    long l5 = longs[6] & 0xAAAA;
    
    /* Complex computation before call */
    double complex_val = d1 * d2 + d3 * d4 + d5;
    float float_sum = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
    
    /* Loop with call inside - triggers multiple caller-save points */
    for (int i = 0; i < 3; i++) {
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* Call with many arguments - uses multiple call-used registers */
        unknown_effect3(d1 + i, d2 - i, (int)l1);
        
        /* Modify values in loop to keep them live */
        d1 += 0.1;
        d2 -= 0.1;
        l1 += i;
        
        /* Conditional to create control flow complexity */
        if (i % 2 == 0) {
            unknown_effect1((int)l2, d3);
            d3 *= 1.1;
        } else {
            unknown_effect2(f6, l3);
            f6 += 0.5f;
        }
    }
    
    /* Final computation using all live values */
    double result = complex_val + float_sum + d1 + d2 + d3 + d4 + d5;
    result += (double)(l1 + l2 + l3 + l4 + l5);
    
    global_counter += (int)result;
    return result;
}

/* Function 3: Nested conditionals with calls */
int complex_control_flow(int *data, int n) {
    int a = data[0];
    int b = data[1];
    int c = data[2];
    int d = data[3];
    int e = data[4];
    int f = data[5];
    int g = data[6];
    int h = data[7];
    int i = data[8];
    int j = data[9];
    
    double x = (double)a * 1.1;
    double y = (double)b * 2.2;
    double z = (double)c * 3.3;
    
    /* Switch statement with calls in different cases */
    int result = 0;
    for (int iter = 0; iter < n; iter++) {
        switch (iter % 4) {
            case 0:
                asm volatile("" : : : "memory");
                unknown_effect1(a + iter, x);
                result += a + b + (int)x;
                x += 0.5;
                break;
            case 1:
                asm volatile("" : : : "memory");
                unknown_effect2((float)y, (long)c);
                result += c + d + (int)y;
                y *= 1.1;
                break;
            case 2:
                asm volatile("" : : : "memory");
                unknown_effect3(z, x + y, e);
                result += e + f + (int)z;
                z -= 0.2;
                break;
            case 3:
                /* Multiple calls in same basic block */
                asm volatile("" : : : "memory");
                unknown_effect1(g, x);
                unknown_effect2((float)z, (long)h);
                result += g + h + i + j;
                break;
        }
        
        /* Keep variables modified and live across iterations */
        a += 1;
        b -= 1;
        c *= 2;
        d /= 2;
    }
    
    global_counter += result;
    return result;
}

/* Main function with loop calling high-pressure functions */
int main() {
    /* Initialize test data */
    int int_data[20];
    double double_data[10];
    float float_data[15];
    long long_data[10];
    
    for (int i = 0; i < 20; i++) {
        int_data[i] = i * 3 + 1;
    }
    for (int i = 0; i < 10; i++) {
        double_data[i] = i * 1.5 + 0.3;
    }
    for (int i = 0; i < 15; i++) {
        float_data[i] = i * 0.7f + 0.1f;
    }
    for (int i = 0; i < 10; i++) {
        long_data[i] = i * 5L + 2L;
    }
    
    int total = 0;
    
    /* Loop to trigger caller-save in reload pass */
    for (int i = 0; i < 100; i++) {
        /* Modify inputs slightly each iteration */
        int_data[0] += i;
        double_data[0] += i * 0.01;
        
        /* Call different high-pressure functions */
        total += high_pressure_int_call(int_data, double_data);
        
        if (i % 3 == 0) {
            total += (int)high_pressure_float_call(float_data, long_data);
        }
        
        if (i % 5 == 0) {
            total += complex_control_flow(int_data, 8);
        }
    }
    
    printf("Result: %d (global: %d)\n", total, global_counter);
    return total != 0 ? 0 : 1;
}
