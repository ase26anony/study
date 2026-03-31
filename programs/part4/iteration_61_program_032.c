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
double complex_recurrence(double *a, double *b, double *c, int n) {
    double sum = 1.0;
    double prod = 2.0;
    double x = 3.14159;
    
    /* Loop 1: Multiple carried dependencies with floating-point operations
     * Creates distance-1 dependencies with varying latencies */
    for (int i = 1; i < n; i++) {
        /* Carried dependency: a[i] depends on a[i-1] (distance 1) */
        a[i] = a[i-1] * 2.71828 + b[i];
        
        /* Another carried dependency with floating-point division (high latency) */
        x = x / 1.234567 + c[i] * 0.987654;
        
        /* Interdependent carried dependencies */
        sum = sum + a[i] * x;
        prod = prod * (sum + 1.0);
        
        /* Mix in some expensive operations to create resource pressure */
        b[i] = sqrt(fabs(prod)) * sin(x);
    }
    
    return sum + prod + x;
}

/* Integer loop with carried dependencies and multi-cycle operations */
int integer_recurrence(int *arr1, int *arr2, int n) {
    int acc1 = arr1[0];
    int acc2 = arr2[0];
    int temp = 1;
    
    /* Loop 2: Integer carried dependencies with multiplication
     * Integer multiplication often has higher latency than addition */
    for (int i = 1; i < n; i++) {
        /* Distance-1 carried dependency */
        acc1 = acc1 * 3 + arr1[i];
        
        /* Another distance-1 dependency chain */
        acc2 = (acc2 + arr2[i]) * 5;
        
        /* Cross-dependency between the two chains */
        temp = temp * (acc1 + acc2);
        
        /* Create artificial serialization */
        arr1[i] = (arr1[i] * arr2[i]) / (temp + 1);
    }
    
    return acc1 + acc2 + temp;
}

/* Loop with array-based carried dependencies */
void array_carried_dep(double *in, double *out, int n) {
    /* Initialize first element */
    out[0] = in[0] * 2.0;
    
    /* Loop 3: Clear distance-1 array dependency
     * out[i] depends on out[i-1] */
    for (int i = 1; i < n; i++) {
        /* Primary carried dependency */
        out[i] = out[i-1] * 1.5 + in[i];
        
        /* Additional computation with potential resource conflicts */
        in[i] = cos(out[i]) * sin(in[i-1]);
        
        /* Mix in division for latency */
        out[i] = out[i] / (1.0 + fabs(in[i]));
    }
}

int main(void) {
    /* Declare and initialize arrays with volatile elements
     * to prevent dead code elimination */
    volatile double a[N], b[N], c[N];
    volatile int arr1[M], arr2[M];
    volatile double in[N], out[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1;
        b[i] = i * 0.2;
        c[i] = i * 0.3;
        in[i] = sin(i * 0.1);
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3;
    }
    
    /* Execute loops with carried dependencies */
    double result1 = complex_recurrence((double*)a, (double*)b, (double*)c, N);
    int result2 = integer_recurrence((int*)arr1, (int*)arr2, M);
    array_carried_dep((double*)in, (double*)out, N);
    
    /* Compute final result to ensure all loops contribute */
    double final_result = result1 + result2 + out[N-1];
    
    /* Use the results to prevent optimization */
    sink = (int)final_result;
    
    /* Print result to create observable side effect */
    printf("Result: %f\n", final_result);
    
    return 0;
}
