/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Force variables to be kept and prevent optimizations */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
double complex_loop_with_dependencies(double *a, double *b, double *c, int n) {
    double sum = 1.0;
    double prod = 2.0;
    double x = 3.14159;
    
    /* Loop 1: Multiple carried dependencies with high-latency operations */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] * 1.01 + b[i];
        
        /* Another distance-1 dependency with floating-point division */
        x = x / 3.14 * c[i] + 0.5;
        
        /* Interdependent carried dependencies */
        sum = sum + a[i] * x;
        prod = prod * (sum + 1.0) / 2.71828;
        
        /* Artificial resource contention: multiple FP operations */
        b[i] = (b[i-1] * 1.5) / (x + 0.001);
        c[i] = c[i-1] * prod + sum / 2.0;
    }
    
    return sum + prod + x;
}

/* Integer loop with carried dependencies and mixed operations */
int integer_loop_with_recurrence(int *arr1, int *arr2, int n) {
    int acc1 = arr1[0];
    int acc2 = arr2[0];
    int temp = 0;
    
    /* Loop with multiple interleaved distance-1 dependencies */
    for (int i = 1; i < n; i++) {
        /* Primary carried dependency chain */
        acc1 = acc1 * 3 + arr1[i];
        
        /* Secondary carried dependency with different distance */
        acc2 = (acc2 + arr2[i]) * 2;
        
        /* Cross-dependency between the two chains */
        temp = acc1 * acc2 / (i + 1);
        
        /* Store results creating more dependencies */
        arr1[i] = temp + acc1;
        arr2[i] = temp - acc2;
        
        /* Additional integer multiplication for resource pressure */
        acc1 = acc1 * 7 / 5;
        acc2 = acc2 * 11 / 8;
    }
    
    return acc1 + acc2 + temp;
}

/* Loop with array recurrence and pointer chasing */
float pointer_chasing_loop(float *data, int *indices, int n) {
    float result = data[0];
    int idx = 0;
    
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependency through pointer chasing */
        idx = indices[idx];
        result = result * 0.99f + data[idx];
        
        /* Additional FP operations for latency */
        data[idx] = data[idx] / 1.1f * result;
        
        /* Conditional to prevent simple analysis */
        if (idx % 3 == 0) {
            result = result + 0.5f;
        }
    }
    
    return result;
}

int main(void) {
    /* Allocate and initialize arrays */
    double *a = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *c = (double*)malloc(N * sizeof(double));
    int *arr1 = (int*)malloc(M * sizeof(int));
    int *arr2 = (int*)malloc(M * sizeof(int));
    float *data = (float*)malloc(N * sizeof(float));
    int *indices = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1;
        b[i] = i * 0.2;
        c[i] = i * 0.3;
        data[i] = i * 0.05f;
        indices[i] = (i * 13) % N;
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3;
    }
    
    /* Execute loops with different dependency patterns */
    double result1 = complex_loop_with_dependencies(a, b, c, N);
    int result2 = integer_loop_with_recurrence(arr1, arr2, M);
    float result3 = pointer_chasing_loop(data, indices, N/2);
    
    /* Use results to prevent dead code elimination */
    sink = (int)result1 + result2 + (int)result3;
    
    /* Print results to ensure side effects */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    printf("Sink: %d\n", sink);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr1);
    free(arr2);
    free(data);
    free(indices);
    
    return 0;
}
