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

/* Prevent optimization of critical variables */
static volatile int sink;

/* Force arrays to be stored in memory */
double __attribute__((used)) arr1[N];
double __attribute__((used)) arr2[N];
int __attribute__((used)) int_arr[M];

int main(void) {
    double result1 = 0.0, result2 = 0.0;
    int int_result = 0;
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < N; i++) {
        arr1[i] = sin(i * 0.1) + 1.0;
        arr2[i] = cos(i * 0.05) * 2.0;
    }
    
    for (int i = 0; i < M; i++) {
        int_arr[i] = (i * 3) % 17;
    }
    
    /* 
     * LOOP 1: Simple carried dependency with integer operations
     * Creates distance-1 dependency: int_result depends on previous iteration
     * Mix of operations to create resource pressure
     */
    int accumulator = 1;
    for (int i = 1; i < M; i++) {
        /* Distance-1 dependency: uses value from previous iteration */
        accumulator = accumulator * 3 + int_arr[i];
        
        /* Additional operation with moderate latency */
        int_arr[i] = (accumulator % 7) * (int_arr[i-1] + 1);
        
        /* Another dependency chain */
        int_result += accumulator * 2 - int_arr[i];
    }
    
    /*
     * LOOP 2: Complex floating-point carried dependencies
     * Multiple interdependent recurrence patterns
     * High-latency operations (division) create resource contention
     */
    double fp_accum = arr1[0];
    for (int i = 1; i < N; i++) {
        /* Primary distance-1 dependency with high-latency division */
        fp_accum = fp_accum / 3.14159 + arr1[i] * 2.0;
        
        /* Secondary dependency chain using the same accumulator */
        double temp = fp_accum * arr2[i];
        
        /* Another recurrence with distance-1 */
        arr2[i] = arr2[i-1] * 0.9 + temp;
        
        /* Complex expression with multiple operations */
        result1 += fp_accum * sin(temp) / (arr2[i] + 1.0);
    }
    
    /*
     * LOOP 3: Nested dependencies and multiple recurrence patterns
     * Designed to increase II calculation complexity
     */
    double x = 1.0, y = 2.0, z = 0.5;
    for (int i = 1; i < N/2; i++) {
        /* Three interdependent recurrence chains */
        x = x * 1.1 + arr1[i] / (y + 0.001);
        y = y * 0.95 + arr2[i] * (z + 0.5);
        z = z * 0.99 + x * y * 0.1;
        
        /* Complex expression using all three */
        result2 += (x + y) * z / (arr1[i] + arr2[i] + 1.0);
        
        /* Additional operation with array dependency */
        arr1[i] = arr1[i-1] * 0.8 + z;
    }
    
    /* Combine results to create observable side effect */
    double final_result = result1 + result2 + int_result;
    
    /* Use volatile sink to prevent dead code elimination */
    sink = (int)final_result;
    
    /* Print result to ensure loops aren't optimized away */
    printf("Result: %f\n", final_result);
    
    return 0;
}
