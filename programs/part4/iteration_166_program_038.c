/* caller-save-test.c - Test program to trigger caller-save insertion in GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* External functions with noinline to prevent optimization */
extern void external_effect1(int, double) __attribute__((noinline));
extern void external_effect2(float, long) __attribute__((noinline));
extern double external_compute(double, double) __attribute__((noinline));

/* Function 1: High register pressure with mixed types */
double high_pressure_function1(int iter, double base) {
    /* Declare many variables of mixed types to create register pressure */
    int v1 = iter * 2;
    int v2 = iter + 100;
    int v3 = iter - 50;
    int v4 = iter * 3;
    int v5 = iter / 2;
    
    double d1 = base * 1.1;
    double d2 = base * 2.2;
    double d3 = base * 3.3;
    double d4 = base * 4.4;
    double d5 = base * 5.5;
    
    float f1 = base * 0.1f;
    float f2 = base * 0.2f;
    float f3 = base * 0.3f;
    
    long l1 = iter * 1000L;
    long l2 = iter * 2000L;
    
    /* Use all variables in computation before call */
    double sum = d1 + d2 + d3 + d4 + d5;
    int prod = v1 * v2 * v3 * v4 * v5;
    float fsum = f1 + f2 + f3;
    long lsum = l1 + l2;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    external_effect1(prod, sum);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Use all variables again after call - they must be restored */
    d1 = d1 * 2.0 + fsum;
    d2 = d2 / 2.0 - fsum;
    d3 = d3 * 3.0 + lsum;
    d4 = d4 / 3.0 - lsum;
    d5 = d5 * 4.0 + prod;
    
    v1 = v1 + (int)d1;
    v2 = v2 + (int)d2;
    v3 = v3 + (int)d3;
    v4 = v4 + (int)d4;
    v5 = v5 + (int)d5;
    
    f1 = f1 * 2.0f + v1;
    f2 = f2 * 3.0f + v2;
    f3 = f3 * 4.0f + v3;
    
    l1 = l1 * 2 + v4;
    l2 = l2 * 3 + v5;
    
    /* Another external call with different signature */
    external_effect2(f1 + f2 + f3, l1 + l2);
    
    /* Final computation using all variables */
    return d1 + d2 + d3 + d4 + d5 + v1 + v2 + v3 + v4 + v5 + f1 + f2 + f3 + l1 + l2;
}

/* Function 2: Different pattern with nested loops */
double high_pressure_function2(int start, int end) {
    double result = 0.0;
    
    for (int i = start; i < end; i++) {
        /* Create many live variables inside loop */
        double a = i * 1.234;
        double b = i * 2.345;
        double c = i * 3.456;
        double d = i * 4.567;
        double e = i * 5.678;
        
        int x = i * 11;
        int y = i * 22;
        int z = i * 33;
        int w = i * 44;
        int u = i * 55;
        
        float f1 = i * 0.111f;
        float f2 = i * 0.222f;
        float f3 = i * 0.333f;
        float f4 = i * 0.444f;
        
        /* Complex computation before call */
        double temp1 = a * b + c * d;
        double temp2 = e * a - b * c;
        int itemp = x * y + z * w;
        float ftemp = f1 * f2 - f3 * f4;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* External call - forces caller-save for all live values */
        double ext_result = external_compute(temp1, temp2);
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* Use all original variables after call */
        a = a + ext_result;
        b = b - ext_result;
        c = c * ext_result;
        d = d / (ext_result + 1.0);
        e = e + ext_result * 2.0;
        
        x = x + (int)ext_result;
        y = y - (int)ext_result;
        z = z * (int)(ext_result * 10);
        w = w / ((int)ext_result + 1);
        u = u + (int)(ext_result * 100);
        
        f1 = f1 + (float)ext_result;
        f2 = f2 - (float)ext_result;
        f3 = f3 * (float)ext_result;
        f4 = f4 / ((float)ext_result + 1.0f);
        
        /* Accumulate result */
        result += a + b + c + d + e + x + y + z + w + u + f1 + f2 + f3 + f4;
        
        /* Conditional call to create different control flow */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
            external_effect1(itemp, ftemp);
            asm volatile("" : : : "memory");
        }
    }
    
    return result;
}

/* Function 3: Complex control flow with switch statement */
double high_pressure_function3(int mode, int count) {
    double total = 0.0;
    int i = 0;
    
    while (i < count) {
        /* Create register pressure */
        double d1 = i * 1.1;
        double d2 = i * 2.2;
        double d3 = i * 3.3;
        int i1 = i * 10;
        int i2 = i * 20;
        int i3 = i * 30;
        float f1 = i * 0.5f;
        float f2 = i * 1.5f;
        long l1 = i * 100L;
        long l2 = i * 200L;
        
        /* Switch with calls in different cases */
        switch (mode) {
            case 0:
                /* Call in case 0 */
                asm volatile("" : : : "memory");
                external_effect1(i1 + i2, d1 + d2);
                asm volatile("" : : : "memory");
                d1 = d1 * 2.0;
                d2 = d2 / 2.0;
                break;
                
            case 1:
                /* Call in case 1 */
                asm volatile("" : : : "memory");
                external_effect2(f1 + f2, l1 + l2);
                asm volatile("" : : : "memory");
                f1 = f1 * 3.0f;
                f2 = f2 / 3.0f;
                break;
                
            case 2:
                /* Call in case 2 */
                asm volatile("" : : : "memory");
                external_compute(d1, d2);
                asm volatile("" : : : "memory");
                d3 = d3 * 4.0;
                i3 = i3 * 4;
                break;
                
            default:
                /* Multiple calls in default */
                asm volatile("" : : : "memory");
                external_effect1(i1, d1);
                external_effect2(f1, l1);
                asm volatile("" : : : "memory");
                break;
        }
        
        /* Use all variables after switch */
        total += d1 + d2 + d3 + i1 + i2 + i3 + f1 + f2 + l1 + l2;
        i++;
        
        /* Another call in loop with different frequency */
        if (i % 13 == 0) {
            asm volatile("" : : : "memory");
            external_compute(total, d1);
            asm volatile("" : : : "memory");
        }
    }
    
    return total;
}

/* Main function with multiple call sites */
int main() {
    double total_result = 0.0;
    const int iterations = 100;
    
    printf("Starting caller-save test...\n");
    
    /* Test function 1 in a loop */
    for (int i = 0; i < iterations; i++) {
        double result = high_pressure_function1(i, i * 1.5);
        total_result += result;
        
        /* Prevent loop optimization */
        asm volatile("" : : : "memory");
    }
    
    printf("After function1: %f\n", total_result);
    
    /* Test function 2 */
    double result2 = high_pressure_function2(0, 50);
    total_result += result2;
    printf("After function2: %f\n", total_result);
    
    /* Test function 3 with different modes */
    for (int mode = 0; mode < 4; mode++) {
        double result3 = high_pressure_function3(mode, 25);
        total_result += result3;
        printf("Mode %d: %f\n", mode, result3);
    }
    
    printf("Final result: %f\n", total_result);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
