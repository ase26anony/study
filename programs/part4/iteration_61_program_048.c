/* test_modulo_sched.c
 * 
 * This program creates loops with specific characteristics to trigger
 * the modulo scheduler's debug output in GCC's RTL optimizer.
 * 
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 * 
 * The loops are designed to:
 * 1. Have cross-iteration dependencies (distance-1)
 * 2. Include high-latency operations
 * 3. Create resource contention
 * 4. Prevent optimization removal
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Prevent dead code elimination */
volatile int g_sink = 0;
volatile double d_sink = 0.0;

/* Complex loop with multiple carried dependencies */
void loop_with_carried_deps(void) {
    volatile int array1[N];
    volatile int array2[N];
    int result[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 3) % 17;
        array2[i] = (i * 5) % 23;
    }
    
    /* Loop 1: Simple carried dependency with integer operations */
    int sum = 1;
    int prod = 1;
    
    for (int i = 1; i < N; i++) {
        /* Distance-1 dependency: sum[i] depends on sum[i-1] */
        sum = sum + array1[i] * 3;
        
        /* Another distance-1 dependency: prod[i] depends on prod[i-1] */
        prod = prod * (sum % 7 + 1);
        
        /* Store result to prevent optimization */
        result[i] = sum + prod;
    }
    
    g_sink = sum + prod;
    
    /* Loop 2: More complex with floating-point operations */
    volatile double fp_array[N];
    double fp_result[N];
    
    for (int i = 0; i < N; i++) {
        fp_array[i] = (i * 1.5) / 3.0;
    }
    
    double x = 1.0;
    double y = 2.0;
    
    for (int i = 1; i < N; i++) {
        /* Multiple high-latency FP operations with carried dependencies */
        x = x / 3.14159 * fp_array[i] + y;  /* x[i] depends on x[i-1] */
        y = y * 1.61803 - x / 2.71828;      /* y[i] depends on y[i-1] and x[i] */
        
        /* Create resource contention with division */
        fp_result[i] = x / y + fp_array[i-1];  /* Uses value from previous iteration */
    }
    
    d_sink = x + y;
}

/* Loop with nested dependencies and array accesses */
void loop_with_nested_deps(void) {
    volatile int data[M][M];
    int output[M];
    
    /* Initialize 2D array */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (i * j) % 31;
        }
    }
    
    /* Complex loop with multiple interleaved dependencies */
    int acc1 = 1, acc2 = 2, acc3 = 3;
    
    for (int i = 1; i < M; i++) {
        /* Chain of dependencies across iterations */
        acc1 = acc1 * 2 + data[i][0];           /* Distance-1 dependency */
        acc2 = acc2 + acc1 * 3;                 /* Depends on acc1 from same iteration */
        acc3 = acc3 * acc2 - data[i-1][1];      /* Distance-1 dependency on data */
        
        /* Create anti-dependencies by reusing variables */
        int temp = acc1;
        acc1 = acc3;
        acc3 = temp + acc2;
        
        output[i] = acc1 + acc2 + acc3;
    }
    
    g_sink = acc1 + acc2 + acc3;
}

/* Loop designed specifically for modulo scheduler */
void modulo_sched_target_loop(void) {
    volatile int a[N], b[N], c[N];
    int result[N];
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        a[i] = i % 19;
        b[i] = (i * 7) % 29;
        c[i] = (i * 11) % 37;
    }
    
    /* This loop has the exact characteristics needed:
     * 1. Clear distance-1 dependency (a[i] depends on a[i-1])
     * 2. Mixed operations with different latencies
     * 3. Multiple uses of the same value creating complex dependencies
     */
    int accum = a[0];
    
    for (int i = 1; i < N; i++) {
        /* Core recurrence: distance-1 carried dependency */
        int temp = accum * 3 + b[i];
        
        /* Another operation using the result */
        accum = temp / 7 + c[i];  /* Division is high-latency */
        
        /* Use previous iteration's value creating anti-dependency */
        result[i] = accum + a[i-1];
        
        /* Additional computation to increase register pressure */
        b[i] = b[i] * accum % 17;
        c[i] = c[i] + temp % 13;
    }
    
    /* Force result to be used */
    int final = 0;
    for (int i = 0; i < N; i++) {
        final += result[i];
    }
    g_sink = final;
}

int main(void) {
    printf("Starting modulo scheduler test loops...\n");
    
    /* Run all loops to ensure coverage */
    loop_with_carried_deps();
    loop_with_nested_deps();
    modulo_sched_target_loop();
    
    /* Print results to prevent optimization */
    printf("Results: int=%d, double=%f\n", g_sink, d_sink);
    
    return 0;
}
