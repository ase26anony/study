/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debug output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be used and prevent optimization */
static volatile int sink;
static volatile double dsink;

/* Complex loop with multiple carried dependencies */
void loop_with_carried_deps(int *arr1, int *arr2, double *darr) {
    int i;
    int acc_int = arr1[0];  /* Initial value for integer recurrence */
    double acc_fp = darr[0]; /* Initial value for FP recurrence */
    
    /* Loop 1: Integer carried dependency with multiplication bottleneck */
    for (i = 1; i < N; i++) {
        /* Distance-1 dependency: uses value from previous iteration */
        acc_int = acc_int * 3 + arr1[i];
        
        /* Another carried dependency in same iteration */
        arr2[i] = arr2[i-1] + acc_int;
        
        /* High-latency FP operation competing for resources */
        acc_fp = acc_fp / 3.14159 * (1.0 + darr[i]);
    }
    
    /* Loop 2: Nested dependencies and FP operations */
    double sum = 0.0;
    double prod = 1.0;
    for (i = 0; i < M; i++) {
        /* Multiple interdependent carried dependencies */
        sum = sum + darr[i] * (i + 1);
        prod = prod * (sum + 1.0);
        
        /* More FP operations to create resource pressure */
        darr[i] = darr[i] / 2.71828 + prod;
    }
    
    /* Force results to be used */
    sink = acc_int + (int)sum;
    dsink = acc_fp + prod;
}

/* Loop with array-based carried dependencies */
void loop_with_array_deps(double *a, double *b, double *c) {
    int i;
    
    /* Strong distance-1 dependency chain through array */
    for (i = 1; i < N; i++) {
        /* Classic recurrence: a[i] depends on a[i-1] */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Additional FP operation to increase II */
        b[i] = b[i] / 1.41421 + a[i];
        
        /* Another carried dependency through c */
        c[i] = c[i-1] + sin(a[i] * 0.01);
    }
    
    /* Compute checksum to prevent elimination */
    double total = 0.0;
    for (i = 0; i < N; i++) {
        total += a[i] + b[i] + c[i];
    }
    dsink = total;
}

/* Mixed integer/FP loop with complex dependency pattern */
void mixed_dependency_loop(int *iv, double *dv) {
    int i;
    int int_acc = iv[0];
    double fp_acc = dv[0];
    
    for (i = 1; i < N; i++) {
        /* Integer carried dependency */
        int_acc = (int_acc * 7) ^ iv[i];
        
        /* FP carried dependency with high-latency operations */
        fp_acc = fp_acc / 2.5 + dv[i] * log(fabs(fp_acc) + 1.0);
        
        /* Cross-type dependency: FP depends on integer */
        dv[i] = dv[i] + sin(int_acc * 0.001);
        
        /* Integer depends on FP */
        iv[i] = iv[i] + (int)(fp_acc * 100);
    }
    
    sink = int_acc;
    dsink = fp_acc;
}

int main() {
    /* Declare and initialize arrays */
    int arr1[N], arr2[N];
    double darr[N], a[N], b[N], c[N];
    int iv[N];
    double dv[N];
    
    int i;
    for (i = 0; i < N; i++) {
        arr1[i] = i * 2 + 1;
        arr2[i] = i * 3;
        darr[i] = i * 0.5;
        a[i] = i * 0.1;
        b[i] = i * 0.2;
        c[i] = i * 0.3;
        iv[i] = i;
        dv[i] = i * 0.25;
    }
    
    /* Execute all loops to create various dependency patterns */
    loop_with_carried_deps(arr1, arr2, darr);
    loop_with_array_deps(a, b, c);
    mixed_dependency_loop(iv, dv);
    
    /* Compute and print final result to ensure loops aren't eliminated */
    double final_result = dsink + sink;
    printf("Result: %f\n", final_result);
    
    return 0;
}
