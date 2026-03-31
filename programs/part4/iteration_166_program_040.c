/* Main test file to trigger caller-save insertion during reload */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions with side effects - prevent inlining */
extern void unknown_effect1(int, double);
extern void unknown_effect2(float, long);
extern void unknown_effect3(short, char, double);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: High pressure with integer and float variables */
int high_pressure_call1(int *ints, double *doubles) {
    /* Unpack into many scalar variables - all must be live across call */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] / (ints[8] + 1);
    int v6 = ints[9] ^ ints[10];
    int v7 = ints[11] | ints[12];
    int v8 = ints[13] & ints[14];
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] + doubles[2];
    double d3 = doubles[3] - doubles[4];
    double d4 = doubles[5] * doubles[6];
    double d5 = doubles[7] / (doubles[8] + 1.0);
    float f1 = (float)doubles[9];
    float f2 = (float)doubles[10];
    
    /* Complex computation using all variables */
    int temp1 = v1 + v2 - v3;
    double temp2 = d1 * d2 - d3;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call - all above variables must be preserved */
    unknown_effect1(v4, d4);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* More computations using the live values */
    temp1 = temp1 * v4 + v5;
    temp2 = temp2 + d5 * 2.0;
    
    /* Another call with different arguments */
    unknown_effect2(f1, (long)v6);
    
    /* Even more computations */
    int result = temp1 + v6 + v7 + v8;
    double dresult = temp2 + d1 + d2;
    
    /* Final call */
    unknown_effect3((short)result, (char)v8, dresult);
    
    return result + (int)dresult;
}

/* Function 2: Different pattern with nested control flow */
double high_pressure_call2(int *ints, double *doubles, int selector) {
    /* More variables - mixed types */
    int a1 = ints[0], a2 = ints[1], a3 = ints[2], a4 = ints[3];
    int b1 = ints[4], b2 = ints[5], b3 = ints[6], b4 = ints[7];
    double x1 = doubles[0], x2 = doubles[1], x3 = doubles[2];
    double y1 = doubles[3], y2 = doubles[4], y3 = doubles[5];
    float z1 = (float)doubles[6], z2 = (float)doubles[7];
    
    /* Complex control flow to create multiple basic blocks */
    double result = 0.0;
    
    for (int i = 0; i < 3; i++) {
        /* Switch statement creates multiple control flow paths */
        switch ((selector + i) % 4) {
            case 0:
                /* Computation using many variables */
                result += a1 * x1 + a2 * x2;
                a1 = (a1 + b1) * 2;
                x1 = x1 * 1.1;
                
                /* Call in one branch */
                asm volatile("" : : : "memory");
                unknown_effect1(a3, y1);
                asm volatile("" : : : "memory");
                
                result += b2 * y2 - a4;
                break;
                
            case 1:
                /* Different computation pattern */
                result -= b3 * x3 + a2 * z1;
                x3 = sqrt(fabs(x3)) + 1.0;
                
                /* Another call */
                asm volatile("" : : : "memory");
                unknown_effect2(z2, (long)b4);
                asm volatile("" : : : "memory");
                
                result *= 1.5;
                break;
                
            case 2:
                /* More computations */
                result = result * 0.8 + y3 * a1;
                y3 = y3 / (x2 + 1.0);
                
                /* Call with three arguments */
                asm volatile("" : : : "memory");
                unknown_effect3((short)a3, (char)b2, z1);
                asm volatile("" : : : "memory");
                
                result -= z2 * 100.0;
                break;
                
            default:
                /* Default case with yet another pattern */
                result = fmod(result, 1000.0) + a4 * b4;
                break;
        }
        
        /* Loop-carried dependency */
        a1 = (a1 + 1) % 100;
        b1 = (b1 * 2) % 255;
    }
    
    return result;
}

/* Function 3: Deeply nested calls and conditionals */
int high_pressure_call3(int *data, double *coeffs, int n) {
    /* Many local variables */
    int accum1 = 0, accum2 = 0, accum3 = 0;
    double daccum1 = 0.0, daccum2 = 0.0;
    float faccum1 = 0.0f, faccum2 = 0.0f;
    
    int t1 = data[0], t2 = data[1], t3 = data[2], t4 = data[3];
    int t5 = data[4], t6 = data[5], t7 = data[6], t8 = data[7];
    
    double c1 = coeffs[0], c2 = coeffs[1], c3 = coeffs[2];
    double c4 = coeffs[3], c5 = coeffs[4];
    
    /* Complex nested loops and conditionals */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            accum1 += t1 * t2 - t3;
            daccum1 += c1 * t4 + c2 * t5;
            
            /* Call in if branch */
            asm volatile("" : : : "memory");
            unknown_effect1(accum1, daccum1);
            asm volatile("" : : : "memory");
            
            t1 = (t1 + i) & 0xFF;
            t2 = (t2 * 3) & 0xFF;
        } 
        else if (i % 3 == 1) {
            accum2 = accum2 ^ t6 ^ t7;
            faccum1 = (float)(c3 * t8);
            
            /* Different call */
            asm volatile("" : : : "memory");
            unknown_effect2(faccum1, (long)accum2);
            asm volatile("" : : : "memory");
            
            t6 = t6 + t7;
            t7 = t7 - t6;
        } 
        else {
            accum3 = t3 * t4 + t5 * t6;
            daccum2 = c4 * accum3 + c5;
            
            /* Third call pattern */
            asm volatile("" : : : "memory");
            unknown_effect3((short)accum3, (char)t8, daccum2);
            asm volatile("" : : : "memory");
            
            t3 = t3 ^ t4;
            t4 = t4 | t3;
        }
        
        /* Loop update with dependency */
        t5 = (t5 + accum1) % 256;
        t8 = (t8 * 2 + 1) % 256;
        
        /* Periodic call in loop */
        if (i % 5 == 0) {
            asm volatile("" : : : "memory");
            unknown_effect1(t5, (double)t8);
            asm volatile("" : : : "memory");
        }
    }
    
    return accum1 + accum2 + accum3 + (int)daccum1 + (int)daccum2;
}

/* Main driver with loop calling high-pressure functions */
int main() {
    const int N = 100;
    int *int_data = malloc(20 * sizeof(int));
    double *double_data = malloc(15 * sizeof(double));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < 20; i++) {
        int_data[i] = (i * 37 + 123) % 1000;
    }
    for (int i = 0; i < 15; i++) {
        double_data[i] = (i * 1.2345 + 5.6789);
    }
    
    int total = 0;
    double total_d = 0.0;
    
    /* Loop to create multiple call sites in compilation */
    for (int iter = 0; iter < N; iter++) {
        /* Modify data slightly each iteration */
        int_data[iter % 20] += iter;
        double_data[iter % 15] += iter * 0.1;
        
        /* Call all three high-pressure functions */
        int r1 = high_pressure_call1(int_data, double_data);
        double r2 = high_pressure_call2(int_data, double_data, iter);
        int r3 = high_pressure_call3(int_data, double_data, 10 + (iter % 5));
        
        total += r1 + r3;
        total_d += r2;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d, %f\n", total, total_d);
    
    free(int_data);
    free(double_data);
    
    return 0;
}
