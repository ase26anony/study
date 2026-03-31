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
double loop_with_carried_deps(double *a, double *b, double *c, int n) {
    double sum = 1.0;
    double prod = 2.0;
    double x = 3.14159;
    
    /* Loop 1: Multiple carried dependencies with floating-point operations
     * Creates distance-1 dependencies: sum[i] depends on sum[i-1]
     * and prod[i] depends on prod[i-1] and sum[i]
     */
    for (int i = 1; i < n; i++) {
        /* Carried dependency on sum with high-latency division */
        sum = sum / 2.71828 + a[i] * b[i];
        
        /* Carried dependency on prod with multiplication */
        prod = prod * (sum + 1.0);
        
        /* Another carried dependency chain on x */
        x = x * 1.01 + sin(c[i]) * cos(c[i-1]);
        
        /* Mix in some integer operations to create resource pressure */
        a[i] = a[i-1] * 0.99 + b[i] / 3.14159;
    }
    
    return sum + prod + x;
}

/* Integer loop with complex recurrence pattern */
int integer_carried_deps(int *arr1, int *arr2, int n) {
    int acc1 = arr1[0];
    int acc2 = arr2[0];
    int acc3 = 1;
    
    /* Loop with multiple interdependent carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: acc1[i] uses acc1[i-1] */
        acc1 = acc1 * 3 + arr1[i];
        
        /* Distance-1 dependency: acc2[i] uses acc2[i-1] and acc1[i] */
        acc2 = (acc2 + acc1) * 7 - arr2[i];
        
        /* Another carried dependency with modulo operation */
        acc3 = (acc3 * 11) % 997 + arr1[i-1] * arr2[i];
        
        /* Cross-iteration array dependency */
        arr1[i] = arr1[i-1] + arr2[i] * 2;
    }
    
    return acc1 + acc2 + acc3;
}

/* Loop with artificial resource bottlenecks using multiple FP operations */
double resource_intensive_loop(double *data, int n) {
    double result = 0.0;
    double temp1 = 1.0;
    double temp2 = 2.0;
    
    /* Create pressure on floating-point units with mixed operations */
    for (int i = 1; i < n; i++) {
        /* High-latency division creating resource contention */
        temp1 = temp1 / 3.141592653589793;
        
        /* Multiplication that depends on previous division result */
        temp2 = temp2 * 1.618033988749895;
        
        /* Complex expression with multiple dependencies */
        result = result + temp1 * temp2 * sin(data[i]) / cos(data[i-1]);
        
        /* Additional FP operations to increase II */
        data[i] = data[i-1] * 0.99 + tan(result) * 0.01;
    }
    
    return result;
}

int main(void) {
    /* Declare and initialize arrays with volatile elements 
     * to prevent dead code elimination */
    double a[N], b[N], c[N];
    int arr1[M], arr2[M];
    double data[N];
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1;
        b[i] = sin(i * 0.05);
        c[i] = cos(i * 0.03);
        data[i] = i * 0.07;
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5 + 1;
    }
    
    /* Execute loops with carried dependencies */
    double result1 = loop_with_carried_deps(a, b, c, N);
    int result2 = integer_carried_deps(arr1, arr2, M);
    double result3 = resource_intensive_loop(data, N);
    
    /* Compute final result to ensure loops aren't optimized away */
    double final_result = result1 + result2 + result3;
    
    /* Use volatile sink to prevent optimization */
    sink = (int)final_result;
    
    /* Print result to create observable side effect */
    printf("Final result: %f\n", final_result);
    
    return 0;
}
