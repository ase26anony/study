/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-optimize-sibling-calls caller-save-test.c caller-save-helper.c -fdump-rtl-reload -fdump-rtl-all */

#include <stdio.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect_1(int, double);
extern void unknown_effect_2(float, long);
extern void unknown_effect_3(double, double, int);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function with extreme register pressure around a call */
/* Uses 15+ variables live across call, mixing int and FP types */
int high_pressure_function_1(int a, int b, int c, int d, int e) {
    /* Create many local variables that must be kept in registers */
    int v1 = a * 3;
    int v2 = b + 7;
    int v3 = c - 5;
    int v4 = d * 2;
    int v5 = e / 3;
    double f1 = (double)a * 1.5;
    double f2 = (double)b * 2.7;
    double f3 = (double)c * 3.14;
    float f4 = (float)d * 1.618f;
    float f5 = (float)e * 2.718f;
    long l1 = (long)a * b;
    long l2 = (long)c * d;
    int v6 = v1 + v2;
    int v7 = v3 * v4;
    double f6 = f1 + f2;
    double f7 = f3 * 1.414;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Call that clobbers call-used registers */
    /* This forces caller-save for all live values */
    unknown_effect_1(v1 + v2, f1 + f2);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all variables after call - they must be restored */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7;
    result += (int)(f1 + f2 + f3 + f6 + f7);
    result += (int)f4 + (int)f5;
    result += (int)(l1 % 1000) + (int)(l2 % 1000);
    
    /* Another call with different register pressure */
    unknown_effect_2(f4, l1);
    
    return result + global_counter;
}

/* Second function with different register usage pattern */
double high_pressure_function_2(double x, double y, int n) {
    /* Create many FP variables that must stay in XMM registers */
    double d1 = x * 1.1;
    double d2 = y * 2.2;
    double d3 = x + y;
    double d4 = x - y;
    double d5 = x * y;
    double d6 = sin(x);
    double d7 = cos(y);
    float f1 = (float)x * 3.3f;
    float f2 = (float)y * 4.4f;
    int i1 = (int)x * 3;
    int i2 = (int)y * 7;
    int i3 = i1 + i2;
    int i4 = i1 * i2;
    long l1 = (long)(x * 1000);
    long l2 = (long)(y * 1000);
    
    /* Complex computation before call */
    for (int i = 0; i < 3; i++) {
        d1 = d1 * 1.01 + d2;
        d2 = d2 * 0.99 - d3;
        i1 = i1 * 2 - i2;
    }
    
    /* Call in conditional context */
    if (n > 0) {
        asm volatile("" : : : "memory");
        unknown_effect_3(d1, d2, i1);
        asm volatile("" : : : "memory");
    }
    
    /* Use all variables after call */
    double result = d1 + d2 + d3 + d4 + d5 + d6 + d7;
    result += f1 + f2;
    result += i1 + i2 + i3 + i4;
    result += l1 / 1000.0 + l2 / 1000.0;
    
    return result;
}

/* Function with calls inside a loop */
void loop_with_calls(int iterations) {
    double sum = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create register pressure within loop */
        int a = i * 3;
        int b = i * 7;
        int c = i * 11;
        double x = sin(i * 0.1);
        double y = cos(i * 0.2);
        float z = (float)i * 0.3f;
        long l = i * 1000L;
        
        /* Multiple live values across call */
        int r1 = high_pressure_function_1(a, b, c, i, i*2);
        double r2 = high_pressure_function_2(x, y, i);
        
        /* Call with mixed arguments */
        asm volatile("" : : : "memory");
        unknown_effect_2(z, l);
        asm volatile("" : : : "memory");
        
        sum += r1 + r2 + z;
        
        /* Conditional call to create different BB structure */
        if (i % 7 == 0) {
            unknown_effect_1(r1 % 100, sum);
        }
    }
    
    global_counter += (int)sum;
}

/* Main function with multiple call sites */
int main() {
    /* Initialize test data */
    int int_data[20];
    double double_data[20];
    
    for (int i = 0; i < 20; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = sin(i * 0.5) * 100.0;
    }
    
    /* Create register pressure in main too */
    int total = 0;
    double acc = 0.0;
    
    /* Multiple calls with high register pressure */
    for (int iter = 0; iter < 100; iter++) {
        /* Many live variables across calls */
        int v1 = int_data[iter % 20];
        int v2 = int_data[(iter + 1) % 20];
        double d1 = double_data[iter % 20];
        double d2 = double_data[(iter + 2) % 20];
        float f1 = (float)d1 * 0.5f;
        long l1 = (long)v1 * v2;
        
        /* Call sequence creating need for caller-save */
        int r1 = high_pressure_function_1(v1, v2, iter, iter*2, iter*3);
        double r2 = high_pressure_function_2(d1, d2, iter);
        
        /* External call with live values */
        asm volatile("" : : : "memory");
        unknown_effect_3(d1, d2, r1);
        asm volatile("" : : : "memory");
        
        total += r1 + (int)r2;
        acc += r2 + f1;
        
        /* Nested loop with call */
        if (iter % 10 == 0) {
            loop_with_calls(5);
        }
    }
    
    printf("Result: %d, %f\n", total, acc);
    printf("Global counter: %d\n", global_counter);
    
    return total > 0 ? 0 : 1;
}
