/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be used and prevent optimization */
volatile int force_use;
volatile double dforce_use;

/* Complex loop with multiple carried dependencies and resource contention */
void complex_loop_with_dependencies(void) {
    /* Arrays with volatile elements to prevent optimization */
    volatile double arr1[N], arr2[N], arr3[N];
    volatile int int_arr[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 2.5;
        arr3[i] = i * 0.5;
        int_arr[i] = i;
    }
    
    /* Loop 1: Simple carried dependency with integer */
    /* distance-1 dependency: result depends on previous iteration */
    int sum = 1;
    int prod = 1;
    for (int i = 1; i < N; i++) {
        /* Carried dependency: sum[i] depends on sum[i-1] */
        sum = sum + int_arr[i] * 3;
        
        /* Another carried dependency with multiplication */
        prod = prod * (sum % 7 + 1);
        
        /* Use result to prevent dead code elimination */
        int_arr[i] = (sum + prod) % 100;
    }
    force_use = sum + prod;
    
    /* Loop 2: Complex floating-point carried dependencies with high latency ops */
    /* This creates non-zero this_latency values for the debug output */
    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    
    for (int i = 1; i < M; i++) {
        /* Multiple carried dependencies creating complex scheduling constraints */
        
        /* Distance-1 dependency chain 1: x depends on previous x */
        x = x / 3.14159 * arr1[i] + sin(y);
        
        /* Distance-1 dependency chain 2: y depends on previous y */
        y = y * 1.61803 / arr2[i] + cos(z);
        
        /* Distance-1 dependency chain 3: z depends on previous z */
        z = z + tan(x) * arr3[i];
        
        /* Cross-dependency between chains to create more complex graph */
        arr1[i] = x * y;
        arr2[i] = y * z;
        arr3[i] = z * x;
        
        /* Integer operation mixed in to create different resource usage */
        int_arr[i] = (int)(x + y + z) % 256;
    }
    
    /* Force use of results */
    dforce_use = x + y + z;
    force_use += int_arr[M-1];
    
    /* Loop 3: Nested carried dependencies with array accesses */
    /* Creates distance1_uses condition for the debug output */
    double buffer[N];
    for (int i = 0; i < N; i++) {
        buffer[i] = i * 0.1;
    }
    
    double accum = buffer[0];
    for (int i = 1; i < N; i++) {
        /* Classic recurrence: a[i] = a[i-1] * c + b[i] */
        /* This creates the distance1_uses condition */
        accum = accum * 1.1 + buffer[i] / (i + 1.0);
        
        /* Additional operation with carried dependency */
        buffer[i] = accum * exp(-i * 0.01);
    }
    
    dforce_use += accum;
}

/* Another loop with artificial resource bottlenecks */
void resource_intensive_loop(void) {
    volatile double a[M], b[M], c[M];
    double result = 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < M; i++) {
        a[i] = i * 0.25;
        b[i] = i * 0.75;
        c[i] = i * 1.25;
    }
    
    /* Loop with multiple high-latency operations competing for FP units */
    for (int i = 1; i < M - 1; i++) {
        /* Multiple divisions - high latency operations */
        double t1 = a[i] / 3.1415926535;
        double t2 = b[i] / 2.7182818284;
        double t3 = c[i] / 1.4142135623;
        
        /* Carried dependency chain */
        a[i] = a[i-1] * t1 + t2;
        b[i] = b[i-1] / (t3 + 0.1);
        c[i] = c[i-1] + a[i] * b[i];
        
        /* More operations to increase register pressure */
        result += a[i] * b[i] * c[i];
    }
    
    dforce_use = result;
}

int main(void) {
    printf("Starting modulo scheduling test loops...\n");
    
    /* Run loops that should trigger modulo scheduler */
    complex_loop_with_dependencies();
    resource_intensive_loop();
    
    /* Print something to ensure loops aren't optimized away */
    printf("Results: force_use = %d, dforce_use = %f\n", 
           force_use, dforce_use);
    
    /* Additional computation to ensure loops are needed */
    double final_check = dforce_use * 2.0 + force_use;
    printf("Final check value: %f\n", final_check);
    
    return (int)final_check % 100;
}
