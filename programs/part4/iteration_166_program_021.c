/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fdump-rtl-reload -fdump-rtl-all caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int, double) __attribute__((noinline));
extern void another_effect(float, long) __attribute__((noinline));
extern void third_effect(double, double, int) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* External function definitions (will be in separate file or marked noinline) */
void unknown_effect(int a, double b) {
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_counter += a;
}

void another_effect(float a, long b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_counter += (int)b;
}

void third_effect(double a, double b, int c) {
    asm volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
    global_counter += c;
}

/* Function with extreme register pressure around calls */
/* Uses 15+ variables that must be live across calls */
double high_pressure_function1(int* ints, double* doubles, int n) {
    /* Unpack into many scalar variables */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] | ints[8];
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] / 2.71828;
    double d3 = doubles[2] + doubles[3];
    double d4 = doubles[4] - doubles[5];
    double d5 = doubles[6] * doubles[7];
    
    float f1 = (float)doubles[8];
    float f2 = (float)doubles[9];
    float f3 = f1 * f2;
    
    long l1 = (long)ints[9] * 1000;
    long l2 = (long)ints[10] << 4;
    
    /* Complex computation mixing all variables */
    double sum = d1;
    sum += d2 * v1;
    sum -= d3 / (v2 + 1);
    sum *= d4 + (double)v3;
    
    /* First call - many variables live across */
    unknown_effect(v1 + v2, d1 + d2);
    
    /* More computations using variables that must stay in registers */
    int v6 = v4 ^ v5;
    double d6 = d5 * 2.0;
    float f4 = f3 + 1.0f;
    
    sum += (double)v6 * d6;
    sum /= (double)(v1 | v2);
    
    /* Second call with different signature */
    another_effect(f4, l1 + l2);
    
    /* Even more computations */
    double d7 = sum * d1;
    int v7 = v3 * v4;
    float f5 = f2 * f3;
    
    /* Third call with three arguments */
    third_effect(d7, d2, v7);
    
    /* Final computation using all variables */
    double result = sum;
    result += d1 * d2 * d3;
    result -= (double)(v1 * v2 * v3) / 1000.0;
    result *= f4 * f5;
    result += (double)l1 / 1000000.0;
    result -= (double)l2 / 100000.0;
    
    /* Force all variables to be used */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                       "r"(v6), "r"(v7),
                       "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5),
                       "r"(d6), "r"(d7),
                       "r"(f1), "r"(f2), "r"(f3), "r"(f4), "r"(f5),
                       "r"(l1), "r"(l2) : "memory");
    
    return result;
}

/* Second high-pressure function with different pattern */
double high_pressure_function2(int* ints, double* doubles, int n) {
    /* Different variable usage pattern */
    int a1 = ints[0], a2 = ints[1], a3 = ints[2], a4 = ints[3], a5 = ints[4];
    int a6 = ints[5], a7 = ints[6], a8 = ints[7], a9 = ints[8], a10 = ints[9];
    
    double b1 = doubles[0], b2 = doubles[1], b3 = doubles[2];
    double b4 = doubles[3], b5 = doubles[4], b6 = doubles[5];
    double b7 = doubles[6], b8 = doubles[7], b9 = doubles[8];
    
    /* Loop with call inside - triggers multiple caller-save insertions */
    double total = 0.0;
    for (int i = 0; i < 10; i++) {
        /* Complex pre-call computation */
        double temp = b1 * a1 + b2 * a2 - b3 * a3;
        temp /= (b4 + (double)a4);
        temp *= sin(b5) + cos(b6);
        
        /* Call with many live values */
        if (i % 3 == 0) {
            unknown_effect(a5 + i, temp);
        } else if (i % 3 == 1) {
            another_effect((float)temp, (long)a6 * i);
        } else {
            third_effect(b7, b8, a7 + i);
        }
        
        /* Post-call computation */
        total += temp * (double)(a8 ^ a9) / (double)(a10 + 1);
        
        /* Modify some values to prevent optimization */
        a1 += i;
        b1 *= 1.01;
        a2 ^= i * 3;
        b2 += 0.5;
    }
    
    /* Conditional call with different live sets */
    if (total > 100.0) {
        double x = b9 * total;
        int y = a1 * a2 * a3;
        unknown_effect(y, x);
        total = x / (double)y;
    } else {
        float f = (float)total * 2.0f;
        long l = (long)a4 * a5 * a6;
        another_effect(f, l);
        total = (double)f * (double)l;
    }
    
    return total;
}

/* Third function with switch statement and calls */
float high_pressure_function3(int* ints, float* floats, int selector) {
    int x1 = ints[0], x2 = ints[1], x3 = ints[2], x4 = ints[3];
    int x5 = ints[4], x6 = ints[5], x7 = ints[6], x8 = ints[7];
    
    float y1 = floats[0], y2 = floats[1], y3 = floats[2], y4 = floats[3];
    float y5 = floats[4], y6 = floats[5], y7 = floats[6], y8 = floats[7];
    
    double accum = 0.0;
    
    /* Switch with calls in different cases */
    switch (selector % 5) {
        case 0:
            accum = (double)(x1 * x2) * y1;
            unknown_effect(x3, accum);
            accum += (double)x4 * y2;
            break;
        case 1:
            accum = (double)(x5 | x6) / y3;
            another_effect(y4, (long)x7);
            accum *= (double)x8 * y5;
            break;
        case 2:
            accum = (double)(x1 ^ x2) + y6;
            third_effect(accum, (double)y7, x3);
            accum -= (double)x4 * y8;
            break;
        case 3:
            accum = (double)x5 * y1 * y2;
            unknown_effect(x6, accum);
            accum /= (double)x7 + y3;
            another_effect(y4, (long)x8);
            break;
        default:
            accum = (double)x1 * y5 + (double)x2 * y6;
            third_effect(accum, (double)y7, x3);
            accum *= (double)x4 - y8;
            unknown_effect(x5, accum);
            break;
    }
    
    /* Nested loop with call */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float temp = y1 * i + y2 * j;
            int val = x1 * i + x2 * j;
            
            if ((i + j) % 2 == 0) {
                another_effect(temp, (long)val);
            } else {
                unknown_effect(val, (double)temp);
            }
            
            accum += (double)temp * (double)val;
        }
    }
    
    return (float)accum;
}

int main() {
    /* Initialize test data */
    int int_data[20];
    double double_data[20];
    float float_data[20];
    
    for (int i = 0; i < 20; i++) {
        int_data[i] = rand() % 1000;
        double_data[i] = (double)(rand() % 1000) / 10.0;
        float_data[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    double total = 0.0;
    
    /* Loop calling high-pressure functions - ensures reload pass sees many calls */
    for (int iter = 0; iter < 100; iter++) {
        /* Modify data slightly each iteration */
        int_data[iter % 20] += iter;
        double_data[iter % 20] *= 1.001;
        
        /* Call first high-pressure function */
        double result1 = high_pressure_function1(int_data, double_data, 20);
        
        /* Call second high-pressure function */
        double result2 = high_pressure_function2(int_data, double_data, 20);
        
        /* Call third high-pressure function */
        float result3 = high_pressure_function3(int_data, float_data, iter);
        
        total += result1 + result2 + (double)result3;
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(iter) : "memory");
    }
    
    printf("Result: %f\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return (int)total % 256;
}
