/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debug output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimization */
static volatile int sink;

/* Complex loop 1: Multiple carried dependencies with integer operations */
void loop1(int *result) {
    volatile int array[N];
    int sum = 1;
    int prod = 2;
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < N; i++) {
        array[i] = (i % 7) + 1;
    }
    
    /* Loop with multiple carried dependencies:
     * 1. sum depends on previous sum (distance-1)
     * 2. prod depends on previous prod (distance-1)  
     * 3. prod also depends on current sum (intra-iteration)
     */
    for (int i = 1; i < N; i++) {
        /* Distance-1 dependency: sum[i] = sum[i-1] + ... */
        sum = sum + array[i] * (i % 5);
        
        /* Another distance-1 dependency with multiplication */
        prod = prod * (sum + 1);
        
        /* Use both results to create more complexity */
        array[i] = (array[i-1] + sum) % (prod % 31 + 1);
    }
    
    *result = sum + prod;
    sink = *result; /* Ensure side effect */
}

/* Complex loop 2: Floating-point operations with high latency */
void loop2(double *result) {
    volatile double data[M];
    double x = 3.14159;
    double y = 2.71828;
    
    /* Initialize with varying values */
    for (int i = 0; i < M; i++) {
        data[i] = sin(i * 0.1) + 2.0;
    }
    
    /* Loop with floating-point carried dependencies and high-latency ops */
    for (int i = 1; i < M; i++) {
        /* Distance-1 dependency with division (high latency) */
        x = x / 1.234567 + data[i];
        
        /* Another distance-1 dependency with multiplication */
        y = y * 0.987654 - x;
        
        /* Cross-dependency between x and y */
        data[i] = (data[i-1] * x) / (y + 1.0);
        
        /* Additional high-latency operation */
        x = sqrt(fabs(x)) + cos(y * 0.01);
    }
    
    *result = x + y;
    sink = (int)*result; /* Ensure side effect */
}

/* Loop 3: Mixed integer/float with complex dependency chain */
void loop3(float *result) {
    volatile float arr[N/2];
    float acc1 = 1.0f;
    float acc2 = 2.0f;
    int counter = 0;
    
    for (int i = 0; i < N/2; i++) {
        arr[i] = (i % 13) * 0.5f;
    }
    
    /* Complex dependency pattern:
     * acc1[i] depends on acc1[i-1] AND acc2[i-1] (distance-1)
     * acc2[i] depends on acc1[i] (intra) AND acc2[i-1] (distance-1)
     */
    for (int i = 1; i < N/2; i++) {
        /* Multiple uses create pressure on scheduler */
        float temp = arr[i-1] * 1.1f;
        
        /* Distance-1 dependency chain */
        acc1 = acc1 * 0.9f + temp;
        
        /* Another distance-1 with intra-iteration dependency */
        acc2 = acc2 / 1.05f + acc1;
        
        /* Use both accumulators */
        arr[i] = (arr[i] + acc1) / (acc2 + 0.001f);
        
        /* Integer operation mixed in */
        counter = (counter + (int)acc1) % 100;
    }
    
    *result = acc1 + acc2 + counter;
    sink = (int)*result;
}

int main() {
    int res1;
    double res2;
    float res3;
    
    printf("Starting modulo scheduler test loops...\n");
    
    /* Execute all loops to create various dependency patterns */
    loop1(&res1);
    loop2(&res2);
    loop3(&res3);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %.6f, %.6f\n", res1, res2, res3);
    
    /* Additional volatile store */
    sink = res1 + (int)res2 + (int)res3;
    
    return 0;
}
