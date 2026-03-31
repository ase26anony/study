/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Force variables to be used and prevent optimization */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
void loop_with_carried_deps(void) {
    volatile double array1[N], array2[N];
    volatile double result[N];
    double acc1 = 1.0, acc2 = 2.0;
    int i;
    
    /* Initialize arrays with non-trivial values */
    for (i = 0; i < N; i++) {
        array1[i] = (i % 7) * 1.5;
        array2[i] = (i % 5) * 2.3;
    }
    
    /* 
     * Loop with multiple carried dependencies:
     * 1. acc1 depends on its previous iteration value (distance-1)
     * 2. acc2 depends on acc1 from same iteration AND its own previous value
     * 3. High-latency floating-point operations
     */
    for (i = 0; i < N; i++) {
        /* First carried dependency chain with FP division (high latency) */
        acc1 = acc1 / 3.14159 + array1[i];
        
        /* Second carried dependency chain using result from first chain */
        acc2 = acc2 * 1.618 + acc1 * array2[i];
        
        /* Store result to prevent elimination */
        result[i] = acc1 + acc2;
    }
    
    /* Use result to prevent dead code elimination */
    sink = (int)result[N-1];
}

/* Loop with integer carried dependencies and resource contention */
void loop_with_integer_deps(void) {
    volatile int data[M];
    int sum = 0, prod = 1;
    int i;
    
    /* Initialize with pattern */
    for (i = 0; i < M; i++) {
        data[i] = (i * 3) % 17;
    }
    
    /*
     * Complex integer loop with:
     * 1. Distance-1 dependency on sum (sum uses previous sum)
     * 2. Distance-1 dependency on prod (prod uses previous prod)
     * 3. Interdependency between sum and prod
     * 4. Integer multiplication (may compete for multiplier units)
     */
    for (i = 0; i < M; i++) {
        /* Carried dependency on sum */
        sum = sum + data[i];
        
        /* Carried dependency on prod, using current sum */
        prod = prod * (sum % 31 + 1);
        
        /* Additional operation with carried dependency */
        data[i] = (data[i] + prod) % 1024;
    }
    
    sink = sum + prod;
}

/* Loop with array-based carried dependencies */
void loop_with_array_deps(void) {
    volatile double arr[N];
    int i;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        arr[i] = i * 0.5;
    }
    
    /*
     * Classic recurrence pattern with distance-1 array dependency
     * arr[i] depends on arr[i-1] from previous iteration
     * Mixed with high-latency operations
     */
    for (i = 1; i < N; i++) {
        /* Distance-1 dependency through array */
        double temp = arr[i-1] * 2.71828;
        
        /* High-latency operation */
        temp = temp / 1.41421;
        
        /* Store with dependency chain */
        arr[i] = arr[i] + temp * (i % 3);
    }
    
    sink = (int)arr[N-1];
}

/* Main function to ensure all loops are executed */
int main(void) {
    double final_result = 0.0;
    
    printf("Starting modulo scheduler test loops...\n");
    
    /* Execute all loops to generate various dependency patterns */
    loop_with_carried_deps();
    loop_with_integer_deps();
    loop_with_array_deps();
    
    /* Compute something with the sink values to prevent elimination */
    final_result = sink * 1.0;
    
    printf("Final marker value: %f\n", final_result);
    printf("Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c\n");
    
    return 0;
}
