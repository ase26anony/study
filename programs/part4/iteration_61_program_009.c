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

/* Prevent dead code elimination */
static volatile int sink = 0;
static volatile double dsink = 0.0;

/* Functions to create artificial dependencies */
static double expensive_op(double x, double y) {
    /* Multiple high-latency operations */
    return (x / 3.141592653589793) * (y / 2.718281828459045);
}

int main(void) {
    /* Arrays with volatile elements to prevent optimization */
    volatile double array1[N];
    volatile double array2[N];
    volatile int int_array[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = i * 1.5;
        array2[i] = i * 0.7;
        int_array[i] = i;
    }
    
    /* LOOP 1: Simple carried dependency with integer */
    /* This creates distance-1 dependencies: result depends on previous iteration */
    int sum = 0;
    int prev = 1;
    for (int i = 0; i < N; i++) {
        /* Carried dependency: current iteration uses value from previous iteration */
        int current = prev * 2 + int_array[i];
        sum += current;
        prev = current % 1000;  /* Modulo to prevent overflow, maintains dependency */
    }
    sink = sum;  /* Force side effect */
    
    /* LOOP 2: Complex floating-point carried dependency chain */
    /* This should create multiple non-zero latency edges */
    double fp_result = 1.0;
    double fp_prev = 1.0;
    
    for (int i = 1; i < N; i++) {
        /* Multiple carried dependencies with high-latency operations */
        double temp1 = fp_prev / 3.14159;  /* High latency division */
        double temp2 = temp1 * array1[i];  /* Multiplication */
        double temp3 = temp2 + array2[i-1]; /* Uses previous array element (distance-1) */
        
        /* Another carried dependency chain */
        fp_prev = fp_prev * 0.99 + temp3;
        fp_result += expensive_op(fp_prev, temp3);
    }
    dsink = fp_result;
    
    /* LOOP 3: Nested dependencies with mixed operations */
    /* Creates complex dependency graph for the scheduler */
    double x = 1.0, y = 2.0, z = 3.0;
    double accum = 0.0;
    
    for (int i = 0; i < M; i++) {
        /* Interdependent carried dependencies */
        double a = x * y + z;          /* Uses x, y, z from previous iteration */
        double b = a / (i + 2.0);      /* High latency division */
        double c = b * array1[i % N];  /* Multiplication */
        
        /* Update for next iteration - creates distance-1 dependencies */
        x = y + c;
        y = z * 0.5;
        z = a - b;
        
        accum += a + b + c;
    }
    dsink += accum;
    
    /* LOOP 4: Array recurrence with distance-1 dependency */
    /* Classic pattern: a[i] depends on a[i-1] */
    double recurrence_array[N];
    recurrence_array[0] = 1.0;
    
    for (int i = 1; i < N; i++) {
        /* Clear distance-1 dependency on array element */
        recurrence_array[i] = recurrence_array[i-1] * 1.1 + array1[i];
        
        /* Additional operation to create more scheduling pressure */
        recurrence_array[i] = recurrence_array[i] / (1.0 + sin(i * 0.01));
    }
    
    /* Compute final result to ensure all loops contribute */
    double final_result = sum + fp_result + accum + recurrence_array[N-1];
    
    printf("Final result: %f\n", final_result);
    
    return 0;
}
