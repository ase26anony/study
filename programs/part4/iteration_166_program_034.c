/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-inline -fdump-rtl-all caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int, double) __attribute__((noinline));
extern void another_effect(float, long) __attribute__((noinline));
extern void mixed_effect(int, float, double, long) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* External function definitions - will be in separate compilation unit */
void unknown_effect(int a, double b) {
    /* Minimal side effect that can't be optimized away */
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_counter += a + (int)b;
}

void another_effect(float a, long b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_counter += (int)a + (int)b;
}

void mixed_effect(int a, float b, double c, long d) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
    global_counter += a + (int)b + (int)c + (int)d;
}

/* Function with extreme register pressure around calls */
/* Uses 15+ live variables across multiple calls */
int high_pressure_function_1(int *ints, double *doubles, float *floats) {
    /* Unpack into many scalar variables */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] ^ ints[8];
    
    double d1 = doubles[0] * 1.5;
    double d2 = doubles[1] + doubles[2];
    double d3 = doubles[3] / doubles[4];
    double d4 = sin(doubles[5]);
    double d5 = cos(doubles[6]);
    
    float f1 = floats[0] * 2.0f;
    float f2 = floats[1] + floats[2];
    float f3 = floats[3] - floats[4];
    float f4 = floats[5] * floats[6];
    
    long l1 = (long)v1 * v2;
    long l2 = (long)v3 * v4;
    
    /* All variables are live across this call */
    /* This should force caller-save insertion */
    unknown_effect(v1, d1);
    
    /* Use all variables in computation to keep them live */
    v1 = v1 + v2 * 3;
    v2 = v2 ^ v3;
    v3 = v3 + v4 - v5;
    v4 = v4 * 2;
    v5 = v5 | 0xFF;
    
    d1 = d1 + d2 * 2.0;
    d2 = d2 - d3;
    d3 = d3 * d4;
    d4 = d4 / d5;
    d5 = d5 + 1.0;
    
    f1 = f1 + f2;
    f2 = f2 * f3;
    f3 = f3 - f4;
    f4 = f4 / 2.0f;
    
    l1 = l1 + l2;
    l2 = l2 * 2;
    
    /* Another call with different register types */
    another_effect(f1, l1);
    
    /* More computations keeping variables live */
    int v6 = v1 + v2 + v3;
    double d6 = d1 * d2 + d3;
    float f5 = f1 * f2 - f3;
    
    /* Third call with mixed arguments */
    mixed_effect(v6, f5, d6, l2);
    
    /* Final computation using all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6;
    result += (int)(d1 + d2 + d3 + d4 + d5 + d6);
    result += (int)(f1 + f2 + f3 + f4 + f5);
    result += (int)(l1 + l2);
    
    return result;
}

/* Second high-pressure function with different pattern */
float high_pressure_function_2(int *ints, double *doubles) {
    /* Different set of live variables */
    int a1 = ints[0] * 3;
    int a2 = ints[1] << 2;
    int a3 = ints[2] >> 1;
    int a4 = ints[3] | ints[4];
    int a5 = ints[5] & ints[6];
    int a6 = ints[7] % (ints[8] + 1);
    int a7 = ints[9] * ints[10];
    int a8 = ints[11] - ints[12];
    
    double b1 = doubles[0] * doubles[1];
    double b2 = doubles[2] / (doubles[3] + 1.0);
    double b3 = exp(doubles[4]);
    double b4 = log(fabs(doubles[5]) + 1.0);
    double b5 = doubles[6] * 3.14159;
    double b6 = doubles[7] + doubles[8];
    
    /* Call in conditional to create different control flow */
    if (a1 > 0) {
        unknown_effect(a1, b1);
        a2 = a2 * 2;
        b2 = b2 + 1.0;
    } else {
        another_effect((float)a2, (long)b2);
        a3 = a3 - 1;
        b3 = b3 * 2.0;
    }
    
    /* Loop with call inside - increases chance of hitting target lines */
    for (int i = 0; i < 10; i++) {
        /* All these variables must be live across the call */
        mixed_effect(a4 + i, (float)b4, b5, (long)a5);
        
        /* Modify variables to prevent optimization */
        a4 = a4 ^ i;
        a5 = a5 + a6;
        a6 = a6 * 2;
        b4 = b4 + (double)i;
        b5 = b5 * 1.01;
        b6 = b6 - 0.5;
    }
    
    /* Switch statement with calls at multiple points */
    switch (a7 % 4) {
        case 0:
            unknown_effect(a7, b6);
            a8 = a8 + 100;
            break;
        case 1:
            another_effect((float)a8, (long)(b1 * 1000));
            a7 = a7 * 2;
            break;
        case 2:
            mixed_effect(a6, (float)b2, b3, (long)a5);
            a6 = a6 >> 1;
            break;
        default:
            unknown_effect(a5, b5);
            a5 = a5 | 0xFFFF;
            break;
    }
    
    /* Final float result mixing all computations */
    float result = (float)(a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8);
    result += (float)(b1 + b2 + b3 + b4 + b5 + b6);
    
    return result;
}

/* Third function with nested loops and calls */
double high_pressure_function_3(int *ints, double *doubles, float *floats) {
    double total = 0.0;
    
    /* Outer loop */
    for (int outer = 0; outer < 5; outer++) {
        int base = outer * 3;
        
        /* Create many live variables */
        int x1 = ints[base] + outer;
        int x2 = ints[base + 1] * 2;
        int x3 = ints[base + 2] - outer;
        
        double y1 = doubles[base] * (outer + 1);
        double y2 = doubles[base + 1] / (outer + 2);
        double y3 = doubles[base + 2] + outer;
        
        float z1 = floats[base] * 1.5f;
        float z2 = floats[base + 1] + (float)outer;
        float z3 = floats[base + 2] - 0.5f;
        
        long l1 = (long)x1 * x2;
        long l2 = (long)x2 * x3;
        long l3 = (long)x3 * x1;
        
        /* Inner loop with call */
        for (int inner = 0; inner < 3; inner++) {
            /* Call with many live variables */
            if (inner == 0) {
                unknown_effect(x1 + inner, y1);
            } else if (inner == 1) {
                another_effect(z1, l1);
            } else {
                mixed_effect(x2, z2, y2, l2);
            }
            
            /* Modify variables to keep them live */
            x1 = x1 ^ inner;
            x2 = x2 + inner;
            x3 = x3 - inner;
            
            y1 = y1 * 1.1;
            y2 = y2 + 0.1;
            y3 = y3 - 0.05;
            
            z1 = z1 * 1.2f;
            z2 = z2 + 0.2f;
            z3 = z3 - 0.1f;
            
            l1 = l1 + inner;
            l2 = l2 * (inner + 1);
            l3 = l3 >> 1;
        }
        
        /* Accumulate results */
        total += x1 + x2 + x3 + y1 + y2 + y3 + z1 + z2 + z3 + l1 + l2 + l3;
    }
    
    return total;
}

int main() {
    /* Initialize test data */
    const int N = 50;
    int int_data[N];
    double double_data[N];
    float float_data[N];
    
    for (int i = 0; i < N; i++) {
        int_data[i] = (i * 37) % 100;  /* Pseudo-random pattern */
        double_data[i] = (double)(i * 41) / 10.0;
        float_data[i] = (float)(i * 43) / 5.0f;
    }
    
    int checksum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* Main loop calling high-pressure functions */
    /* This ensures reload pass sees multiple call sites */
    for (int iter = 0; iter < 100; iter++) {
        /* Modify data slightly each iteration */
        int_data[iter % N] += iter;
        double_data[iter % N] += (double)iter / 100.0;
        float_data[iter % N] += (float)iter / 50.0f;
        
        /* Call all three high-pressure functions */
        checksum += high_pressure_function_1(int_data, double_data, float_data);
        float_sum += high_pressure_function_2(int_data, double_data);
        double_sum += high_pressure_function_3(int_data, double_data, float_data);
        
        /* Add memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    /* Use results to prevent optimization */
    printf("Checksum: %d\n", checksum);
    printf("Float sum: %f\n", float_sum);
    printf("Double sum: %f\n", double_sum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
