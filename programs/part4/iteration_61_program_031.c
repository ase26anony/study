/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debug output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Force variables to be preserved through optimization */
static volatile int sink __attribute__((used));

/* Complex loop with multiple carried dependencies */
void loop_with_carried_deps(void) {
    /* Arrays with volatile elements to prevent optimization */
    volatile double arr1[N];
    volatile double arr2[N];
    volatile double arr3[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = (double)(i % 37) * 1.5;
        arr2[i] = (double)(i % 41) * 2.3;
        arr3[i] = (double)(i % 43) * 3.7;
    }
    
    /* Loop 1: Simple carried dependency with integer */
    volatile int sum = 0;
    for (int i = 1; i < N; i++) {
        /* Distance-1 dependency: sum[i] depends on sum[i-1] */
        sum = sum + (int)arr1[i] * (int)arr2[i-1];
        /* Additional operation to create resource pressure */
        arr3[i] = arr3[i-1] * 2.71828 / 3.14159;
    }
    sink = sum;
    
    /* Loop 2: Complex FP carried dependency chain */
    volatile double fp_acc = 1.0;
    for (int i = 1; i < N; i++) {
        /* Multiple carried dependencies creating complex scheduling */
        double temp1 = arr1[i] / 2.5;          /* High latency division */
        double temp2 = temp1 * arr2[i-1];      /* Multiplication */
        fp_acc = fp_acc * temp2;               /* Carried dependency */
        
        /* Another independent carried dependency */
        arr3[i] = (arr3[i-1] + arr1[i]) / 1.414;
        
        /* Mix operations to compete for FP units */
        arr2[i] = arr2[i-1] * 3.14159 / temp1;
    }
    sink = (int)fp_acc;
    
    /* Loop 3: Nested carried dependencies with array access */
    volatile double result[N];
    result[0] = arr1[0];
    
    for (int i = 1; i < N; i++) {
        /* Multiple distance-1 dependencies */
        double a = result[i-1] * arr2[i];      /* Dependency chain 1 */
        double b = a / (arr3[i-1] + 1.0);      /* Dependency chain 2 */
        double c = b * 1.618034;               /* Dependency chain 3 */
        result[i] = c + arr1[i-1];             /* Final result with carried dep */
        
        /* Additional operation with high latency */
        arr2[i] = arr2[i-1] / 3.333 + arr1[i];
    }
    sink = (int)result[N-1];
}

/* Loop with artificial resource bottlenecks */
void loop_with_resource_conflicts(void) {
    volatile double data[M];
    volatile double coeff[M];
    
    /* Initialize with non-trivial values */
    for (int i = 0; i < M; i++) {
        data[i] = (double)(i * i % 97) + 0.5;
        coeff[i] = (double)(i * 3 % 73) + 0.3;
    }
    
    /* Complex loop with multiple high-latency operations */
    volatile double accum = 1.0;
    volatile double prod = 1.0;
    
    for (int i = 1; i < M; i++) {
        /* Create resource conflicts: multiple divides compete for FP divider */
        double t1 = data[i] / coeff[i-1];      /* Division 1 */
        double t2 = coeff[i] / (data[i-1] + 1.0); /* Division 2 */
        
        /* Carried dependencies with different distances */
        accum = accum * t1;                    /* Distance-1 */
        prod = prod / (t2 + accum);            /* Distance-1, depends on accum */
        
        /* Additional operation to increase II */
        data[i] = data[i-1] * 0.99 + t1 * t2;
    }
    
    /* Use results to prevent elimination */
    sink = (int)(accum + prod);
}

/* Main function with observable side effects */
int main(void) {
    printf("Starting modulo scheduler test loops...\n");
    
    /* Run loops that should trigger modulo scheduling */
    loop_with_carried_deps();
    loop_with_resource_conflicts();
    
    /* Compute and print final result to ensure loops aren't dead code */
    volatile int final_result = sink;
    printf("Test completed. Final marker: %d\n", final_result);
    
    return 0;
}
