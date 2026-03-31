/* Main test file to trigger caller-save insertion during reload */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect1(int, double);
extern void unknown_effect2(float, long);
extern void unknown_effect3(double, double, double);
extern int unknown_computation(int, int, int, int, int, int, int, int);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: High pressure with integer and floating point mix */
__attribute__((noinline))
int high_pressure_function1(int a, int b, int c, int d, int e,
                           double f, double g, double h, double i, double j) {
    /* Create many intermediate values that must stay in registers */
    int v1 = a * b + c;
    int v2 = d * e - a;
    int v3 = v1 ^ v2;
    int v4 = (v3 << 3) | (v2 >> 2);
    int v5 = v4 * 0x5A827999;
    
    double d1 = f * g + h;
    double d2 = i / j * 3.14159;
    double d3 = d1 * d2 - f;
    double d4 = sin(d3) * cos(d2);
    double d5 = d4 * d4 + d3;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Call that clobbers call-used registers */
    unknown_effect1(v3, d3);
    
    /* More computations using values that were live across call */
    int v6 = v5 + v4 * 2;
    int v7 = v6 ^ (v1 & v2);
    int v8 = v7 * 0x6ED9EBA1;
    
    double d6 = d5 * 2.0 + d4;
    double d7 = exp(d6) * log(fabs(d3) + 1.0);
    double d8 = d7 * d6 - d5;
    
    /* Another call with different register pressure */
    asm volatile("" : : : "memory");
    unknown_effect2((float)d8, (long)v8);
    
    /* Final computation mixing all values */
    int result = (int)(v8 + (int)d8 + v6 + (int)(d7 * 1000.0));
    
    /* Prevent tail call optimization */
    asm volatile("" : : : "memory");
    return result;
}

/* Function 2: Different pattern with more variables */
__attribute__((noinline))
double high_pressure_function2(int x1, int x2, int x3, int x4, int x5,
                              int x6, int x7, int x8, int x9, int x10,
                              double y1, double y2, double y3, double y4, double y5,
                              double y6, double y7, double y8, double y9, double y10) {
    /* Integer computations */
    int sum1 = x1 + x2 + x3;
    int sum2 = x4 * x5 - x6;
    int sum3 = (x7 << 4) | (x8 >> 2);
    int sum4 = x9 ^ x10;
    int prod1 = sum1 * sum2;
    int prod2 = sum3 * sum4;
    int diff1 = prod1 - prod2;
    int diff2 = (diff1 * 0x9E3779B9) & 0x7FFFFFFF;
    
    /* Floating computations */
    double fsum1 = y1 + y2 + y3;
    double fsum2 = y4 * y5 / y6;
    double fsum3 = sin(y7) * cos(y8);
    double fsum4 = exp(y9) * log(fabs(y10) + 1.0);
    double fprod1 = fsum1 * fsum2;
    double fprod2 = fsum3 * fsum4;
    double fdiff1 = fprod1 - fprod2;
    double fdiff2 = fdiff1 * 3.141592653589793;
    
    /* Call in a conditional to create control flow complexity */
    asm volatile("" : : : "memory");
    if (diff1 > 0) {
        unknown_effect3(fsum1, fsum2, fsum3);
    } else {
        unknown_effect3(fsum4, fprod1, fprod2);
    }
    
    /* Loop with calls inside - creates multiple caller-save points */
    double accumulator = 0.0;
    for (int i = 0; i < 10; i++) {
        int temp1 = diff2 + i * 0x1234567;
        double temp2 = fdiff2 * (i + 1) * 0.1;
        
        /* Call inside loop - forces caller-save in loop body */
        asm volatile("" : : : "memory");
        unknown_effect1(temp1 & 0xFF, temp2);
        
        accumulator += temp2 * (temp1 % 100);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    /* Switch statement with calls at different cases */
    int selector = diff2 & 0x3;
    double final_result = 0.0;
    
    switch (selector) {
        case 0:
            asm volatile("" : : : "memory");
            unknown_effect2((float)accumulator, (long)diff1);
            final_result = accumulator * 2.0;
            break;
        case 1:
            asm volatile("" : : : "memory");
            unknown_effect3(accumulator, fdiff1, fdiff2);
            final_result = accumulator / 2.0;
            break;
        case 2:
            asm volatile("" : : : "memory");
            unknown_effect1(diff1, accumulator);
            final_result = accumulator + fdiff1;
            break;
        default:
            asm volatile("" : : : "memory");
            unknown_effect2((float)fdiff2, (long)diff2);
            final_result = accumulator - fdiff2;
            break;
    }
    
    return final_result;
}

/* Function 3: Extreme register pressure with nested calls */
__attribute__((noinline))
long extreme_pressure_function(int iterations) {
    /* Declare many variables to use all available registers */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    double da = 1.1, db = 2.2, dc = 3.3, dd = 4.4, de = 5.5;
    double df = 6.6, dg = 7.7, dh = 8.8, di = 9.9, dj = 10.10;
    double dk = 11.11, dl = 12.12, dm = 13.13, dn = 14.14, d_o = 15.15;
    
    long result = 0;
    
    /* Complex loop with multiple calls */
    for (int iter = 0; iter < iterations; iter++) {
        /* Update all variables to keep them live */
        a = a * 3 + iter;
        b = b * 5 - iter;
        c = c ^ a;
        d = d | b;
        e = e * 7 + c;
        f = f * 11 - d;
        g = g ^ e;
        h = h | f;
        i = i * 13 + g;
        j = j * 17 - h;
        k = k ^ i;
        l = l | j;
        m = m * 19 + k;
        n = n * 23 - l;
        o = o ^ m;
        p = p | n;
        
        da = da * 1.1 + iter * 0.01;
        db = db * 1.2 - iter * 0.02;
        dc = sin(dc + da);
        dd = cos(dd + db);
        de = de * 1.5 + dc;
        df = df * 1.6 - dd;
        dg = sin(dg + de);
        dh = cos(dh + df);
        di = di * 1.9 + dg;
        dj = dj * 2.0 - dh;
        dk = sin(dk + di);
        dl = cos(dl + dj);
        dm = dm * 2.3 + dk;
        dn = dn * 2.4 - dl;
        d_o = sin(d_o + dm);
        
        /* Call with many arguments - uses many argument registers */
        asm volatile("" : : : "memory");
        int call_result = unknown_computation(a, b, c, d, e, f, g, h);
        
        /* Use result in further computation */
        result += call_result * (iter + 1);
        
        /* Another call with floating point */
        asm volatile("" : : : "memory");
        unknown_effect3(da, db, dc);
        
        /* Conditional call */
        if (iter % 3 == 0) {
            asm volatile("" : : : "memory");
            unknown_effect2((float)dd, (long)call_result);
        }
        
        /* Prevent loop optimizations */
        asm volatile("" : : : "memory");
    }
    
    /* Final mixing of all variables */
    result += (long)(a + b + c + d + e + f + g + h +
                     i + j + k + l + m + n + o + p);
    result += (long)(da + db + dc + dd + de + df + dg + dh +
                     di + dj + dk + dl + dm + dn + d_o);
    
    return result;
}

/* Main function that drives everything */
int main() {
    int total_result = 0;
    double total_double = 0.0;
    
    /* Test function 1 multiple times */
    for (int i = 0; i < 100; i++) {
        int r1 = high_pressure_function1(
            i, i+1, i+2, i+3, i+4,
            1.1 * i, 2.2 * i, 3.3 * i, 4.4 * i, 5.5 * i
        );
        total_result += r1;
    }
    
    /* Test function 2 */
    for (int i = 0; i < 50; i++) {
        double r2 = high_pressure_function2(
            i, i*2, i*3, i*4, i*5, i*6, i*7, i*8, i*9, i*10,
            0.1 * i, 0.2 * i, 0.3 * i, 0.4 * i, 0.5 * i,
            0.6 * i, 0.7 * i, 0.8 * i, 0.9 * i, 1.0 * i
        );
        total_double += r2;
    }
    
    /* Test extreme pressure function */
    long r3 = extreme_pressure_function(100);
    
    /* Use results to prevent optimization */
    printf("Total result: %d\n", total_result);
    printf("Total double: %f\n", total_double);
    printf("Extreme result: %ld\n", r3);
    
    /* Reference global to prevent dead code elimination */
    printf("Global counter: %d\n", global_counter);
    
    return (total_result > 0) ? 0 : 1;
}
