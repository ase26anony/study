/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be preserved through optimization */
static volatile int sink __attribute__((used));

/* Complex loop with multiple carried dependencies */
double loop_with_carried_deps(double *arr, double *brr, int n) {
    double sum = 1.0;
    double prod = 2.0;
    double x = 3.14159;
    
    /* Loop 1: Multiple carried dependencies with floating-point operations
     * Creates distance-1 dependencies: sum[i] depends on sum[i-1]
     * and prod[i] depends on prod[i-1] and sum[i]
     */
    for (int i = 1; i < n; i++) {
        /* Carried dependency on sum with high-latency division */
        sum = sum / 2.71828 + arr[i] * brr[i-1];
        
        /* Another carried dependency on prod with multiplication */
        prod = prod * (sum + 1.0) * 0.99;
        
        /* Third carried dependency chain */
        x = x * 1.01 + sin(arr[i] * 0.1);
    }
    
    return sum + prod + x;
}

/* Integer loop with array-based carried dependencies */
int integer_carried_deps(int *a, int *b, int n) {
    int result = 0;
    
    /* Loop 2: Integer carried dependencies with array accesses
     * a[i] depends on a[i-1] and b[i-1] - creates distance-1 deps
     */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: a[i] uses a[i-1] */
        a[i] = a[i-1] * 3 + b[i];
        
        /* Another distance-1 dependency chain */
        b[i] = (b[i-1] + a[i]) / 2;
        
        /* Accumulator with carried dependency */
        result += a[i] * b[i-1];
    }
    
    return result;
}

/* Mixed operations to create resource contention */
float resource_contention_loop(float *farr, int *iarr, int n) {
    float f1 = 1.5f;
    float f2 = 2.5f;
    float f3 = 3.5f;
    
    /* Loop 3: Mix float and int operations to stress functional units */
    for (int i = 0; i < n; i++) {
        /* High-latency float operations */
        f1 = f1 * 1.1f + farr[i] / 3.14f;
        f2 = f2 / 1.05f + sinf(f1) * 2.0f;
        f3 = f3 * 0.95f + cosf(f2) * farr[i];
        
        /* Integer operations mixed in */
        iarr[i] = (int)(f1 * 100) + iarr[i % M];
    }
    
    return f1 + f2 + f3;
}

int main(void) {
    /* Declare and initialize arrays with volatile elements
     * to prevent dead code elimination */
    double arr[N] __attribute__((used));
    double brr[N] __attribute__((used));
    int iarr1[N] __attribute__((used));
    int iarr2[N] __attribute__((used));
    float farr[M] __attribute__((used));
    int iarr3[M] __attribute__((used));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = (double)(i % 100) * 0.1;
        brr[i] = (double)((i + 1) % 100) * 0.2;
        iarr1[i] = i * 2;
        iarr2[i] = i * 3;
    }
    
    for (int i = 0; i < M; i++) {
        farr[i] = (float)(i % 50) * 0.3f;
        iarr3[i] = i * 4;
    }
    
    /* Execute loops with carried dependencies */
    double result1 = loop_with_carried_deps(arr, brr, N);
    int result2 = integer_carried_deps(iarr1, iarr2, N);
    float result3 = resource_contention_loop(farr, iarr3, M);
    
    /* Compute final result to ensure loops aren't optimized away */
    double final_result = result1 + result2 + result3;
    
    /* Use volatile sink to prevent optimization */
    sink = (int)final_result;
    
    /* Print result to create observable side effect */
    printf("Final result: %f\n", final_result);
    
    return 0;
}
