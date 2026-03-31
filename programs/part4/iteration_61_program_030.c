/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be kept and prevent optimizations */
static volatile int g_sink = 0;
static volatile double g_double_sink = 0.0;

/* Complex loop with multiple carried dependencies */
void loop_with_carried_deps(int *result1, double *result2) {
    int i;
    int acc_int = 1;
    double acc_double = 1.0;
    
    /* Loop 1: Integer carried dependency with multiplication bottleneck */
    /* Creates distance-1 dependencies: acc_int[i] depends on acc_int[i-1] */
    for (i = 1; i < N; i++) {
        /* High-latency integer multiplication creates resource pressure */
        acc_int = acc_int * 3 + i;
        
        /* Additional operation to create more complex dependency graph */
        acc_int = (acc_int >> 2) | (acc_int << 30); /* rotate */
        
        /* Use volatile to prevent dead code elimination */
        g_sink = acc_int;
    }
    
    *result1 = acc_int;
    
    /* Loop 2: Floating-point with complex carried dependencies */
    /* Multiple recurrence relations to increase II */
    double fp_acc1 = 1.0;
    double fp_acc2 = 2.0;
    double array[N];
    
    /* Initialize array with some values */
    for (i = 0; i < N; i++) {
        array[i] = sin(i * 0.01);
    }
    
    for (i = 1; i < M; i++) {
        /* High-latency floating-point division and multiplication */
        /* Creates distance-1 dependency: fp_acc1[i] depends on fp_acc1[i-1] */
        fp_acc1 = fp_acc1 / 3.14159 * array[i];
        
        /* Another carried dependency chain */
        fp_acc2 = fp_acc2 * 1.5 + fp_acc1;
        
        /* Cross-dependency between the two chains */
        fp_acc1 = fp_acc1 + fp_acc2 * 0.1;
        
        /* Use result in array to create memory dependencies */
        array[i] = fp_acc1 + fp_acc2;
        
        g_double_sink = fp_acc1 + fp_acc2;
    }
    
    *result2 = fp_acc1 + fp_acc2;
}

/* Loop with nested dependencies and array accesses */
void loop_with_nested_deps(double *output) {
    int i, j;
    double matrix[8][8];
    double vector[8];
    double result[8] = {0};
    
    /* Initialize */
    for (i = 0; i < 8; i++) {
        vector[i] = i * 0.5;
        for (j = 0; j < 8; j++) {
            matrix[i][j] = (i + j) * 0.1;
        }
    }
    
    /* Matrix-vector multiplication with carried dependencies */
    /* Each iteration depends on previous due to accumulation pattern */
    for (i = 0; i < 8; i++) {
        double sum = 0.0;
        for (j = 0; j < 8; j++) {
            /* High-latency operations */
            sum += matrix[i][j] * vector[j] / (j + 1.0);
        }
        
        /* Carried dependency through vector update */
        vector[i] = vector[i] + sum * 0.01;
        
        /* Another carried dependency chain */
        if (i > 0) {
            result[i] = result[i-1] + sum;
        } else {
            result[i] = sum;
        }
        
        g_double_sink = sum;
    }
    
    *output = result[7];
}

/* Main function with observable side effects */
int main() {
    int int_result;
    double double_result1, double_result2;
    
    printf("Starting modulo scheduler test loops...\n");
    
    /* Execute loops with different dependency patterns */
    loop_with_carried_deps(&int_result, &double_result1);
    loop_with_nested_deps(&double_result2);
    
    /* Compute final result with all outputs to prevent dead code elimination */
    double final_result = int_result * 0.001 + double_result1 + double_result2;
    
    /* Print results to create observable side effect */
    printf("Results: int=%d, double1=%.6f, double2=%.6f\n", 
           int_result, double_result1, double_result2);
    printf("Final combined result: %.6f\n", final_result);
    
    /* Additional volatile store to ensure loops aren't optimized away */
    g_sink = int_result;
    g_double_sink = final_result;
    
    return (final_result > 0) ? 0 : 1;
}
