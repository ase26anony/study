/* test_modulo_sched.c
 * 
 * This program creates loops with specific characteristics to trigger
 * the modulo scheduler's debug output in GCC's RTL optimizer.
 * The loops contain:
 * 1. Cross-iteration dependencies (distance-1 carried dependencies)
 * 2. High-latency operations (floating-point division)
 * 3. Multiple interdependent operations
 * 4. Volatile variables to prevent optimization
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Volatile variables to prevent dead code elimination */
volatile double sink1 = 0.0;
volatile double sink2 = 0.0;
volatile int sink3 = 0;

/* Function with loop containing distance-1 carried dependency */
double loop_with_carried_dependency(double init, const double* b, double c) {
    double a[N];
    volatile double result = 0.0;
    
    /* Initialize first element */
    a[0] = init;
    
    /* Loop with carried dependency: a[i] depends on a[i-1]
     * This creates distance-1 dependencies for the scheduler */
    for (int i = 1; i < N; i++) {
        /* Complex expression with multiple operations:
         * - Division (high latency)
         * - Multiplication
         * - Addition
         * All create scheduling challenges */
        a[i] = (a[i-1] * c + b[i]) / 3.141592653589793;
        
        /* Additional operation to create more scheduling pressure */
        a[i] = a[i] * sin((double)i * 0.01);
    }
    
    /* Compute result to ensure loop isn't eliminated */
    for (int i = 0; i < N; i++) {
        result += a[i];
    }
    
    return result;
}

/* Function with multiple interdependent carried dependencies */
double complex_dependency_loop(double init1, double init2, const double* data) {
    double x = init1;
    double y = init2;
    volatile double sum = 0.0;
    
    /* Loop with two interdependent carried dependencies:
     * x depends on previous x AND y
     * y depends on previous y AND x
     * This creates complex scheduling constraints */
    for (int i = 0; i < M; i++) {
        /* First carried dependency chain */
        double temp_x = x * 1.5 + y * 0.7;
        
        /* High-latency operation in the dependency chain */
        temp_x = temp_x / (data[i] + 2.0);
        
        /* Second carried dependency chain */
        double temp_y = y * 0.9 + x * 0.3;
        temp_y = temp_y / (cos((double)i * 0.05) + 1.5);
        
        /* Update both values for next iteration */
        x = temp_x;
        y = temp_y;
        
        /* Accumulate to prevent elimination */
        sum += x + y;
    }
    
    return sum;
}

/* Integer loop with carried dependency and mixed operations */
int integer_carried_dependency(int init, const int* arr) {
    int acc = init;
    volatile int result = 0;
    
    /* Integer loop with carried dependency and
     * mixed operations (multiplication, addition) */
    for (int i = 0; i < N; i++) {
        /* Carried dependency on acc */
        acc = acc * 3 + arr[i];
        
        /* Additional operation that uses acc */
        int temp = acc % 17;
        
        /* Another carried dependency-like pattern */
        result = result ^ (acc + temp);
    }
    
    return result;
}

int main(void) {
    /* Initialize data arrays */
    double b[N];
    double data[M];
    int arr[N];
    
    /* Fill arrays with non-trivial values */
    for (int i = 0; i < N; i++) {
        b[i] = sin((double)i * 0.1) * 10.0;
        arr[i] = (i * 7) % 13;
    }
    
    for (int i = 0; i < M; i++) {
        data[i] = cos((double)i * 0.07) * 5.0 + 1.0;
    }
    
    /* Execute loops with carried dependencies */
    double result1 = loop_with_carried_dependency(1.0, b, 2.5);
    double result2 = complex_dependency_loop(1.0, 2.0, data);
    int result3 = integer_carried_dependency(42, arr);
    
    /* Store results in volatile sinks to prevent optimization */
    sink1 = result1;
    sink2 = result2;
    sink3 = result3;
    
    /* Print results to create observable side effect */
    printf("Results: %f, %f, %d\n", result1, result2, result3);
    
    return 0;
}
