/* test_modulo_sched.c
 * 
 * This program creates loops with specific patterns to trigger
 * GCC's modulo scheduler debug output for dependency edges.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimizations from removing our loops */
static volatile int sink;

/* Function with multiple loops exhibiting different dependency patterns */
void compute_loops(void) {
    /* Loop 1: Simple carried dependency with integer operations */
    int array1[N];
    int result1 = 1;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 7 + 3) % 19;
    }
    
    /* Core loop with distance-1 dependency: a[i] depends on a[i-1] */
    /* This creates the distance1_uses condition */
    for (int i = 1; i < N; i++) {
        /* Cross-iteration dependency: result1 from iteration i-1 used in iteration i */
        /* Integer multiplication creates resource pressure */
        result1 = result1 * 3 + array1[i];
        
        /* Additional operation to create more scheduling complexity */
        array1[i] = (array1[i-1] + result1) / 2;
    }
    
    sink = result1;  /* Ensure loop isn't eliminated */
    
    /* Loop 2: Complex floating-point operations with multiple dependencies */
    double array2[M];
    double array3[M];
    double result2 = 1.0;
    double result3 = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < M; i++) {
        array2[i] = sin(i * 0.1) + 2.0;
        array3[i] = cos(i * 0.05) * 3.0;
    }
    
    /* Loop with multiple carried dependencies and high-latency operations */
    for (int i = 1; i < M; i++) {
        /* Multiple cross-iteration dependencies */
        double temp = result2 / 3.14159;  /* High-latency division */
        
        /* Distance-1 dependency on array2 */
        result2 = temp * array2[i] + array2[i-1];
        
        /* Another distance-1 dependency chain */
        result3 = result3 + array3[i] * array3[i-1];
        
        /* Mix operations to create resource conflicts */
        array2[i] = (array2[i-1] + result2) / (result3 + 1.0);
    }
    
    sink = (int)(result2 + result3);
    
    /* Loop 3: Nested dependencies with integer and floating point */
    float array4[N];
    int result4 = 0;
    
    for (int i = 0; i < N; i++) {
        array4[i] = (i % 23) * 0.5f;
    }
    
    for (int i = 2; i < N; i++) {
        /* Complex dependency chain spanning multiple iterations */
        float a = array4[i-2] * 1.7f;
        float b = array4[i-1] / 2.3f;  /* Division creates latency */
        float c = a + b;
        
        /* Cross-iteration dependency with distance 1 */
        array4[i] = c * array4[i] + array4[i-1];
        
        /* Integer operation with carried dependency */
        result4 = (result4 * 2 + (int)array4[i]) % 10007;
    }
    
    sink = result4;
}

/* Loop with artificial resource bottleneck using multiple divisions */
void bottleneck_loop(void) {
    double data1[N], data2[N], data3[N];
    double sum = 0.0;
    
    /* Initialize with values that prevent division by zero */
    for (int i = 0; i < N; i++) {
        data1[i] = (i % 17) + 1.5;
        data2[i] = (i % 13) + 2.5;
        data3[i] = (i % 11) + 0.5;
    }
    
    /* Loop designed to maximize II calculation */
    /* Multiple high-latency divisions in dependency chain */
    for (int i = 1; i < N; i++) {
        /* Chain of divisions - each depends on previous iteration's result */
        double t1 = data1[i] / data2[i-1];      /* Distance-1 dependency */
        double t2 = t1 / data3[i];              /* Intra-iteration dependency */
        double t3 = data2[i] / (t2 + 1.0);      /* Another division */
        
        /* Cross-iteration accumulation */
        sum = sum + t1 * t2 * t3;
        
        /* Update arrays with carried dependencies */
        data1[i] = data1[i-1] * 0.9 + t1;
        data2[i] = data2[i-1] * 0.8 + t2;
    }
    
    sink = (int)sum;
}

/* Main function with observable side effects */
int main(void) {
    int final_result = 0;
    
    printf("Starting modulo scheduler test loops...\n");
    
    /* Execute all loops to generate various dependency patterns */
    compute_loops();
    bottleneck_loop();
    
    /* Create a final result that depends on all computations */
    final_result = sink;
    
    printf("Final result: %d\n", final_result);
    printf("Check generated .rtl dump files for scheduler debug output\n");
    
    return final_result % 100;
}
