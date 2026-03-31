/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-optimize-sibling-calls -fdump-rtl-reload caller-save-test.c external.c */

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
    int v3 = ints[2] - ints[3];
    int v4 = ints[4] ^ ints[5];
    int v5 = ints[6] | ints[7];
    int v6 = ints[8] & ints[9];
    
    double d1 = doubles[0] * 1.5;
    double d2 = doubles[1] / 2.0;
    double d3 = doubles[2] + doubles[3];
    double d4 = doubles[4] - doubles[5];
    double d5 = sin(doubles[6]);
    double d6 = cos(doubles[7]);
    
    float f1 = (float)d1 * 0.5f;
    float f2 = (float)d2 + 1.0f;
    
    /* All these variables are live across the call */
    /* Use them in computations to ensure they stay in registers */
    int temp1 = v1 * v2 + v3;
    double temp2 = d1 * d2 + d3;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    unknown_effect(temp1, temp2);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all variables after the call - they must be restored */
    int result = v1 + v2 - v3 + v4 ^ v5 & v6;
    result += (int)(d1 + d2 - d3 + d4 * d5 / d6);
    result += (int)(f1 * f2);
    
    /* Modify global to prevent optimization */
    global_counter += result;
    
    return result;
}

/* Function 2: Different pattern with nested control flow */
double high_pressure_call_2(int *ints, float *floats, int n) {
    double sum = 0.0;
    
    /* Loop with calls inside - increases chance of caller-save insertion */
    for (int i = 0; i < n; i++) {
        /* Create many live values */
        int a = ints[i] + i;
        int b = ints[i+1] * 2;
        int c = ints[i+2] - 3;
        int d = ints[i+3] ^ 0xFF;
        
        float f1 = floats[i] * 2.0f;
        float f2 = floats[i+1] + 1.0f;
        float f3 = floats[i+2] - 0.5f;
        float f4 = floats[i+3] / 3.0f;
        
        double d1 = (double)f1 * 1.5;
        double d2 = (double)f2 / 2.0;
        double d3 = (double)f3 + 0.25;
        double d4 = (double)f4 - 1.0;
        
        /* Conditional to create complex CFG */
        if (i % 3 == 0) {
            /* Use variables before call */
            int temp = a * b + c - d;
            float ftemp = f1 + f2 * f3 / f4;
            
            asm volatile("" : : : "memory");
            
            /* Call with mixed arguments - uses multiple register classes */
            mixed_effect(temp, ftemp, d1, (long)d2);
            
            asm volatile("" : : : "memory");
            
            /* Use variables after call */
            sum += a + b + c + d;
            sum += f1 + f2 + f3 + f4;
        } else if (i % 3 == 1) {
            /* Different call pattern */
            asm volatile("" : : : "memory");
            
            another_effect(f1, (long)a);
            
            asm volatile("" : : : "memory");
            
            sum += d1 * d2 - d3 / d4;
        } else {
            /* Yet another pattern */
            asm volatile("" : : : "memory");
            
            unknown_effect(b, d3);
            
            asm volatile("" : : : "memory");
            
            sum += (double)(a ^ b & c | d);
        }
        
        /* Mix in more computations to keep values live */
        d1 = d1 * 2.0 + sin(d2);
        d2 = d2 / 1.5 + cos(d3);
    }
    
    return sum;
}

/* Function 3: Switch statement with multiple call sites */
int high_pressure_call_3(int *ints, double *doubles, int mode) {
    int result = 0;
    
    /* Create many live variables */
    int v1 = ints[0];
    int v2 = ints[1];
    int v3 = ints[2];
    int v4 = ints[3];
    int v5 = ints[4];
    int v6 = ints[5];
    int v7 = ints[6];
    int v8 = ints[7];
    
    double d1 = doubles[0];
    double d2 = doubles[1];
    double d3 = doubles[2];
    double d4 = doubles[3];
    double d5 = doubles[4];
    double d6 = doubles[5];
    
    float f1 = (float)doubles[6];
    float f2 = (float)doubles[7];
    
    /* Switch creates multiple basic blocks with calls */
    switch (mode % 4) {
        case 0:
            /* Computation before call */
            result = v1 * v2 + v3 - v4;
            asm volatile("" : : : "memory");
            unknown_effect(result, d1);
            asm volatile("" : : : "memory");
            /* Use remaining variables */
            result += v5 + v6 + v7 + v8;
            result += (int)(d2 + d3 + d4);
            break;
            
        case 1:
            /* Different computation pattern */
            result = v2 ^ v3 | v4 & v5;
            asm volatile("" : : : "memory");
            another_effect(f1, (long)v6);
            asm volatile("" : : : "memory");
            result += (int)(d5 * d6 + f2);
            break;
            
        case 2:
            /* Multiple calls in same case */
            result = v7 - v8 + v1;
            asm volatile("" : : : "memory");
            mixed_effect(result, f1, d3, (long)v2);
            asm volatile("" : : : "memory");
            result += v3 * v4;
            asm volatile("" : : : "memory");
            unknown_effect(v5, d4);
            asm volatile("" : : : "memory");
            result += (int)(d5 / d6);
            break;
            
        case 3:
            /* Complex computation with call in middle */
            result = (v1 + v2) * (v3 - v4);
            double temp = d1 * d2 + d3 / d4;
            asm volatile("" : : : "memory");
            unknown_effect(result, temp);
            asm volatile("" : : : "memory");
            result += (int)(temp + d5 + d6);
            result ^= v5 | v6 & v7 ^ v8;
            break;
    }
    
    return result;
}

int main() {
    /* Initialize test data */
    int int_data[100];
    double double_data[100];
    float float_data[100];
    
    for (int i = 0; i < 100; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 1.5 + 0.3;
        float_data[i] = i * 0.7f + 0.1f;
    }
    
    int total = 0;
    
    /* Loop to create pressure and multiple call sites */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the calls to hit different paths */
        total += high_pressure_call_1(int_data + iter, double_data + iter);
        
        if (iter % 10 == 0) {
            double sum = high_pressure_call_2(int_data, float_data, 20);
            total += (int)sum;
        }
        
        if (iter % 7 == 0) {
            total += high_pressure_call_3(int_data + iter * 2, 
                                         double_data + iter * 2, 
                                         iter);
        }
    }
    
    printf("Result: %d (global: %d)\n", total, global_counter);
    return total != 0 ? 0 : 1;
}
