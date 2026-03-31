/* test_modulo_sched.c
 * Program designed to trigger modulo scheduler debug output in GCC
 * by creating loops with carried dependencies and resource constraints
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be used and prevent optimization */
static volatile int g_sink = 0;
static volatile double g_double_sink = 0.0;

/* Complex loop with multiple carried dependencies */
void loop_with_carried_deps(int *result) {
    volatile int array1[N];
    volatile int array2[N];
    int temp[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = i % 37;
        array2[i] = (i * 3) % 41;
    }
    
    /* Loop 1: Simple carried dependency with integer operations */
    int sum = 1;
    int prod = 1;
    for (int i = 1; i < N; i++) {
        /* Distance-1 dependency: sum[i] depends on sum[i-1] */
        sum = sum + array1[i] * 3;
        
        /* Another distance-1 dependency: prod[i] depends on prod[i-1] */
        prod = prod * (sum % 17 + 1);
        
        /* Cross dependency between sum and prod */
        temp[i] = (sum + prod) % 100;
    }
    
    /* Loop 2: More complex with floating point operations */
    volatile double fp_array[M];
    double x = 1.0;
    double y = 2.0;
    
    for (int i = 0; i < M; i++) {
        fp_array[i] = sin(i * 0.1);
    }
    
    /* Loop with floating-point carried dependencies and high-latency ops */
    for (int i = 1; i < M; i++) {
        /* Distance-1 dependency with floating-point division (high latency) */
        x = x / 3.14159 + fp_array[i];
        
        /* Another distance-1 dependency with multiplication */
        y = y * 1.23456 - fp_array[i-1];
        
        /* Cross iteration dependency mixing x and y */
        fp_array[i] = (x * y) / (fp_array[i-1] + 1.0);
    }
    
    /* Loop 3: Nested dependencies with array accesses */
    double acc = 0.0;
    for (int i = 2; i < M; i++) {
        /* Multiple distance-1 dependencies */
        double t1 = fp_array[i-1] * 2.0;
        double t2 = fp_array[i-2] / 1.5;
        
        /* Complex expression with carried dependency chain */
        acc = acc + (t1 - t2) * (fp_array[i] + 1.0);
        
        /* Store back creating anti-dependency */
        fp_array[i] = acc * 0.9;
    }
    
    /* Compute final result to prevent dead code elimination */
    *result = (int)(sum + prod + (int)acc);
    g_sink = *result;
    g_double_sink = x + y + acc;
}

/* Another loop with different pattern */
void loop_with_resource_conflict(int *result) {
    volatile int data[N];
    int accum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        data[i] = (i * i) % 100;
    }
    
    /* Loop with multiple competing operations */
    for (int i = 1; i < N; i++) {
        /* Integer multiplication (often has latency) */
        int mult = data[i] * data[i-1];
        
        /* Modulo operation */
        int mod = mult % 97;
        
        /* Carried dependency chain */
        accum = accum + mod;
        
        /* Store with anti-dependency */
        data[i] = accum & 0xFF;
        
        /* Additional operation creating register pressure */
        int extra = (accum * 3) / (data[i-1] + 1);
        g_sink = extra;  /* Force side effect */
    }
    
    *result = accum;
}

int main() {
    int result1, result2;
    
    printf("Starting modulo scheduler test loops...\n");
    
    /* First complex loop */
    loop_with_carried_deps(&result1);
    printf("Loop 1 result: %d\n", result1);
    
    /* Second loop with different characteristics */
    loop_with_resource_conflict(&result2);
    printf("Loop 2 result: %d\n", result2);
    
    /* Final computation using both results */
    int final_result = result1 * 3 + result2 * 7;
    printf("Final result: %d\n", final_result);
    
    /* Use volatile sinks to prevent optimization */
    printf("Volatile sinks: %d, %f\n", g_sink, g_double_sink);
    
    return final_result % 100;
}
