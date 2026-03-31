/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimization of critical variables */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
double loop1(double *restrict a, double *restrict b, double *restrict c, int n) {
    double sum = 1.0;
    double prod = 2.0;
    
    /* Loop with distance-1 dependency on sum and prod */
    for (int i = 1; i < n; i++) {
        /* Carried dependency: sum[i] depends on sum[i-1] */
        sum = sum * 3.14159 + a[i];
        
        /* Another carried dependency with different distance */
        prod = prod / 2.71828 * b[i];
        
        /* Cross dependency between sum and prod */
        c[i] = (sum + prod) / (a[i-1] + 0.5);
    }
    
    return sum + prod;
}

/* Loop with integer carried dependencies and high-latency operations */
int loop2(int *restrict arr1, int *restrict arr2, int *restrict arr3, int n) {
    int acc1 = arr1[0];
    int acc2 = arr2[0];
    
    /* Multiple interdependent carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency chain */
        acc1 = (acc1 * arr1[i]) / (arr2[i-1] + 1);
        
        /* Another distance-1 dependency with multiplication */
        acc2 = (acc2 + arr3[i]) * (acc1 % 7 + 2);
        
        /* Store with dependency on both accumulators */
        arr3[i] = (acc1 + acc2) / (arr1[i-1] % 5 + 1);
    }
    
    return acc1 + acc2;
}

/* Loop with floating-point operations that stress functional units */
float loop3(float *restrict fa, float *restrict fb, float *restrict fc, int n) {
    float x = fa[0];
    float y = fb[0];
    
    /* Mix of high-latency FP operations with carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Division is high-latency, creates resource contention */
        x = x / 1.2345f + fa[i] * y;
        
        /* More FP operations with carried dependency */
        y = y * 0.9876f - fb[i] / x;
        
        /* Store with complex dependency */
        fc[i] = (x * y) / (fa[i-1] + fb[i-1]);
    }
    
    return x + y;
}

/* Main function to ensure loops aren't optimized away */
int main(void) {
    /* Declare and initialize arrays */
    double a[N], b[N], c[N];
    int arr1[M], arr2[M], arr3[M];
    float fa[N], fb[N], fc[N];
    
    /* Initialize with non-trivial values */
    for (int i = 0; i < N; i++) {
        a[i] = sin(i * 0.1);
        b[i] = cos(i * 0.2);
        fa[i] = i * 0.3f;
        fb[i] = i * 0.4f;
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
        arr3[i] = i * 7;
    }
    
    /* Execute loops with carried dependencies */
    double result1 = loop1(a, b, c, N);
    int result2 = loop2(arr1, arr2, arr3, M);
    float result3 = loop3(fa, fb, fc, N);
    
    /* Use volatile sink to prevent dead code elimination */
    sink = (int)result1 + result2 + (int)result3;
    
    /* Print results to create observable side effect */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    printf("Sink: %d\n", sink);
    
    return 0;
}
