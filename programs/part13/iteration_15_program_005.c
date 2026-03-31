/* test_ddg.c - Complex loop to trigger DDG edge creation */

#include <math.h>
#include <stdlib.h>

/* Non-inlineable function to force latency modeling */
static double __attribute__((noinline)) 
external_func(double x) {
    return x * 0.75;
}

/* Complex loop with multiple dependency types */
void process_data(double* restrict src, double* restrict coeff, 
                  double* dest, double* alt, int n, int offset) {
    double acc = 1.0;
    double tmp;
    
    /* Loop with mixed dependencies */
    for (int i = 1; i < n; ++i) {
        /* 1. True Data Dependency (RAW) with high latency */
        double val = src[i] / coeff[i];          /* Division - variable latency */
        
        /* 2. Loop-carried dependency with sqrt */
        acc = sqrt(acc + val);                   /* sqrt() - high latency */
        
        /* 3. Anti-dependency (WAR) */
        tmp = dest[i-1];                         /* Read dest[i-1] */
        dest[i] = acc + tmp;                     /* Write dest[i] - anti-dep on tmp */
        
        /* 4. Output dependency (WAW) with control flow */
        if (i % 16 == 0) {
            acc = external_func(acc);            /* Function call - non-inlineable */
        } else if (i % 8 == 0) {
            acc = 1.0;                           /* Overwrites acc - output dep */
        }
        
        /* 5. Complex addressing with potential aliasing */
        int idx = i + offset + (int)(coeff[i % 4]);
        if (idx < n && idx > 0) {
            alt[idx] = src[i] * 2.0;             /* May alias with dest[] */
        }
        
        /* 6. Another loop-carried dependency */
        coeff[i] = coeff[i-1] * 0.99;            /* Distance = 1 */
    }
}

/* Helper to prevent optimizations */
static volatile double sink;

int main() {
    const int N = 1024;
    
    /* Allocate and initialize arrays */
    double* src = (double*)malloc(N * sizeof(double));
    double* coeff = (double*)malloc(N * sizeof(double));
    double* dest = (double*)malloc(N * sizeof(double));
    double* alt = (double*)malloc(N * sizeof(double));
    
    if (!src || !coeff || !dest || !alt) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        src[i] = (double)(i % 100);
        coeff[i] = 1.0 + (i % 10) * 0.1;
        dest[i] = 0.0;
        alt[i] = 0.0;
    }
    
    /* Process with different offsets to create varied addressing */
    for (int offset = -2; offset <= 2; ++offset) {
        process_data(src, coeff, dest, alt, N, offset);
    }
    
    /* Use volatile sink to prevent dead code elimination */
    sink = dest[N/2] + alt[N/4];
    
    /* Cleanup */
    free(src);
    free(coeff);
    free(dest);
    free(alt);
    
    return 0;
}
