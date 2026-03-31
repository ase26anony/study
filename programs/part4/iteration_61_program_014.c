/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debug output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Force variables to be kept in RTL representation */
static volatile int sink;

/* Loop 1: Integer carried dependency with multiplication bottleneck */
int loop1_carried_dependency(void) {
    volatile int array[N];
    int result = 1;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        array[i] = (i * 3) % 17;
    }
    
    /* Main loop with distance-1 carried dependency
     * a[i] depends on a[i-1] creating distance1_uses = true
     * Integer multiplication creates resource pressure */
    for (int i = 1; i < N; i++) {
        /* Complex expression with carried dependency and multiple operations */
        array[i] = (array[i-1] * 7 + array[i] * 3) % 31;
        
        /* Additional operation to increase II */
        result = result * (array[i] + 1);
    }
    
    sink = result;  /* Prevent dead code elimination */
    return result;
}

/* Loop 2: Floating-point with high latency operations */
double loop2_fp_bottleneck(void) {
    volatile double fp_array[M];
    double accumulator = 1.0;
    
    /* Initialize with non-trivial values */
    for (int i = 0; i < M; i++) {
        fp_array[i] = 1.0 + (i % 13) * 0.1;
    }
    
    /* Loop with floating-point division/multiplication bottleneck
     * Division has high latency and competes for FP units */
    for (int i = 1; i < M; i++) {
        /* Carried dependency chain with high-latency operations */
        fp_array[i] = fp_array[i-1] / 3.14159 * fp_array[i] + 0.5;
        
        /* Additional dependency to increase complexity */
        accumulator = accumulator * (fp_array[i] + 1.0);
        
        /* More operations to create resource contention */
        fp_array[i] = fp_array[i] * 2.71828 - 1.0;
    }
    
    sink = (int)accumulator;
    return accumulator;
}

/* Loop 3: Multiple interdependent carried dependencies */
int loop3_complex_dependencies(void) {
    int vec1[N], vec2[N], vec3[N];
    int sum = 0, prod = 1;
    
    /* Initialize vectors */
    for (int i = 0; i < N; i++) {
        vec1[i] = i * 2;
        vec2[i] = i * 3 + 1;
        vec3[i] = i * 5 + 2;
    }
    
    /* Complex loop with multiple carried dependencies
     * This should create a high II due to resource constraints */
    for (int i = 1; i < N; i++) {
        /* First carried dependency chain */
        vec1[i] = vec1[i-1] * vec2[i] + vec3[i];
        
        /* Second carried dependency chain */
        vec2[i] = vec2[i-1] + vec1[i] * 3;
        
        /* Third carried dependency chain */
        vec3[i] = vec3[i-1] * 2 - vec2[i];
        
        /* Accumulate results with multiplication (resource intensive) */
        sum = sum + vec1[i] + vec2[i] + vec3[i];
        prod = prod * (vec1[i] % 17 + 1);
    }
    
    sink = sum + prod;
    return sum * prod;
}

/* Loop 4: Mixed operations with artificial anti-dependencies */
float loop4_mixed_operations(void) {
    float a[N], b[N], c[N];
    float total = 0.0f;
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)i * 0.3f;
        c[i] = (float)i * 0.7f;
    }
    
    /* Loop with mixed float/int operations and carried dependencies */
    for (int i = 2; i < N; i++) {
        /* Multiple carried dependencies of distance 1 and 2 */
        a[i] = a[i-1] * b[i] + c[i-2];
        b[i] = b[i-1] / 2.0f + a[i];
        c[i] = c[i-1] * 1.5f - b[i-1];
        
        /* Resource-intensive computation */
        total += a[i] * b[i] * c[i];
    }
    
    sink = (int)total;
    return total;
}

int main(void) {
    int result1;
    double result2;
    int result3;
    float result4;
    
    printf("Starting modulo scheduler test loops...\n");
    
    /* Execute all loops to ensure they're not optimized away */
    result1 = loop1_carried_dependency();
    result2 = loop2_fp_bottleneck();
    result3 = loop3_complex_dependencies();
    result4 = loop4_mixed_operations();
    
    /* Use results to prevent optimization */
    printf("Results: %d, %.2f, %d, %.2f\n", 
           result1, result2, result3, result4);
    
    /* Force compiler to keep all computations */
    sink = result1 + (int)result2 + result3 + (int)result4;
    
    return 0;
}
