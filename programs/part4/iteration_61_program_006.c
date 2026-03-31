/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be preserved through optimization */
static volatile int g_sink = 0;
static double g_final_result = 0.0;

/* Function with multiple carried dependencies and resource contention */
void compute_loop_with_dependencies(void) {
    /* Arrays with volatile elements to prevent optimization */
    volatile double array1[N];
    volatile double array2[N];
    volatile double results[N];
    
    /* Initialize arrays with non-trivial values */
    for (int i = 0; i < N; i++) {
        array1[i] = sin(i * 0.1) + 1.0;
        array2[i] = cos(i * 0.05) * 2.0;
    }
    
    /* LOOP 1: Simple carried dependency with integer */
    volatile int accumulator = 1;
    for (int i = 1; i < N; i++) {
        /* Distance-1 dependency: accumulator from previous iteration */
        accumulator = accumulator * 2 + i;
        /* Use result to prevent dead code elimination */
        results[i] = accumulator * 0.01;
    }
    g_sink = accumulator;  /* Force side effect */
    
    /* LOOP 2: Complex floating-point carried dependencies */
    double fp_accum1 = 1.0;
    double fp_accum2 = 2.0;
    
    for (int i = 1; i < M; i++) {
        /* Multiple carried dependencies creating complex DDG */
        
        /* Distance-1 dependency chain 1: fp_accum1 depends on previous iteration */
        fp_accum1 = fp_accum1 / 3.14159 * array1[i];  /* High latency divide */
        
        /* Distance-1 dependency chain 2: fp_accum2 depends on previous iteration */
        fp_accum2 = fp_accum2 * 1.61803 + array2[i];  /* High latency multiply */
        
        /* Cross-dependency between the two chains */
        double temp = fp_accum1 * fp_accum2;  /* Another multiply */
        
        /* Use both accumulators to create resource contention */
        results[i] = (fp_accum1 + fp_accum2) / temp;  /* Another divide */
        
        /* Additional operation to increase register pressure */
        fp_accum1 = fp_accum1 + sin(temp * 0.1);
    }
    
    g_final_result = fp_accum1 + fp_accum2;
}

/* Second function with nested loops and array dependencies */
void compute_with_array_recurrence(void) {
    volatile double data[N];
    volatile double coeffs[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        data[i] = i * 0.5;
        coeffs[i] = 1.0 / (i + 1.0);
    }
    
    /* LOOP 3: Array-based recurrence with multiple dependencies */
    for (int i = 2; i < N - 2; i++) {
        /* Multiple distance-1 dependencies on array elements */
        double val1 = data[i-1] * coeffs[i];      /* Multiply */
        double val2 = data[i-2] / coeffs[i-1];    /* Divide - different latency */
        
        /* Complex expression with carried dependencies */
        data[i] = (val1 + val2) * 0.5 / (1.0 + val1 * val2);
        
        /* Additional operation with dependency on computed value */
        coeffs[i] = coeffs[i] * data[i] + 0.1;
    }
    
    /* Use results to prevent elimination */
    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    g_sink = (int)sum;
}

/* Third function with mixed integer/floating point operations */
void mixed_operations_loop(void) {
    volatile int int_data[N];
    volatile double dbl_data[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        int_data[i] = i % 37;
        dbl_data[i] = sqrt(i + 1.0);
    }
    
    /* LOOP 4: Mixed operations with carried dependencies */
    int int_acc = int_data[0];
    double dbl_acc = dbl_data[0];
    
    for (int i = 1; i < N; i++) {
        /* Integer carried dependency */
        int_acc = (int_acc * 13 + int_data[i]) % 1001;
        
        /* Floating point carried dependency with high-latency operations */
        dbl_acc = dbl_acc / 2.71828 + dbl_data[i] * log(i + 1.0);
        
        /* Cross-dependency between integer and float */
        dbl_data[i] = dbl_acc * int_acc;
        
        /* Another operation using both */
        int_data[i] = (int)(dbl_data[i] / (int_acc + 1));
    }
    
    g_final_result += dbl_acc + int_acc;
}

int main(void) {
    printf("Starting modulo scheduler test...\n");
    
    /* Call all computation functions to ensure they're not eliminated */
    compute_loop_with_dependencies();
    compute_with_array_recurrence();
    mixed_operations_loop();
    
    /* Print results to create observable side effects */
    printf("Final sink value: %d\n", g_sink);
    printf("Final result: %f\n", g_final_result);
    
    return 0;
}
