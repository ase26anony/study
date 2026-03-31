/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimization */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
void loop_with_carried_deps(int *result) {
    volatile double x = 3.14159;
    volatile double y = 2.71828;
    double arr[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i * 0.5;
    }
    
    /* Loop 1: Simple carried dependency with integer */
    int sum = 0;
    for (int i = 1; i < N; i++) {
        /* Distance-1 dependency: sum[i] depends on sum[i-1] */
        sum = sum + (int)(arr[i] * 2.0);
        /* Additional operation to create resource pressure */
        arr[i] = arr[i-1] * 1.1 + sin(arr[i]) * 0.01;
    }
    
    /* Loop 2: Complex FP carried dependency chain */
    double acc1 = 1.0, acc2 = 2.0;
    for (int i = 1; i < M; i++) {
        /* Multiple carried dependencies creating complex scheduling */
        double temp = acc1 * 0.99;
        
        /* High-latency FP operations competing for resources */
        acc1 = acc1 / 3.14159 * acc2 + log(fabs(temp) + 1.0);
        acc2 = acc2 * 0.95 + cos(acc1 * 0.1);
        
        /* Another distance-1 dependency */
        arr[i] = arr[i-1] * acc1 + arr[i] * acc2;
        
        /* Integer operation to mix instruction types */
        sum += (int)(acc1 * 100);
    }
    
    /* Loop 3: Nested dependencies with array accesses */
    double prod = 1.0;
    for (int i = 2; i < N-2; i++) {
        /* Multiple interleaved dependencies */
        double a = arr[i-2] * 0.8;
        double b = arr[i-1] * 0.9;
        double c = arr[i] * 1.0;
        
        /* Complex expression with carried dependencies */
        arr[i] = (a + b) / (c + 0.001) * prod;
        prod = prod * (arr[i] + 1.0) * 0.999;
        
        /* Another recurrence */
        sum += (int)(prod * 10);
    }
    
    *result = sum;
    sink = *result; /* Ensure loop isn't optimized away */
}

/* Loop with artificial resource bottlenecks */
void resource_intensive_loop(double *output) {
    volatile double a = 10.0;
    volatile double b = 20.0;
    double buffer[N];
    
    /* Initialize with varying values */
    for (int i = 0; i < N; i++) {
        buffer[i] = i * 0.3;
    }
    
    /* Loop with FP divides and multiplies - high latency ops */
    for (int i = 1; i < N; i++) {
        /* Multiple high-latency operations in dependency chain */
        double div_result = buffer[i-1] / 2.71;
        double mul_result = div_result * buffer[i];
        
        /* More operations to create scheduling complexity */
        buffer[i] = mul_result + sin(div_result) * cos(mul_result);
        
        /* Update carried variable */
        a = a * 0.99 + buffer[i] * 0.01;
        b = b / 1.01 + a * 0.5;
    }
    
    /* Final computation with carried dependency */
    double total = 0.0;
    for (int i = 1; i < N; i++) {
        total = total + buffer[i] * buffer[i-1];
        /* Additional operation to prevent simple optimization */
        buffer[i] = sqrt(fabs(buffer[i])) + 0.001;
    }
    
    *output = total;
    sink = (int)total;
}

int main() {
    int int_result;
    double fp_result;
    
    printf("Starting modulo scheduler test loops...\n");
    
    /* Execute loops with different dependency patterns */
    loop_with_carried_deps(&int_result);
    resource_intensive_loop(&fp_result);
    
    /* Combine results to ensure both loops contribute */
    double final_result = int_result * 0.01 + fp_result;
    
    printf("Results: int=%d, fp=%.6f, final=%.6f\n", 
           int_result, fp_result, final_result);
    
    /* Use results to prevent dead code elimination */
    sink = int_result + (int)fp_result;
    
    return (final_result > 1000.0) ? 0 : 1;
}
