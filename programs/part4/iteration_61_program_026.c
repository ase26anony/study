/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be preserved through optimization */
static volatile int g_sink __attribute__((used));
static volatile double g_double_sink __attribute__((used));

/* Complex loop with multiple carried dependencies */
void loop_with_carried_deps(int *arr1, int *arr2, int *result) {
    int i;
    int sum = arr1[0];  /* Initial value for recurrence */
    int prod = 1;
    
    /* Loop 1: Integer carried dependency with multiplication bottleneck */
    for (i = 1; i < N; i++) {
        /* Distance-1 dependency: sum[i] depends on sum[i-1] */
        sum = sum * 3 + arr1[i];
        
        /* Another carried dependency chain */
        prod = prod * (sum % 17 + 1);
        
        /* Store result with index dependency */
        arr2[i] = (arr2[i-1] + sum) * prod;
    }
    
    *result = sum + prod;
}

/* Loop with floating-point bottlenecks and complex dependencies */
double loop_with_fp_bottlenecks(double *arr, double *coeffs) {
    int i;
    double x = arr[0];
    double y = 1.0;
    
    /* Loop 2: Floating-point operations with carried dependencies */
    for (i = 1; i < M; i++) {
        /* High-latency FP division with carried dependency */
        x = x / 3.141592653589793;
        
        /* FP multiplication with another dependency chain */
        y = y * (x + coeffs[i]);
        
        /* Cross-dependency between the two chains */
        x = x * y + sin(coeffs[i-1]);
        
        /* Array access with distance-1 dependency */
        arr[i] = arr[i-1] * 0.99 + x;
    }
    
    return x + y;
}

/* Nested loops to increase scheduling complexity */
void nested_loops_with_deps(int *matrix, int size) {
    int i, j;
    int acc = matrix[0];
    
    for (i = 1; i < size; i++) {
        int row_acc = matrix[i * size];
        
        for (j = 1; j < size; j++) {
            /* Multiple carried dependencies within inner loop */
            row_acc = row_acc * 2 + matrix[i * size + j];
            
            /* Cross-iteration dependency with outer loop */
            acc = (acc + row_acc) % 1023;
            
            /* Store with dependency on previous iteration */
            matrix[i * size + j] = matrix[(i-1) * size + j] + row_acc;
        }
        
        /* Outer loop carried dependency */
        matrix[i * size] = acc;
    }
    
    g_sink = acc;
}

int main(void) {
    int arr1[N], arr2[N];
    double fp_arr[M], coeffs[M];
    int matrix[64 * 64];  /* 64x64 matrix */
    int i, int_result;
    double fp_result;
    
    /* Initialize arrays with non-trivial patterns */
    for (i = 0; i < N; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 2;
    }
    
    for (i = 0; i < M; i++) {
        fp_arr[i] = (double)i * 0.1;
        coeffs[i] = sin((double)i * 0.05);
    }
    
    for (i = 0; i < 64 * 64; i++) {
        matrix[i] = i % 97;
    }
    
    /* Execute loops with different dependency patterns */
    loop_with_carried_deps(arr1, arr2, &int_result);
    fp_result = loop_with_fp_bottlenecks(fp_arr, coeffs);
    nested_loops_with_deps(matrix, 64);
    
    /* Use results to prevent dead code elimination */
    g_sink = int_result;
    g_double_sink = fp_result;
    
    /* Print results to ensure side effects */
    printf("Results: int=%d, double=%.6f, matrix[0]=%d\n", 
           int_result, fp_result, matrix[0]);
    
    return 0;
}
