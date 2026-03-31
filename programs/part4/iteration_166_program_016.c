/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fdump-rtl-reload -fdump-rtl-all caller-save-test.c external.c */

#include <stdio.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int a, double b);
extern void another_effect(float f, long long l);
extern void mixed_effect(int i1, int i2, double d1, double d2);

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Function 1: High pressure with integer and floating point values */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Unpack many variables to create register pressure */
    int v1 = ints[0] + 1;
    int v2 = ints[1] * 2;
    int v3 = ints[2] - ints[0];
    int v4 = ints[3] ^ ints[1];
    int v5 = ints[4] | 0xFF;
    int v6 = ints[5] << 2;
    int v7 = ints[6] >> 1;
    int v8 = ints[7] & 0x7F;
    
    double d1 = doubles[0] * 1.5;
    double d2 = doubles[1] / 2.0;
    double d3 = doubles[2] + doubles[0];
    double d4 = doubles[3] - doubles[1];
    double d5 = doubles[4] * M_PI;
    double d6 = doubles[5] / M_E;
    double d7 = doubles[6] + 3.14159;
    double d8 = doubles[7] - 2.71828;
    
    /* Mix computations to keep values live */
    int sum_int = v1 + v2 + v3 + v4;
    double sum_double = d1 + d2 + d3 + d4;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call with many live values */
    unknown_effect(sum_int, sum_double);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* More computations using live values */
    v1 = v1 * v5 + v6;
    v2 = v2 ^ v7 | v8;
    d1 = d1 * d5 + d6;
    d2 = d2 - d7 * d8;
    
    /* Second external call */
    another_effect((float)d1, (long long)v1);
    
    /* Final computations */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    result += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);
    
    global_counter++;
    return result;
}

/* Function 2: Different pattern with nested control flow */
double high_pressure_call_2(int *ints, double *doubles, int selector) {
    int a1 = ints[0];
    int a2 = ints[1];
    int a3 = ints[2];
    int a4 = ints[3];
    int a5 = ints[4];
    int a6 = ints[5];
    int a7 = ints[6];
    int a8 = ints[7];
    int a9 = ints[8];
    int a10 = ints[9];
    
    double b1 = doubles[0];
    double b2 = doubles[1];
    double b3 = doubles[2];
    double b4 = doubles[3];
    double b5 = doubles[4];
    double b6 = doubles[5];
    double b7 = doubles[6];
    double b8 = doubles[7];
    double b9 = doubles[8];
    double b10 = doubles[9];
    
    /* Complex control flow to force multiple caller-save points */
    double result = 0.0;
    
    for (int i = 0; i < 10; i++) {
        /* Varying computations based on loop index */
        int temp_int = a1 * i + a2;
        double temp_double = b1 * i + b2;
        
        if (i % 3 == 0) {
            temp_int += a3 ^ a4;
            temp_double += b3 * b4;
            
            /* Call in conditional branch */
            mixed_effect(temp_int, a5, temp_double, b5);
            
            asm volatile("" : : : "memory");
            
            temp_int = temp_int * a6 / (a7 + 1);
            temp_double = temp_double - b6 * b7;
        } else if (i % 3 == 1) {
            temp_int = temp_int | a8;
            temp_double = temp_double / (b8 + 1.0);
            
            /* Another call site */
            unknown_effect(temp_int, temp_double);
            
            asm volatile("" : : : "memory");
            
            temp_int = temp_int & a9;
            temp_double = temp_double + b9;
        } else {
            temp_int = temp_int - a10;
            temp_double = temp_double * b10;
            
            /* Third call site */
            another_effect((float)temp_double, (long long)temp_int);
            
            asm volatile("" : : : "memory");
        }
        
        result += temp_int + temp_double;
        
        /* Modify some values to keep them live across iterations */
        a1 = (a1 + 1) & 0xFF;
        b1 = b1 * 0.99;
    }
    
    global_counter += 2;
    return result;
}

/* Function 3: Switch statement with calls */
int high_pressure_call_3(int *ints, double *doubles, char mode) {
    int r1 = ints[0];
    int r2 = ints[1];
    int r3 = ints[2];
    int r4 = ints[3];
    int r5 = ints[4];
    int r6 = ints[5];
    
    double s1 = doubles[0];
    double s2 = doubles[1];
    double s3 = doubles[2];
    double s4 = doubles[3];
    double s5 = doubles[4];
    double s6 = doubles[5];
    
    int result = 0;
    
    switch (mode) {
        case 'A':
            r1 = r1 * r2 + r3;
            s1 = s1 + s2 * s3;
            
            unknown_effect(r1, s1);
            asm volatile("" : : : "memory");
            
            result = r1 + (int)s1;
            break;
            
        case 'B':
            r4 = r4 ^ r5 | r6;
            s4 = s4 - s5 / s6;
            
            another_effect((float)s4, (long long)r4);
            asm volatile("" : : : "memory");
            
            result = r4 - (int)s4;
            break;
            
        case 'C':
            r2 = r2 * r3 + r4;
            r5 = r5 ^ r6;
            s2 = s2 * s3 + s4;
            s5 = s5 - s6;
            
            mixed_effect(r2, r5, s2, s5);
            asm volatile("" : : : "memory");
            
            result = r2 + r5 + (int)(s2 + s5);
            break;
            
        default:
            /* Multiple calls in default case */
            for (int i = 0; i < 3; i++) {
                r1 += r2;
                s1 *= s2;
                
                if (i == 1) {
                    unknown_effect(r1, s1);
                } else {
                    another_effect((float)s1, (long long)r1);
                }
                
                asm volatile("" : : : "memory");
                
                r2 = r2 ^ r3;
                s2 = s2 / s3;
            }
            result = r1 + r2 + (int)(s1 + s2);
    }
    
    global_counter++;
    return result;
}

int main() {
    int int_data[20];
    double double_data[20];
    
    /* Initialize test data */
    for (int i = 0; i < 20; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 2.5 + 0.1;
    }
    
    int total = 0;
    
    /* Loop with calls to trigger caller-save insertion */
    for (int iter = 0; iter < 100; iter++) {
        /* Modify data slightly each iteration */
        int_data[iter % 20] = iter;
        double_data[iter % 20] = iter * 0.7;
        
        /* Call different high-pressure functions */
        total += high_pressure_call_1(int_data, double_data);
        
        double result2 = high_pressure_call_2(int_data, double_data, iter % 3);
        total += (int)result2;
        
        char modes[] = {'A', 'B', 'C', 'D'};
        total += high_pressure_call_3(int_data, double_data, modes[iter % 4]);
    }
    
    printf("Result: %d (counter: %d)\n", total, global_counter);
    return total != 0 ? 0 : 1;
}
