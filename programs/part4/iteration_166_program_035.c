/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-optimize-sibling-calls caller-save-test.c external.c -o test */

#include <stdio.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect(int, double);
extern void another_effect(float, long);
extern void mixed_effect(int, float, double, long);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function with high register pressure around a call */
int high_pressure_function1(int a, int b, int c, int d, int e, 
                           double f, double g, double h, double i, double j,
                           float k, float l, float m, float n, float o) {
    /* Force all parameters to be live across the call */
    int v1 = a * b + c;
    int v2 = d * e + v1;
    double v3 = f * g + h;
    double v4 = i * j + v3;
    float v5 = k * l;
    float v6 = m * n * o;
    
    /* Additional computations to create more live values */
    int v7 = v1 + v2;
    int v8 = v7 * a - b;
    double v9 = v3 * v4 / f;
    double v10 = v9 + g - h;
    float v11 = v5 * 2.0f;
    float v12 = v6 / 3.0f;
    int v13 = v8 ^ c;
    double v14 = v10 * 1.5;
    float v15 = v11 + v12;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    unknown_effect(v7, v9);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all live values after the call */
    v1 = v2 + v13;
    v3 = v4 * v14;
    v5 = v15 + v11 - v12;
    
    /* More computations mixing values */
    int result = v1 + (int)v3 + (int)v5 + v8 + v13;
    result += (int)(v9 * 100.0) + (int)(v10 * 50.0);
    
    return result;
}

/* Second function with different register pressure pattern */
double high_pressure_function2(int iter) {
    /* Create many local variables that must be kept in registers */
    double d1 = iter * 1.1;
    double d2 = iter * 2.2;
    double d3 = iter * 3.3;
    double d4 = iter * 4.4;
    double d5 = iter * 5.5;
    float f1 = iter * 1.1f;
    float f2 = iter * 2.2f;
    float f3 = iter * 3.3f;
    int i1 = iter * 11;
    int i2 = iter * 22;
    int i3 = iter * 33;
    int i4 = iter * 44;
    long l1 = iter * 111L;
    long l2 = iter * 222L;
    
    /* Complex computation before call */
    for (int j = 0; j < 3; j++) {
        d1 = d1 * d2 + d3;
        d2 = d2 * d3 - d4;
        d3 = d3 / d5 + d1;
        f1 = f1 * f2 + f3;
        i1 = i1 ^ i2 + i3;
        l1 = l1 + l2 * j;
    }
    
    /* Call with mixed arguments */
    another_effect(f1, l1);
    
    /* Use values after call */
    double sum = d1 + d2 + d3 + d4 + d5;
    sum += f1 + f2 + f3;
    sum += i1 + i2 + i3 + i4;
    sum += l1 + l2;
    
    return sum;
}

/* Function with nested calls in control flow */
int complex_control_flow(int start, int end) {
    int total = 0;
    
    for (int i = start; i < end; i++) {
        /* Create register pressure inside loop */
        double a = i * 1.234;
        double b = i * 5.678;
        float c = i * 3.14f;
        float d = i * 2.71f;
        int e = i * 7;
        int f = i * 13;
        long g = i * 17L;
        long h = i * 19L;
        
        /* Conditional with calls on both paths */
        if (i % 3 == 0) {
            /* Path 1: Call with many live values */
            a = a * b + c;
            b = b / a - d;
            e = e ^ f + i;
            g = g * h;
            
            mixed_effect(e, c, a, g);
            
            total += (int)(a + b + c + d) + e + f;
        } else if (i % 3 == 1) {
            /* Path 2: Different call pattern */
            c = c * d * i;
            d = d / c + i;
            f = f * e - i;
            h = h + g * i;
            
            unknown_effect(f, a);
            
            total += (int)(c * 100.0f) + f + (int)h;
        } else {
            /* Path 3: Yet another pattern */
            a = sin(a) * cos(b);
            b = tan(a) * atan(b);
            
            another_effect(c, g);
            
            total += (int)(a * b * 1000.0);
        }
        
        /* More computations to keep values live */
        a = a + 1.0;
        b = b - 1.0;
        c = c * 1.1f;
        d = d / 1.1f;
        e = e + 2;
        f = f - 2;
        g = g * 2L;
        h = h / 2L;
    }
    
    return total;
}

/* Main function that exercises all patterns */
int main() {
    int result1 = 0;
    double result2 = 0.0;
    int result3 = 0;
    
    /* Loop to ensure caller-save insertion happens multiple times */
    for (int i = 0; i < 100; i++) {
        /* Call first high-pressure function */
        result1 += high_pressure_function1(
            i, i+1, i+2, i+3, i+4,
            i*1.1, i*2.2, i*3.3, i*4.4, i*5.5,
            i*1.1f, i*2.2f, i*3.3f, i*4.4f, i*5.5f
        );
        
        /* Call second function */
        result2 += high_pressure_function2(i);
        
        /* Call complex control flow function every 10 iterations */
        if (i % 10 == 0) {
            result3 += complex_control_flow(i, i + 5);
        }
        
        /* Prevent loop optimization */
        asm volatile("" : : : "memory");
    }
    
    printf("Results: %d, %f, %d\n", result1, result2, result3);
    return 0;
}
