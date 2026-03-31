/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimization */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
double complex_recurrence(double *a, double *b, double *c, int n) {
    double sum = 1.0;
    double prod = 2.0;
    double x = 3.14159;
    
    /* Loop 1: Multiple carried dependencies with high-latency operations */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] * 1.01 + b[i];
        
        /* Another distance-1 dependency with division (high latency) */
        x = x / 3.14159 * c[i] + 0.5;
        
        /* Cross-dependency between sum and prod */
        sum = sum + a[i] * x;
        prod = prod * (sum + 1.0) / 2.71828;
        
        /* Artificial resource contention: multiple FP operations */
        b[i] = (b[i-1] + sin(x)) * cos(prod);
    }
    
    return sum + prod + x;
}

/* Integer loop with carried dependencies */
int integer_recurrence(int *arr, int *brr, int n) {
    int acc1 = arr[0];
    int acc2 = brr[0];
    
    /* Loop with distance-1 dependencies and integer multiplication */
    for (int i = 1; i < n; i++) {
        /* Carried dependency chain */
        acc1 = acc1 * 7 + arr[i];
        acc2 = acc2 * 3 + brr[i];
        
        /* Cross-iteration dependency between the two accumulators */
        arr[i] = (acc1 + acc2) * (arr[i-1] % 17);
        
        /* Another dependency with modulo (potentially high latency) */
        brr[i] = (brr[i-1] * arr[i]) % 31;
    }
    
    return acc1 + acc2;
}

/* Mixed-type loop with complex dependency pattern */
float mixed_dependencies(float *fa, double *db, int *ic, int n) {
    float fsum = fa[0];
    double dprod = db[0];
    
    for (int i = 1; i < n; i++) {
        /* Type conversion adds complexity */
        fsum = fsum + (float)dprod * fa[i];
        
        /* High-latency double division */
        dprod = dprod / 1.23456789 * db[i];
        
        /* Integer dependency affects floating point */
        ic[i] = ic[i-1] * 2 + (int)(fabs(fsum) * 100);
        
        /* Cross-type dependency */
        fa[i] = (float)(dprod * 0.5) + sqrtf(fsum);
        
        /* Another carried dependency */
        db[i] = db[i-1] * 0.987654321 + sin(dprod);
    }
    
    return fsum + (float)dprod;
}

int main(void) {
    /* Declare and initialize arrays */
    double a[N], b[N], c[N];
    int arr[M], brr[M];
    float fa[M];
    double db[M];
    int ic[M];
    
    /* Initialize with non-zero values to avoid trivial optimizations */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1;
        b[i] = i * 0.2 + 1.0;
        c[i] = i * 0.3 + 2.0;
        if (i < M) {
            arr[i] = i * 3;
            brr[i] = i * 5 + 1;
            fa[i] = i * 0.7f;
            db[i] = i * 1.3;
            ic[i] = i * 2;
        }
    }
    
    /* Execute loops with different dependency patterns */
    double result1 = complex_recurrence(a, b, c, N);
    int result2 = integer_recurrence(arr, brr, M);
    float result3 = mixed_dependencies(fa, db, ic, M);
    
    /* Use results to prevent dead code elimination */
    sink = (int)result1 + result2 + (int)result3;
    
    /* Print results to ensure side effects */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    printf("Sink: %d\n", sink);
    
    return 0;
}
