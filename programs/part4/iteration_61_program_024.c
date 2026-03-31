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

/* Complex loop with multiple carried dependencies and resource contention */
double complex_loop_with_dependencies(double *arr1, double *arr2, int n) {
    double sum = 1.0;
    double prod = 2.0;
    double acc = 0.5;
    
    /* Loop 1: Multiple carried dependencies with high-latency operations */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: acc[i] depends on acc[i-1] */
        acc = acc / 3.14159 * arr1[i] + sin(acc * 0.1);
        
        /* Another distance-1 dependency: sum[i] depends on sum[i-1] */
        sum = sum + arr2[i] * (acc + 1.0);
        
        /* Cross-dependency between different variables */
        prod = prod * (sum / (fabs(acc) + 1.0));
        
        /* Artificial resource bottleneck: multiple FP divisions */
        arr1[i] = arr1[i-1] / 2.71828 + arr2[i] / 1.41421;
    }
    
    return sum + prod + acc;
}

/* Integer loop with carried dependencies and multiplication bottleneck */
int integer_loop_with_recurrence(int *data, int n) {
    int result = 1;
    int temp = data[0];
    
    /* Loop with distance-1 dependencies and integer multiplication */
    for (int i = 1; i < n; i++) {
        /* Carried dependency: temp[i] depends on temp[i-1] */
        temp = temp * 37 + data[i];
        
        /* Another carried dependency with different distance */
        result = result ^ (temp * i);
        
        /* Create artificial anti-dependencies */
        data[i] = data[i-1] + result % 17;
    }
    
    return result * temp;
}

/* Mixed-type loop with complex dependency pattern */
float mixed_operations(float *fa, int *ia, double *da, int n) {
    float f_acc = fa[0];
    double d_acc = da[0];
    int i_acc = ia[0];
    
    for (int i = 1; i < n; i++) {
        /* Type conversion creates additional latency */
        f_acc = f_acc * 1.1f + (float)d_acc;
        
        /* Integer operation with carried dependency */
        i_acc = (i_acc * 31 + ia[i]) % 1023;
        
        /* Double precision with division (high latency) */
        d_acc = d_acc / 1.6180339887 + da[i] * 0.70710678118;
        
        /* Cross-type dependency */
        fa[i] = f_acc + (float)(d_acc * i_acc);
        
        /* Another carried dependency chain */
        ia[i] = ia[i-1] + (int)(f_acc * 10.0f);
    }
    
    return f_acc + (float)d_acc + (float)i_acc;
}

int main(void) {
    /* Declare and initialize arrays with volatile elements to prevent optimization */
    double arr1[N], arr2[N];
    int int_data[M];
    float float_arr[M];
    double double_arr[M];
    int int_arr[M];
    
    /* Initialize with non-trivial patterns */
    for (int i = 0; i < N; i++) {
        arr1[i] = sin(i * 0.1) + 1.0;
        arr2[i] = cos(i * 0.05) * 2.0;
    }
    
    for (int i = 0; i < M; i++) {
        int_data[i] = (i * 17) % 23;
        float_arr[i] = i * 0.123f;
        double_arr[i] = i * 0.456;
        int_arr[i] = i * 3;
    }
    
    /* Execute loops with different dependency patterns */
    double result1 = complex_loop_with_dependencies(arr1, arr2, N);
    int result2 = integer_loop_with_recurrence(int_data, M);
    float result3 = mixed_operations(float_arr, int_arr, double_arr, M);
    
    /* Compute final result to ensure loops aren't dead code */
    double final_result = result1 + result2 + result3;
    
    /* Use volatile sink to prevent dead code elimination */
    g_sink = (int)final_result;
    
    /* Print result to create observable side effect */
    printf("Final result: %f\n", final_result);
    
    /* Additional volatile store to ensure loops execute */
    volatile int dummy = 0;
    for (int i = 0; i < 10; i++) {
        dummy += int_data[i % M];
    }
    
    return g_sink & 0xFF;
}
