/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimizations from removing our loops */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
double complex_recurrence(double *a, double *b, double c, int n) {
    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    
    /* Loop 1: Multiple carried dependencies with high-latency operations */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: x[i] depends on x[i-1] */
        x = x / 3.14159 * b[i] + a[i-1];
        
        /* Another distance-1 dependency chain */
        y = y * 1.61803 - a[i] / 2.71828;
        
        /* Cross-dependency between chains */
        z = (x + y) / (z + 0.0001);
        
        /* Resource contention: multiple divides in same iteration */
        a[i] = (x * y) / (z + 1.0) + sin(b[i] * 0.01);
    }
    
    return x + y + z;
}

/* Integer loop with carried dependencies and multiplications */
int integer_recurrence(int *arr1, int *arr2, int n) {
    int sum = arr1[0];
    int prod = 1;
    int acc = 0;
    
    /* Loop with multiple interdependent carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency on sum */
        sum = sum + arr1[i] * arr2[i-1];
        
        /* Distance-1 dependency on prod with high-latency multiply */
        prod = prod * (sum % 100 + 1);
        
        /* Another dependency chain */
        acc = acc ^ (prod >> (i % 8));
        
        /* Cross-iteration dependency through array */
        arr2[i] = (arr1[i] + arr2[i-1]) * acc;
    }
    
    return sum + prod + acc;
}

/* Mixed-type loop to stress the scheduler */
float mixed_operations(float *farr, double *darr, int n) {
    float fsum = farr[0];
    double dsum = darr[0];
    
    for (int i = 1; i < n; i++) {
        /* Float operations with carried dependency */
        fsum = fsum * 1.1f + farr[i] / 2.5f;
        
        /* Double operations with carried dependency */
        dsum = dsum / 1.7 + darr[i-1] * 0.9;
        
        /* Interaction between float and double chains */
        farr[i] = (float)dsum * 0.5f + fsum;
        darr[i] = (double)fsum * 1.5 + dsum;
    }
    
    return fsum + (float)dsum;
}

int main(void) {
    /* Initialize arrays with volatile elements to prevent optimization */
    double a[N], b[N];
    int arr1[M], arr2[M];
    float farr[N];
    double darr[N];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1;
        b[i] = sin(i * 0.05);
        farr[i] = i * 0.2f;
        darr[i] = cos(i * 0.03);
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
    }
    
    /* Execute loops with different dependency patterns */
    double result1 = complex_recurrence(a, b, 2.5, N);
    int result2 = integer_recurrence(arr1, arr2, M);
    float result3 = mixed_operations(farr, darr, N);
    
    /* Force side effects to prevent dead code elimination */
    sink = result2;
    
    /* Print results to ensure loops execute */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    
    return 0;
}
