/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be kept and prevent optimizations */
static volatile int sink;
static volatile double dsink;

/* Function with multiple loops exhibiting different dependency patterns */
double compute_loop_patterns(void) {
    /* Loop 1: Integer carried dependency with arithmetic operations */
    int array1[N];
    int result1 = 1;
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i % 7) + 1;
    }
    
    /* Core loop with distance-1 dependency and mixed operations */
    for (int i = 1; i < N; i++) {
        /* Carried dependency: uses value from previous iteration */
        result1 = result1 * 3 + array1[i];
        /* Additional operation to create resource pressure */
        array1[i] = (array1[i-1] + result1) / 2;
    }
    
    /* Loop 2: Floating-point with high-latency operations */
    double array2[M];
    double result2 = 1.0;
    double coeff = 3.14159;
    
    /* Initialize with varying values */
    for (int i = 0; i < M; i++) {
        array2[i] = sin(i * 0.1) + 1.0;
    }
    
    /* Loop with floating-point division/multiplication chain */
    for (int i = 1; i < M; i++) {
        /* Complex carried dependency chain with high-latency ops */
        double temp = result2 / coeff;          /* Division - high latency */
        temp = temp * array2[i];                /* Multiplication */
        result2 = temp + array2[i-1];           /* Distance-1 dependency */
        
        /* Additional dependency to increase II */
        array2[i] = array2[i-1] * 0.99 + temp * 0.01;
    }
    
    /* Loop 3: Nested dependencies and multiple accumulators */
    int array3[N];
    int sum = 0;
    int prod = 1;
    
    for (int i = 0; i < N; i++) {
        array3[i] = i + 1;
    }
    
    for (int i = 1; i < N; i++) {
        /* Interdependent carried dependencies */
        sum = sum + array3[i] * array3[i-1];    /* Distance-1 use */
        prod = prod * (sum % 17 + 1);           /* Depends on sum */
        array3[i] = (array3[i-1] + prod) % 256; /* Another distance-1 */
    }
    
    /* Loop 4: Mixed types and operations to stress scheduler */
    float array4[M];
    float accum = 1.0f;
    
    for (int i = 0; i < M; i++) {
        array4[i] = (i % 5) * 0.5f;
    }
    
    for (int i = 2; i < M; i++) {
        /* Multiple distance dependencies */
        float t1 = array4[i-1] * accum;         /* Distance-1 */
        float t2 = array4[i-2] / 2.0f;          /* Distance-2 */
        accum = t1 + t2 + array4[i];
        array4[i] = accum * 0.9f;
    }
    
    /* Combine results to prevent dead code elimination */
    double final_result = (double)result1 * result2 * sum * prod * accum;
    
    /* Use volatile sinks to ensure computations aren't optimized away */
    sink = result1 + sum + prod;
    dsink = result2 + accum;
    
    return final_result;
}

/* Main function with observable side effects */
int main(void) {
    double result = compute_loop_patterns();
    
    /* Print result to create observable side effect */
    printf("Result: %f\n", result);
    
    /* Additional volatile use to keep everything alive */
    printf("Sink values: %d, %f\n", sink, dsink);
    
    return 0;
}
