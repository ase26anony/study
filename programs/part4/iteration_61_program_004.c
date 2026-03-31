/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Force variables to be used and prevent optimization */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
void loop_with_carried_deps(int *result1, int *result2) {
    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    volatile double coeffs[N];
    volatile double data[N];
    
    /* Initialize arrays to prevent constant propagation */
    for (int i = 0; i < N; i++) {
        coeffs[i] = 1.5 + (i % 7);
        data[i] = 0.8 + (i % 5);
    }
    
    /* 
     * Loop 1: Multiple carried dependencies with floating-point operations
     * This creates distance-1 dependencies with varying latencies
     */
    for (int i = 1; i < N; i++) {
        /* Distance-1 dependency: x[i] depends on x[i-1] */
        x = x / 3.14159 * coeffs[i] + data[i];
        
        /* Another distance-1 dependency chain */
        y = y * 1.61803 - x / 2.0;
        
        /* Cross-dependency between chains */
        z = (x + y) / z * 1.41421;
        
        /* Array-based carried dependency */
        coeffs[i] = coeffs[i-1] * 0.99 + z;
    }
    
    *result1 = (int)(x + y + z);
    
    /* 
     * Loop 2: Integer operations with resource contention
     * Mix of multiplications and divisions to stress functional units
     */
    int a = 1, b = 2, c = 3;
    volatile int arr[M];
    
    for (int i = 0; i < M; i++) {
        arr[i] = i * 2;
    }
    
    for (int i = 1; i < M; i++) {
        /* Complex carried dependency chain with integer ops */
        a = a * 17 + arr[i] / 3;      /* High latency multiplication + division */
        b = b * 13 + a % 7;           /* Another multiplication chain */
        c = (c * 11 + b) / 5;         /* Mix of operations */
        
        /* Array recurrence with distance-1 */
        arr[i] = arr[i-1] + a - b + c;
    }
    
    *result2 = a + b + c;
    
    /* Ensure results are used */
    sink = *result1 + *result2;
}

/* Loop with nested dependencies and high II potential */
void loop_with_nested_deps(double *final_result) {
    volatile double vec1[N], vec2[N], vec3[N];
    double sum = 0.0;
    double prod = 1.0;
    
    /* Initialize with non-trivial patterns */
    for (int i = 0; i < N; i++) {
        vec1[i] = (i % 13) * 1.1;
        vec2[i] = (i % 17) * 0.9;
        vec3[i] = (i % 19) * 1.3;
    }
    
    /*
     * Complex loop with interdependent carried dependencies:
     * 1. sum depends on previous sum (distance-1)
     * 2. prod depends on current sum
     * 3. vec3 depends on previous vec3 and current prod
     * This creates a web of dependencies forcing non-trivial II
     */
    for (int i = 1; i < N; i++) {
        /* First carried dependency chain */
        sum = sum + vec1[i] * vec2[i];
        
        /* Second chain depends on first */
        prod = prod * (sum + 1.0) / 2.71828;
        
        /* Third chain with distance-1 array dependency */
        vec3[i] = vec3[i-1] * prod + vec1[i];
        
        /* Additional operations to increase pressure */
        vec1[i] = vec1[i] / 1.41421 + vec3[i] * 0.7071;
        vec2[i] = vec2[i-1] * 0.99 + prod / 3.14159;
    }
    
    *final_result = sum + prod + vec3[N-1];
    sink = (int)(*final_result);
}

/* Main function to drive everything */
int main() {
    int res1, res2;
    double final_res;
    
    printf("Starting modulo scheduling test loops...\n");
    
    /* Execute loops with carried dependencies */
    loop_with_carried_deps(&res1, &res2);
    
    /* Execute loop with nested dependencies */
    loop_with_nested_deps(&final_res);
    
    /* Print results to ensure side effects */
    printf("Result 1: %d\n", res1);
    printf("Result 2: %d\n", res2);
    printf("Final result: %f\n", final_res);
    
    /* Use results in computation */
    int total = res1 + res2 + (int)final_res;
    printf("Total: %d\n", total);
    
    return total % 100;  /* Non-zero return to prevent optimization */
}

/* Additional function with different pattern */
__attribute__((used)) 
static void extra_loop_for_coverage() {
    volatile int buffer[100];
    int acc = 42;
    
    /* Simple but non-trivial carried dependency */
    for (int i = 1; i < 100; i++) {
        buffer[i] = buffer[i-1] * 3 + acc;
        acc = acc + buffer[i] / 2;
    }
    
    sink = acc;
}
