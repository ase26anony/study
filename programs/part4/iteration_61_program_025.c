/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be preserved through optimization */
static volatile int sink;

/* Complex loop with multiple carried dependencies and resource contention */
void complex_loop_with_dependencies(double *restrict a, 
                                   double *restrict b, 
                                   double *restrict c,
                                   int n) {
    /* Multiple carried dependencies with different distances */
    double acc1 = 1.0;
    double acc2 = 2.0;
    double acc3 = 3.0;
    
    /* Loop with mixed operations competing for FP units */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: acc1[i] depends on acc1[i-1] */
        acc1 = acc1 / 3.14159 * b[i] + a[i-1];
        
        /* Distance-1 dependency with different latency operations */
        acc2 = acc2 * 1.61803 + c[i] / 2.71828;
        
        /* Nested dependency chain */
        double temp = acc1 * acc2;
        acc3 = acc3 / (temp + 1.0) * sin((double)i * 0.01);
        
        /* Store results to prevent elimination */
        a[i] = acc1 + acc2 + acc3;
    }
    
    /* Use results to prevent dead code elimination */
    sink = (int)(acc1 + acc2 + acc3);
}

/* Integer loop with carried dependencies and multiplication pressure */
void integer_loop_with_resources(int *restrict arr1, 
                                int *restrict arr2, 
                                int *restrict arr3,
                                int n) {
    int sum = arr1[0];
    int prod = arr2[0];
    
    /* Loop with integer multiplications and carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency on sum */
        sum = sum + arr1[i] * arr2[i-1];
        
        /* Distance-1 dependency on prod with high-latency operation */
        prod = prod * (sum % 7 + 1);
        
        /* Another carried dependency chain */
        arr3[i] = arr3[i-1] + prod * arr1[i];
    }
    
    sink = sum + prod;
}

/* Loop with artificial resource bottlenecks using division */
void fp_heavy_loop(double *restrict x, 
                   double *restrict y, 
                   double *restrict z,
                   int n) {
    /* Initialize with volatile to prevent constant propagation */
    volatile double init = 2.0;
    double a = init;
    double b = init * 0.5;
    
    /* Loop designed to create high II due to FP division bottlenecks */
    for (int i = 1; i < n; i++) {
        /* Multiple FP divisions competing for limited units */
        a = a / (x[i] + 1.0) * y[i-1];
        b = b / (y[i] + 2.0) + z[i-1];
        
        /* Cross-dependency between a and b */
        double c = a * b;
        x[i] = x[i-1] + c / (a + b);
        
        /* Another division to increase pressure */
        y[i] = y[i-1] / (c + 1.0) * 0.99;
    }
    
    sink = (int)(a + b);
}

/* Main function with observable side effects */
int main(void) {
    /* Declare arrays with enough elements to avoid small loop unrolling */
    double arr1[N], arr2[N], arr3[N];
    int iarr1[M], iarr2[M], iarr3[M];
    
    /* Initialize arrays with non-constant patterns */
    for (int i = 0; i < N; i++) {
        arr1[i] = (double)(i % 37) * 0.1;
        arr2[i] = (double)(i % 23) * 0.2;
        arr3[i] = (double)(i % 17) * 0.3;
    }
    
    for (int i = 0; i < M; i++) {
        iarr1[i] = (i * 3) % 19;
        iarr2[i] = (i * 5) % 17;
        iarr3[i] = (i * 7) % 13;
    }
    
    /* Execute loops with different dependency patterns */
    complex_loop_with_dependencies(arr1, arr2, arr3, N);
    integer_loop_with_resources(iarr1, iarr2, iarr3, M);
    fp_heavy_loop(arr1, arr2, arr3, N);
    
    /* Compute and print final result to ensure loops aren't eliminated */
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += arr1[i] + arr2[i] + arr3[i];
    }
    
    for (int i = 0; i < M; i++) {
        final_result += (double)(iarr1[i] + iarr2[i] + iarr3[i]);
    }
    
    printf("Final result: %f\n", final_result);
    printf("Sink value: %d\n", sink);
    
    return 0;
}
