/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-optimize-sibling-calls -fdump-rtl-all caller-save-test.c external.c */

#include <stdio.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int, double);
extern void another_effect(float, long);
extern void mixed_effect(int, float, double, long);

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Function 1: High pressure with integer and floating point values */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Unpack many variables to create register pressure */
    int v1 = ints[0] + 1;
    int v2 = ints[1] * 2;
    int v3 = ints[2] - ints[0];
    int v4 = ints[3] | ints[1];
    int v5 = ints[4] & 0xFF;
    int v6 = ints[5] << 2;
    int v7 = ints[6] >> 1;
    int v8 = ints[7] ^ ints[2];
    int v9 = ints[8] + ints[3];
    int v10 = ints[9] * 3;
    
    double d1 = doubles[0] * 1.5;
    double d2 = doubles[1] + 2.0;
    double d3 = doubles[2] / 3.0;
    double d4 = doubles[3] - doubles[0];
    double d5 = doubles[4] * doubles[1];
    double d6 = sin(doubles[2]);
    double d7 = cos(doubles[3]);
    double d8 = doubles[4] + doubles[5];
    double d9 = doubles[6] * 2.71828;
    double d10 = doubles[7] / 3.14159;
    
    /* Use all variables in computation before call */
    int int_sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    double double_sum = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call - clobbers call-used registers */
    unknown_effect(int_sum, double_sum);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use variables after call - forces them to be live across call */
    v1 = v1 * 2 + v2;
    v3 = v3 | v4;
    v5 = v5 ^ v6;
    v7 = v7 + v8 - v9;
    v10 = v10 * v1;
    
    d1 = d1 * 2.0 + d2;
    d3 = d3 / d4;
    d5 = d5 * d6;
    d7 = d7 + d8 - d9;
    d10 = d10 * 3.14159;
    
    /* More computations mixing ints and doubles */
    double mixed = (double)v1 + d1;
    mixed += (double)v3 * d3;
    mixed += (double)v5 / d5;
    mixed += (double)v7 * d7;
    mixed += (double)v10 + d10;
    
    /* Another call with different arguments */
    another_effect((float)d2, (long)v2);
    
    /* Final computation */
    return (int)(mixed + int_sum + double_sum);
}

/* Function 2: Different pattern with nested control flow */
float high_pressure_call_2(int *ints, float *floats, int n) {
    float result = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Create many live values inside loop */
        int a1 = ints[i] + i;
        int a2 = ints[i+1] * 2;
        int a3 = ints[i+2] - i;
        int a4 = ints[i+3] | a1;
        int a5 = ints[i+4] & 0x7F;
        
        float f1 = floats[i] * 1.1f;
        float f2 = floats[i+1] + 2.2f;
        float f3 = floats[i+2] / 3.3f;
        float f4 = floats[i+3] - floats[i];
        float f5 = floats[i+4] * floats[i+1];
        
        /* Conditional to create control flow complexity */
        if (i % 3 == 0) {
            /* Call within conditional block */
            mixed_effect(a1, f1, (double)f2, (long)a2);
            
            /* More computations */
            a1 = a1 * a2 + a3;
            f1 = f1 * f2 - f3;
        } else if (i % 3 == 1) {
            /* Different call pattern */
            unknown_effect(a3, (double)f4);
            
            a4 = a4 ^ a5;
            f4 = f4 / f5 * 2.0f;
        } else {
            /* Third call pattern */
            another_effect(f3, (long)a4);
            
            a5 = a5 + a1 * 2;
            f5 = f5 + f1 * 3.0f;
        }
        
        /* Use all values after calls */
        result += (float)a1 + f1 + (float)a2 + f2 + (float)a3 + f3 + 
                  (float)a4 + f4 + (float)a5 + f5;
        
        /* Memory barrier every few iterations */
        if (i % 5 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return result;
}

/* Function 3: Switch statement with calls */
double high_pressure_call_3(int *ints, double *doubles, int selector) {
    double total = 0.0;
    
    /* Many live variables */
    int w1 = ints[0];
    int w2 = ints[1];
    int w3 = ints[2];
    int w4 = ints[3];
    int w5 = ints[4];
    int w6 = ints[5];
    int w7 = ints[6];
    int w8 = ints[7];
    
    double z1 = doubles[0];
    double z2 = doubles[1];
    double z3 = doubles[2];
    double z4 = doubles[3];
    double z5 = doubles[4];
    double z6 = doubles[5];
    double z7 = doubles[6];
    double z8 = doubles[7];
    
    /* Complex switch with calls */
    switch (selector % 4) {
        case 0:
            unknown_effect(w1 + w2, z1 + z2);
            w3 = w3 * w4;
            z3 = z3 * z4;
            break;
        case 1:
            another_effect((float)z5, (long)w5);
            w6 = w6 | w7;
            z6 = z6 / z7;
            break;
        case 2:
            mixed_effect(w2, (float)z2, z3, (long)w3);
            w8 = w8 ^ w1;
            z8 = z8 - z1;
            break;
        case 3:
            unknown_effect(w4 * w5, z4 * z5);
            w7 = w7 + w6;
            z7 = z7 + z6;
            break;
    }
    
    /* Use all variables after switch */
    total = (double)(w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8) +
            (z1 + z2 + z3 + z4 + z5 + z6 + z7 + z8);
    
    /* Another call */
    asm volatile("" : : : "memory");
    unknown_effect((int)total, total);
    asm volatile("" : : : "memory");
    
    /* Final computation using all variables */
    return total + (double)w1 * z1 + (double)w2 * z2 + 
           (double)w3 * z3 + (double)w4 * z4;
}

int main() {
    /* Initialize test data */
    int int_data[100];
    float float_data[100];
    double double_data[100];
    
    for (int i = 0; i < 100; i++) {
        int_data[i] = i * 3 + 1;
        float_data[i] = i * 1.5f;
        double_data[i] = i * 2.71828;
    }
    
    int result1 = 0;
    float result2 = 0.0f;
    double result3 = 0.0;
    
    /* Loop to create multiple call sites */
    for (int iter = 0; iter < 100; iter++) {
        /* Modify data slightly each iteration */
        int_data[iter % 50] += iter;
        float_data[iter % 50] += iter * 0.1f;
        double_data[iter % 50] += iter * 0.01;
        
        /* Call all three high-pressure functions */
        result1 += high_pressure_call_1(int_data, double_data);
        
        if (iter % 10 == 0) {
            result2 += high_pressure_call_2(int_data, float_data, 20);
        }
        
        if (iter % 7 == 0) {
            result3 += high_pressure_call_3(int_data, double_data, iter);
        }
        
        /* Prevent loop optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Use results to prevent elimination */
    printf("Results: %d, %f, %f\n", result1, result2, result3);
    global_counter = result1 + (int)result2 + (int)result3;
    
    return global_counter > 0 ? 0 : 1;
}
