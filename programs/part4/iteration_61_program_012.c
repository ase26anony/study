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

/* Function with multiple carried dependencies and resource contention */
void compute_loop_with_dependencies(void) {
    /* Arrays with volatile elements to prevent elimination */
    volatile double arr1[N];
    volatile double arr2[N];
    volatile int int_arr[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i % 7) * 1.5;
        arr2[i] = (i % 5) * 2.3;
        int_arr[i] = i * 3;
    }
    
    /* LOOP 1: Simple carried dependency with integer */
    /* Distance-1 dependency: result[i] depends on result[i-1] */
    int result = 1;
    for (int i = 1; i < N; i++) {
        /* Carried dependency chain */
        result = result * 3 + int_arr[i];
        /* Use result in another computation to create more dependencies */
        int_arr[i] = (result % 17) + int_arr[i-1];  /* Another distance-1 dependency */
    }
    g_sink = result;  /* Prevent dead code elimination */
    
    /* LOOP 2: Complex floating-point with multiple carried dependencies */
    /* This creates high-latency operations and resource contention */
    double fp_result = 1.0;
    double fp_accum = 0.0;
    
    for (int i = 1; i < M; i++) {
        /* Multiple carried dependencies with floating-point operations */
        /* High-latency division creates resource pressure */
        fp_result = fp_result / 3.14159 * arr1[i] + arr2[i-1];  /* Distance-1 dependency */
        
        /* Another carried dependency chain */
        fp_accum = fp_accum + fp_result * arr2[i];
        
        /* Cross-dependency between the two chains */
        arr1[i] = fp_accum * 0.5 + arr1[i-1];  /* Another distance-1 dependency */
        
        /* Additional high-latency operation to increase II */
        fp_result = sqrt(fabs(fp_result)) * 2.0;
    }
    g_double_sink = fp_result + fp_accum;
    
    /* LOOP 3: Nested dependencies with mixed operations */
    /* Creates complex dependency graph for the scheduler */
    double x = 1.0, y = 2.0, z = 3.0;
    for (int i = 1; i < N/2; i++) {
        /* Interleaved carried dependencies */
        x = x * y + int_arr[i];          /* x[i] depends on x[i-1] */
        y = y / 2.71828 + x;             /* y[i] depends on y[i-1] AND x[i] */
        z = z + x * y;                   /* z[i] depends on z[i-1], x[i], y[i] */
        
        /* Use in array computation with carried dependency */
        arr2[i] = arr2[i-1] + z * 0.25;  /* Distance-1 dependency */
    }
    g_double_sink += x + y + z;
}

/* Another function with different pattern to increase coverage chances */
void compute_with_recurrences(void) {
    volatile double a[N], b[N], c[N];
    volatile int counters[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1;
        b[i] = i * 0.2;
        c[i] = i * 0.3;
        counters[i] = i;
    }
    
    /* Complex recurrence relations */
    double sum = 0.0;
    double prod = 1.0;
    
    for (int i = 1; i < N; i++) {
        /* Multiple recurrence relations with different distances */
        a[i] = a[i-1] * b[i] + c[i];      /* Distance-1 */
        b[i] = b[i-1] / 1.414 + a[i];     /* Distance-1 with use of a[i] */
        
        /* Cross-iteration dependency with latency */
        sum = sum + a[i] * b[i-1];        /* Uses b from previous iteration */
        
        /* Another chain with high latency */
        prod = prod * (sum + 1.0);
        
        /* Integer carried dependency */
        counters[i] = counters[i-1] + (int)(prod) % 256;
    }
    
    g_sink = (int)sum + (int)prod + counters[N-1];
}

int main(void) {
    printf("Starting modulo scheduling test...\n");
    
    /* Call functions with loops designed for modulo scheduling */
    compute_loop_with_dependencies();
    compute_with_recurrences();
    
    /* Compute final result to ensure all computations are used */
    double final_result = (double)g_sink + g_double_sink;
    
    printf("Final result: %f\n", final_result);
    printf("Test completed. Check RTL dump files for modulo scheduler output.\n");
    
    return 0;
}
