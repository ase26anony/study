/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debug output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimization */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
double loop1(double *a, double *b, double c) {
    double x = 1.0;
    double y = 2.0;
    
    /* Loop with distance-1 dependency chain */
    for (int i = 1; i < N; i++) {
        /* Carried dependency: x depends on previous iteration's x */
        x = x / 3.14159 * b[i] + a[i-1];
        
        /* Another carried dependency with different latency */
        y = y * 1.5 + x / 2.71828;
        
        /* Cross-dependency between x and y */
        a[i] = x * y + c;
    }
    
    return x + y;
}

/* Loop with integer carried dependencies and high-latency operations */
int loop2(int *arr1, int *arr2) {
    int sum = arr1[0];
    int prod = 1;
    
    /* Multiple interdependent carried dependencies */
    for (int i = 1; i < M; i++) {
        /* Distance-1 dependency on sum */
        sum = sum + arr1[i] * arr2[i];
        
        /* High-latency operation with carried dependency */
        prod = prod * (sum % 7 + 1);
        
        /* Another carried dependency chain */
        arr1[i] = (arr1[i-1] + prod) >> 1;
    }
    
    return sum + prod;
}

/* Loop with floating-point resource contention */
float loop3(float *farr, float init) {
    float a = init;
    float b = init * 0.5f;
    float c = init * 0.25f;
    
    /* Multiple FP operations competing for functional units */
    for (int i = 0; i < N; i++) {
        /* High-latency FP operations with carried dependencies */
        a = a / 1.7f + farr[i] * 2.3f;
        b = b * 1.3f - a / 4.1f;
        c = c + sqrtf(fabsf(b)) * 0.9f;
        
        /* Cross-iteration dependency */
        farr[i] = a * b + c;
        
        /* Additional dependency to increase II */
        if (i > 0) {
            farr[i] += farr[i-1] * 0.1f;
        }
    }
    
    return a + b + c;
}

/* Main function with observable side effects */
int main() {
    /* Initialize arrays with volatile elements to prevent optimization */
    double a[N], b[N];
    int arr1[M], arr2[M];
    float farr[N];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = (double)(i % 37) * 0.1;
        b[i] = (double)(i % 23) * 0.2;
        farr[i] = (float)(i % 19) * 0.3f;
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
    }
    
    /* Execute loops with carried dependencies */
    double result1 = loop1(a, b, 2.5);
    int result2 = loop2(arr1, arr2);
    float result3 = loop3(farr, 10.0f);
    
    /* Force compiler to keep computations */
    sink = result2;
    
    /* Print results to prevent dead code elimination */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    
    return 0;
}
