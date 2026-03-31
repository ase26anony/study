/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-optimize-sibling-calls -fdump-rtl-reload caller-save-test.c external.c -o test */

#include <stdio.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int, double);
extern void another_effect(float, long);
extern void mixed_effect(int, float, double, long);

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Function 1: High pressure with integer and floating point live values */
double high_pressure_call_1(int *ints, double *doubles, int n) {
    /* Declare many local variables that must be kept in registers */
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
    float f4 = f1 + f2;
    
    long l1 = (long)ints[9] * 1000;
    long l2 = (long)ints[10] << 4;
    
    /* Complex computation keeping values live */
    for (int i = 0; i < 3; i++) {
        v1 = v1 + v2 * i;
        v3 = v3 ^ v4;
        d1 = d1 * d2 + sin(d3);
        d4 = d4 / (d5 + 1.0);
        f3 = f3 * f4 - (float)d1;
        l1 = l1 + l2 / (i + 1);
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    unknown_effect(v1, d1);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all live values after the call - forcing caller-save */
    v2 = v2 + v3 * v4;
    v5 = v5 ^ (v1 >> 2);
    d2 = d2 * cos(d3) + d4;
    d5 = d5 / (d1 + 0.5);
    f4 = f4 * (float)d2 - f3;
    l2 = l2 + l1 * 2;
    
    /* Another call with different register pressure */
    another_effect(f3, l1);
    
    /* More computations */
    for (int i = 0; i < 2; i++) {
        v1 = v1 + (v2 * v3) / (v4 + 1);
        d1 = sqrt(d1 * d1 + d2 * d2);
        f1 = f1 * 2.0f - f2;
        l1 = l1 | (l2 << i);
    }
    
    /* Final mixed call */
    mixed_effect(v5, f4, d5, l2);
    
    /* Return a checksum using all variables */
    return (double)(v1 + v2 + v3 + v4 + v5) + 
           d1 + d2 + d3 + d4 + d5 + 
           (double)(f1 + f2 + f3 + f4) + 
           (double)(l1 + l2);
}

/* Function 2: Different pattern with nested conditionals */
float high_pressure_call_2(int *ints, float *floats, int selector) {
    int a1 = ints[0];
    int a2 = ints[1];
    int a3 = ints[2];
    int a4 = ints[3];
    int a5 = ints[4];
    int a6 = ints[5];
    int a7 = ints[6];
    int a8 = ints[7];
    
    float b1 = floats[0];
    float b2 = floats[1];
    float b3 = floats[2];
    float b4 = floats[3];
    float b5 = floats[4];
    float b6 = floats[5];
    
    double c1 = (double)ints[8];
    double c2 = (double)ints[9];
    double c3 = c1 * c2;
    double c4 = c1 / (c2 + 1.0);
    
    /* Complex conditional with calls at different branches */
    if (selector > 0) {
        for (int i = 0; i < selector; i++) {
            a1 = a1 * a2 + i;
            a3 = a3 ^ a4;
            b1 = b1 * b2 - (float)i;
            b3 = b3 / (b4 + 1.0f);
            c3 = c3 + sin(c4);
            
            /* Call in loop - high pressure for caller-save */
            if (i % 3 == 0) {
                unknown_effect(a1, c3);
                a2 = a2 + global_counter;
            }
        }
        
        /* Use values after loop */
        a5 = a5 * a6 + a7;
        a8 = a8 | (a1 << 2);
        b5 = b5 * b6 + b1;
        c4 = c4 * exp(c3);
        
        another_effect(b2, (long)a3);
    } else {
        /* Different computation path */
        a1 = a1 + a3 * a5;
        a2 = a2 ^ a4;
        b1 = b1 + b3 * b5;
        c3 = sqrt(c3 * c3 + c4 * c4);
        
        mixed_effect(a6, b4, c3, (long)a7);
        
        a8 = a8 + global_counter * 2;
        b6 = b6 * 2.0f - b2;
    }
    
    /* Switch statement with calls */
    switch (selector % 4) {
        case 0:
            unknown_effect(a1, (double)b1);
            break;
        case 1:
            another_effect(b2, (long)a2);
            break;
        case 2:
            mixed_effect(a3, b3, c3, (long)a4);
            break;
        default:
            unknown_effect(a5, c4);
            break;
    }
    
    /* Final computation */
    return b1 + b2 + b3 + b4 + b5 + b6 + 
           (float)(a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8) +
           (float)(c3 + c4);
}

/* Function 3: Loop with multiple call sites */
void high_pressure_loop(int iterations) {
    /* Many local variables */
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    double y1 = 1.1, y2 = 2.2, y3 = 3.3, y4 = 4.4, y5 = 5.5;
    float z1 = 1.1f, z2 = 2.2f, z3 = 3.3f;
    long l1 = 1000, l2 = 2000, l3 = 3000;
    
    for (int i = 0; i < iterations; i++) {
        /* Update all variables - keep them live */
        x1 = x1 * x2 + i;
        x3 = x3 ^ x4;
        x5 = x5 + x1 * x2;
        
        y1 = y1 * sin(y2);
        y3 = y3 + cos(y4);
        y5 = y5 / (y1 + 1.0);
        
        z1 = z1 * 1.1f - z2;
        z3 = z3 + z1 * z2;
        
        l1 = l1 + l2 * i;
        l3 = l3 | (l1 << 1);
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* Call that clobbers registers - variables must be saved */
        if (i % 2 == 0) {
            unknown_effect(x1, y1);
        } else {
            another_effect(z1, l1);
        }
        
        /* More computations after call */
        x2 = x2 + x3 * x4;
        x4 = x4 ^ x5;
        
        y2 = y2 * exp(y3);
        y4 = y4 + log(fabs(y5) + 1.0);
        
        z2 = z2 * 0.9f + z3;
        
        l2 = l2 + l3 / (i + 1);
        
        /* Another call */
        if (i % 3 == 0) {
            mixed_effect(x3, z2, y2, l2);
        }
        
        /* Update global to prevent loop elimination */
        global_counter += i;
    }
}

/* Main function with test driver */
int main() {
    /* Test data arrays */
    int int_data[20];
    double double_data[20];
    float float_data[20];
    
    /* Initialize with pattern */
    for (int i = 0; i < 20; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 1.5 + 0.3;
        float_data[i] = i * 0.7f + 0.2f;
    }
    
    double total = 0.0;
    
    /* Multiple calls to trigger different caller-save scenarios */
    for (int i = 0; i < 100; i++) {
        /* Vary parameters to prevent optimization */
        int_data[0] = i;
        double_data[0] = i * 0.1;
        
        /* Call first high-pressure function */
        total += high_pressure_call_1(int_data, double_data, i % 10);
        
        /* Call second function with different pattern */
        total += high_pressure_call_2(int_data, float_data, i % 7);
        
        /* Call loop-based function every 10 iterations */
        if (i % 10 == 0) {
            high_pressure_loop(5);
        }
        
        /* Prevent compiler from optimizing away the loop */
        asm volatile("" : "+r"(total) : : "memory");
    }
    
    /* Use the result */
    printf("Result: %f\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return (int)total % 256;
}
