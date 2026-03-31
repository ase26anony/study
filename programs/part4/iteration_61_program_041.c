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
        
        /* Another distance-1 dependency with floating-point division */
        x = x / 3.1415926535 * c[i] + 0.5;
        
        /* Cross-dependency between sum and prod */
        sum = sum + a[i] * b[i];
        prod = prod * (sum + 1.0) / 2.71828;
        
        /* Artificial resource pressure: multiple FP operations */
        c[i] = (c[i-1] + sin(x)) * cos(prod) / tan(sum + 0.001);
    }
    
    return sum + prod + x;
}

/* Integer loop with carried dependencies */
int integer_recurrence(int *arr1, int *arr2, int n) {
    int acc1 = arr1[0];
    int acc2 = arr2[0];
    
    /* Loop with multiple interleaved carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependencies */
        acc1 = acc1 * 3 + arr1[i];      /* acc1[i] depends on acc1[i-1] */
        acc2 = acc2 / 2 + arr2[i];      /* acc2[i] depends on acc2[i-1] */
        
        /* Cross-iteration dependency between the two accumulators */
        arr1[i] = (acc1 + acc2) % 10007;
        arr2[i] = (acc1 * acc2) % 10007;
        
        /* Additional computation to increase II */
        arr1[i] = (arr1[i] * arr1[i-1]) / (arr2[i] + 1);
    }
    
    return acc1 + acc2;
}

/* Mixed-type loop with complex dependency chain */
float mixed_dependencies(float *farr, double *darr, int *iarr, int n) {
    float f_acc = farr[0];
    double d_acc = darr[0];
    
    for (int i = 1; i < n; i++) {
        /* Type conversions add latency */
        f_acc = f_acc * 1.1f + (float)d_acc;
        d_acc = d_acc / 1.01 + (double)f_acc;
        
        /* Integer dependency chain */
        iarr[i] = iarr[i-1] * 2 + (int)(f_acc * 10);
        
        /* Complex expression with multiple operations */
        farr[i] = sinf(f_acc) * cosf((float)d_acc) + 
                  tanf((float)iarr[i] * 0.01f);
        darr[i] = sqrt(fabs(d_acc)) * log(fabs(f_acc) + 1.0);
    }
    
    return f_acc + (float)d_acc;
}

int main(void) {
    /* Declare and initialize arrays */
    double a[N], b[N], c[N];
    int arr1[M], arr2[M];
    float farr[N];
    double darr[N];
    int iarr[N];
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.5;
        b[i] = sin(i * 0.1);
        c[i] = cos(i * 0.05);
        farr[i] = i * 0.25f;
        darr[i] = i * 0.125;
        iarr[i] = i * 3;
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3 + 1;
    }
    
    /* Execute loops with carried dependencies */
    double result1 = complex_recurrence(a, b, c, N);
    int result2 = integer_recurrence(arr1, arr2, M);
    float result3 = mixed_dependencies(farr, darr, iarr, N);
    
    /* Compute final result to prevent dead code elimination */
    double final_result = result1 + result2 + result3;
    
    /* Use volatile sink to ensure computation isn't optimized away */
    sink = (int)final_result;
    
    /* Print result to create observable side effect */
    printf("Result: %f\n", final_result);
    
    /* Additional computation to increase optimization opportunities */
    double check = 0.0;
    for (int i = 0; i < 10; i++) {
        check += a[i] * b[i] * c[i];
    }
    printf("Check: %f\n", check);
    
    return 0;
}
