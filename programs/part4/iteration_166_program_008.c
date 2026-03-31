/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fdump-rtl-reload -fdump-rtl-all caller-save-test.c external.c */

#include <stdio.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int, double) __attribute__((noinline));
extern void another_effect(float, long) __attribute__((noinline));
extern void mixed_effect(int, float, double, long) __attribute__((noinline));

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Memory barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* First high-pressure function with integer dominance */
int high_pressure_int_dominant(int *ints, double *doubles) {
    /* Unpack many variables to create register pressure */
    int v1 = ints[0] + 1;
    int v2 = ints[1] * 2;
    int v3 = ints[2] - ints[0];
    int v4 = ints[3] ^ ints[1];
    int v5 = ints[4] | ints[2];
    int v6 = ints[5] & ints[3];
    int v7 = ints[6] << 2;
    int v8 = ints[7] >> 1;
    int v9 = ints[8] + ints[4];
    int v10 = ints[9] * ints[5];
    
    double d1 = doubles[0] * 1.5;
    double d2 = doubles[1] / 2.0;
    double d3 = doubles[2] + doubles[0];
    double d4 = doubles[3] - doubles[1];
    
    /* Mix computations to keep values live */
    int sum1 = v1 + v2 + v3;
    double prod1 = d1 * d2 * 3.14159;
    
    /* Call with live values in call-used registers */
    COMPILER_BARRIER();
    unknown_effect(sum1, prod1);
    COMPILER_BARRIER();
    
    /* More computations using the live values */
    int sum2 = v4 + v5 + v6 + v7;
    double prod2 = d3 * d4 * 2.71828;
    
    /* Another call with different arguments */
    COMPILER_BARRIER();
    another_effect((float)prod2, (long)sum2);
    COMPILER_BARRIER();
    
    /* Final computations mixing all values */
    int result = v8 + v9 + v10;
    result += (int)(d1 + d2 + d3 + d4);
    result += sum1 + sum2;
    
    return result;
}

/* Second high-pressure function with floating-point dominance */
double high_pressure_float_dominant(float *floats, long *longs) {
    /* Many floating-point variables */
    float f1 = floats[0] * 1.1f;
    float f2 = floats[1] / 2.2f;
    float f3 = floats[2] + floats[0];
    float f4 = floats[3] - floats[1];
    float f5 = floats[4] * floats[2];
    float f6 = floats[5] / floats[3];
    float f7 = floats[6] + 3.14f;
    float f8 = floats[7] - 2.71f;
    
    /* Integer variables mixed in */
    long l1 = longs[0] + 100;
    long l2 = longs[1] * 2;
    long l3 = longs[2] ^ longs[0];
    long l4 = longs[3] | longs[1];
    
    /* Intermediate computations */
    double mix1 = (double)f1 * (double)f2 + (double)l1;
    double mix2 = (double)f3 * (double)f4 - (double)l2;
    
    /* Call in conditional context */
    COMPILER_BARRIER();
    if (mix1 > mix2) {
        mixed_effect((int)l3, f5, mix1, l4);
    } else {
        mixed_effect((int)l4, f6, mix2, l3);
    }
    COMPILER_BARRIER();
    
    /* More computations after call */
    float fsum = f5 + f6 + f7 + f8;
    long lsum = l1 + l2 + l3 + l4;
    
    /* Another call in loop-like pattern */
    for (int i = 0; i < 3; i++) {
        COMPILER_BARRIER();
        another_effect(fsum + i, lsum - i);
        COMPILER_BARRIER();
        fsum += 0.5f;
        lsum >>= 1;
    }
    
    return (double)fsum * (double)lsum + mix1 + mix2;
}

/* Third function with nested control flow */
int high_pressure_complex(int *data, double *coeffs, int n) {
    int result = 0;
    
    /* Switch statement creates multiple control flow paths */
    for (int i = 0; i < n; i++) {
        int val = data[i];
        double coeff = coeffs[i % 8];
        
        /* Many live variables in loop */
        int t1 = val * 2;
        int t2 = val + 1;
        int t3 = val ^ 0x55;
        int t4 = val << 1;
        int t5 = val >> 1;
        double d1 = coeff * 1.1;
        double d2 = coeff / 2.2;
        double d3 = coeff + 0.5;
        double d4 = coeff - 0.25;
        
        /* Complex conditional with calls */
        switch (val % 4) {
            case 0:
                COMPILER_BARRIER();
                unknown_effect(t1 + t2, d1 * d2);
                COMPILER_BARRIER();
                result += t1 + (int)d1;
                break;
            case 1:
                COMPILER_BARRIER();
                another_effect((float)d2, (long)t3);
                COMPILER_BARRIER();
                result += t2 + (int)d2;
                break;
            case 2:
                COMPILER_BARRIER();
                mixed_effect(t4, (float)d3, d4, (long)t5);
                COMPILER_BARRIER();
                result += t3 + (int)d3;
                break;
            default:
                COMPILER_BARRIER();
                unknown_effect(t5, d1 + d4);
                COMPILER_BARRIER();
                result += t4 + (int)d4;
                break;
        }
        
        /* Keep variables live across iteration */
        global_counter += (t1 > t2) ? 1 : 0;
    }
    
    return result;
}

int main() {
    /* Create test data arrays */
    int int_data[20];
    double double_data[20];
    float float_data[20];
    long long_data[20];
    
    /* Initialize with pattern */
    for (int i = 0; i < 20; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 1.5 + 0.3;
        float_data[i] = i * 0.7f + 0.1f;
        long_data[i] = i * 5L + 2L;
    }
    
    int total = 0;
    double total_d = 0.0;
    
    /* Loop to create repeated call sites */
    for (int iter = 0; iter < 100; iter++) {
        /* Modify data slightly each iteration */
        int_data[iter % 20] += iter;
        double_data[iter % 20] += iter * 0.01;
        
        /* Call different high-pressure functions */
        total += high_pressure_int_dominant(int_data, double_data);
        total_d += high_pressure_float_dominant(float_data, long_data);
        
        /* Every 10 iterations, call the complex version */
        if (iter % 10 == 0) {
            total += high_pressure_complex(int_data, double_data, 15);
        }
    }
    
    /* Use results to prevent optimization */
    printf("Result: %d, %.2f, counter: %d\n", 
           total, total_d, global_counter);
    
    return 0;
}
