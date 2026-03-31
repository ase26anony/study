/* modulo-sched-test.c
 * Designed to trigger uncovered lines in GCC's modulo scheduler
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -dP modulo-sched-test.c -lm -o modulo-sched-test
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and force memory accesses */
static volatile int offset = 1;

/* Function to generate pseudo-random data */
static double simple_rand(int seed) {
    return ((seed * 1103515245 + 12345) & 0x7fffffff) / 2147483648.0;
}

int main(int argc, char *argv[]) {
    const int N = 1024;
    double array_a[N], array_b[N], array_c[N];
    double sum = 0.0;
    int i;
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < N; i++) {
        array_a[i] = simple_rand(i);
        array_b[i] = simple_rand(i + N);
        array_c[i] = 0.0;
    }
    
    /* Use command-line argument to create variability in offset */
    int dynamic_offset = (argc > 1) ? (atoi(argv[1]) % 4) : 1;
    if (dynamic_offset == 0) dynamic_offset = 1;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required patterns:
     * 1. Loop-carried dependency (reduction)
     * 2. High-latency operations (sqrt, division)
     * 3. Multiple memory accesses with potential aliasing
     * 4. Conditional store with cross-iteration dependency
     */
    for (i = 0; i < N - dynamic_offset; i++) {
        /* High-latency operation 1: square root with division */
        double x = sqrt(array_a[i] + 1.0) / 1.234567;
        
        /* High-latency operation 2: floating-point division with variable divisor */
        double y = array_b[i + dynamic_offset] / (x + 0.001);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + y;  /* This creates a true recurrence */
        
        /* Access multiple arrays with non-constant offsets */
        double temp = array_a[i] + array_b[i + dynamic_offset];
        
        /* Conditional store with cross-iteration dependency */
        if (i > 0) {
            /* Dependency on previous iteration's value */
            if (array_c[i-1] > 0.5) {  /* Distance-1 memory dependency */
                array_c[i] = x * 0.5;
            } else {
                array_c[i] = y * 0.3;
            }
        } else {
            array_c[i] = x;
        }
        
        /* Additional high-latency operation to increase register pressure */
        array_b[i] = sin(temp) * 0.01;  /* sin() is high latency */
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Array checksum: %f\n", array_c[N-2] + array_c[N-3]);
    
    return 0;
}
