/* ddg_test.c - Complex loop to trigger DDG edge creation in GCC */

#include <math.h>
#include <stdio.h>

/* Non-inlineable function to force function call latency */
static double __attribute__((noinline)) 
external_func(double x, double y) {
    return x * 0.75 + y * 0.25;
}

/* Complex loop with multiple dependency types */
void process_data(double* restrict src, double* restrict coeff, 
                  double* dest, double* alt, int n, int offset) {
    volatile double sink_acc = 0.0;
    double acc = 1.0;
    double tmp;
    int i;
    
    /* Loop with complex dependencies */
    for (i = 1; i < n; ++i) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        acc = sqrt(acc + src[i] * coeff[i]);
        
        /* 2. Anti-dependency (WAR) - read then write same location */
        tmp = dest[i-1];               /* Read dest[i-1] */
        dest[i] = acc + tmp;           /* Write dest[i] */
        
        /* 3. Output dependency (WAW) with control flow */
        if (i % 8 == 0) {
            acc = external_func(acc, coeff[i]);  /* Overwrites acc */
        }
        
        /* 4. Complex addressing with potential aliasing */
        int idx = i + (int)(coeff[i % 4] * 2);
        if (idx < n) {
            alt[idx] = src[i] * 0.5;   /* May alias with dest */
        }
        
        /* 5. Integer division with variable divisor (high latency) */
        int divisor = (i % 16) + 1;
        int int_result = (i * 100) / divisor;
        
        /* 6. More loop-carried dependency with memory */
        dest[i] += dest[i-1] * 0.1;
        
        /* 7. Control flow creating control dependencies */
        if (int_result % 3 == 0) {
            acc = acc * 0.9;
        } else {
            acc = acc * 1.1;
        }
        
        /* 8. Function call creating edges */
        if (i % 32 == 0) {
            acc = external_func(acc, dest[i]);
        }
    }
    
    /* Prevent dead code elimination */
    sink_acc = acc + dest[n/2];
}

/* Helper to initialize arrays */
void init_arrays(double* src, double* coeff, double* dest, double* alt, int n) {
    for (int i = 0; i < n; ++i) {
        src[i] = sin(i * 0.1);
        coeff[i] = cos(i * 0.05);
        dest[i] = 0.0;
        alt[i] = 0.0;
    }
}

int main() {
    const int N = 1024;
    double src[N];
    double coeff[N];
    double dest[N];
    double alt[N];
    
    /* Initialize with deterministic values */
    init_arrays(src, coeff, dest, alt, N);
    
    /* Process with different offsets to create varied addressing */
    for (int iter = 0; iter < 3; ++iter) {
        process_data(src, coeff, dest, alt, N, iter * 10);
    }
    
    /* Use results to prevent optimization */
    volatile double final_sink = 0.0;
    for (int i = 0; i < 10; ++i) {
        final_sink += dest[i] + alt[i];
    }
    
    printf("Result: %f\n", final_sink);
    return 0;
}
