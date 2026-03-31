/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be used and prevent optimization */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
double loop1(double *a, double *b, double *c, int n) {
    double sum = 0.0;
    double prod = 1.0;
    
    /* Loop with distance-1 dependency on 'sum' and 'prod' */
    for (int i = 1; i < n; i++) {
        /* Carried dependency: sum[i] depends on sum[i-1] */
        sum = sum + a[i] * b[i];
        
        /* Another carried dependency: prod[i] depends on prod[i-1] */
        prod = prod * (sum + 1.0);
        
        /* Cross-dependency between sum and prod */
        c[i] = c[i-1] + sum / (prod + 0.001);
    }
    
    return sum + prod;
}

/* Loop with floating-point operations that compete for FP units */
float loop2(float *x, float *y, float *z, int n) {
    float acc = x[0];
    
    /* Multiple high-latency FP operations with carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Division and multiplication compete for FP units */
        float t1 = acc / 3.14159f;
        float t2 = t1 * y[i];
        float t3 = t2 + sqrtf(fabsf(z[i]));
        
        /* Carried dependency chain */
        acc = t3 * 0.99f + x[i] * 0.01f;
        
        /* Additional operation to increase resource pressure */
        y[i] = y[i-1] * 1.1f + acc;
    }
    
    return acc;
}

/* Integer loop with mixed operations and dependencies */
int loop3(int *arr1, int *arr2, int n) {
    int result = arr1[0];
    
    /* Integer operations with carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Multiplication (potentially high latency) */
        int prod = result * arr2[i];
        
        /* Modulo operation */
        int mod = prod % 17;
        
        /* Carried dependency */
        result = arr1[i] + mod + (result >> 2);
        
        /* Anti-dependency */
        arr2[i] = arr2[i-1] ^ result;
    }
    
    return result;
}

/* Nested dependency loop */
double loop4(double *data, int n) {
    double a = data[0];
    double b = data[1];
    double c = data[2];
    
    for (int i = 3; i < n; i++) {
        /* Complex dependency chain */
        double new_a = b * c + sin(data[i]);
        double new_b = a / 2.71828 + cos(data[i-1]);
        double new_c = new_a * new_b - tan(data[i-2]);
        
        /* Update with carried dependencies */
        a = new_a + 0.1;
        b = new_b * 0.9;
        c = new_c / 1.1;
        
        data[i] = a + b + c;
    }
    
    return a + b + c;
}

int main(void) {
    /* Declare and initialize arrays */
    double a[N], b[N], c[N];
    float x[M], y[M], z[M];
    int arr1[N], arr2[N];
    double data[N];
    
    /* Initialize with non-zero values to avoid trivial optimizations */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1 + 1.0;
        b[i] = i * 0.2 + 2.0;
        c[i] = i * 0.3 + 3.0;
        arr1[i] = i * 3;
        arr2[i] = i * 5 + 1;
        data[i] = sin(i * 0.1);
    }
    
    for (int i = 0; i < M; i++) {
        x[i] = i * 0.05f;
        y[i] = i * 0.1f + 0.5f;
        z[i] = i * 0.15f - 0.3f;
    }
    
    /* Execute loops with different dependency patterns */
    double res1 = loop1(a, b, c, N);
    float res2 = loop2(x, y, z, M);
    int res3 = loop3(arr1, arr2, N/2);
    double res4 = loop4(data, N);
    
    /* Use results to prevent dead code elimination */
    sink = (int)res1 + (int)res2 + res3 + (int)res4;
    
    /* Print results to create observable side effect */
    printf("Results: %f, %f, %d, %f\n", res1, (double)res2, res3, res4);
    printf("Sink: %d\n", sink);
    
    return 0;
}
