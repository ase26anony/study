/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Force variables to be preserved through optimization */
static volatile int g_sink __attribute__((used));

/* Complex loop with multiple carried dependencies and resource contention */
void complex_loop_with_dependencies(double *restrict a, 
                                   double *restrict b, 
                                   double *restrict c, 
                                   int n) {
    /* Multiple carried dependencies with distance 1 */
    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    
    /* Loop with three interdependent carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: x_i depends on x_{i-1} */
        x = x / 3.14159 * b[i];  /* High-latency division */
        
        /* Distance-1 dependency: y_i depends on y_{i-1} and x_i */
        y = y * 1.61803 + x;     /* Multiplication with dependency on x */
        
        /* Distance-1 dependency: z_i depends on z_{i-1} and y_i */
        z = z / 2.71828 + y;     /* Another division with dependency on y */
        
        /* Store result with dependency on all three */
        a[i] = x + y + z;
        
        /* Additional computation to create resource pressure */
        c[i] = c[i-1] * 0.99 + a[i];  /* Another distance-1 dependency */
    }
    
    /* Force results to be used */
    g_sink = (int)(x + y + z);
}

/* Integer loop with multiple carried dependencies */
void integer_loop_with_dependencies(int *restrict arr1,
                                   int *restrict arr2,
                                   int *restrict result,
                                   int n) {
    int sum = arr1[0];
    int prod = arr2[0];
    
    /* Loop with two interdependent carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: sum_i depends on sum_{i-1} */
        sum = sum + arr1[i] * arr2[i];  /* Multiplication competes for units */
        
        /* Distance-1 dependency: prod_i depends on prod_{i-1} and sum_i */
        prod = prod * (sum + 1);        /* Multiplication with dependency */
        
        /* Store with both dependencies */
        result[i] = sum + prod;
    }
    
    g_sink = sum + prod;
}

/* Mixed-type loop to stress the scheduler */
void mixed_loop(float *restrict farr,
                double *restrict darr,
                int *restrict iarr,
                int n) {
    float f_acc = farr[0];
    double d_acc = darr[0];
    int i_acc = iarr[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple carried dependencies of different types */
        f_acc = f_acc * 1.1f + farr[i];      /* Float multiplication */
        d_acc = d_acc / 1.234567 + darr[i];  /* Double division - high latency */
        i_acc = i_acc + iarr[i] * (int)f_acc; /* Integer with float dependency */
        
        /* Cross-type dependencies create complex scheduling */
        farr[i] = (float)d_acc + f_acc;
        darr[i] = d_acc * 0.5;
        iarr[i] = i_acc % 100;
    }
    
    g_sink = (int)(f_acc + d_acc + i_acc);
}

int main(void) {
    /* Allocate and initialize arrays with volatile to prevent optimization */
    double *a = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *c = (double*)malloc(N * sizeof(double));
    
    int *arr1 = (int*)malloc(M * sizeof(int));
    int *arr2 = (int*)malloc(M * sizeof(int));
    int *result = (int*)malloc(M * sizeof(int));
    
    float *farr = (float*)malloc(N * sizeof(float));
    double *darr = (double*)malloc(N * sizeof(double));
    int *iarr = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1;
        b[i] = i * 0.2 + 1.0;
        c[i] = i * 0.3 + 2.0;
        farr[i] = i * 0.4f;
        darr[i] = i * 0.5;
        iarr[i] = i;
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3;
        result[i] = 0;
    }
    
    /* Execute loops with different dependency patterns */
    complex_loop_with_dependencies(a, b, c, N);
    integer_loop_with_dependencies(arr1, arr2, result, M);
    mixed_loop(farr, darr, iarr, N);
    
    /* Compute final result to ensure loops aren't dead code */
    double final_sum = 0.0;
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + c[i] + farr[i] + darr[i] + iarr[i];
    }
    for (int i = 0; i < M; i++) {
        final_sum += arr1[i] + arr2[i] + result[i];
    }
    
    printf("Final result: %f\n", final_sum);
    printf("Sink value: %d\n", g_sink);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(arr1); free(arr2); free(result);
    free(farr); free(darr); free(iarr);
    
    return 0;
}
