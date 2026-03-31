/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical loops */
#define NOOPT __attribute__((optimize("O1")))

/* Volatile variable to prevent dead code elimination */
volatile int sink = 0;

/* Non-inlinable function to create side effects */
void __attribute__((noinline, noclone)) do_work(int *counter) {
    *counter += 1;
    sink = *counter;  /* Volatile store */
}

/* Function containing the loops we want to test */
NOOPT void test_loops(int iterations, int *results) {
    int i, j, k, m;
    int local_sink = 0;
    
    /* Loop 1: Basic for loop with decrementing counter */
    for (i = iterations; i > 0; i--) {
        do_work(&local_sink);
        results[0] += local_sink;
    }
    
    /* Loop 2: do-while loop with decrement */
    j = iterations;
    do {
        do_work(&local_sink);
        results[1] += local_sink;
    } while (--j > 0);
    
    /* Loop 3: while loop with post-decrement */
    k = iterations;
    while (k--) {
        do_work(&local_sink);
        results[2] += local_sink;
    }
    
    /* Loop 4: Nested loops - inner loop uses decrementing counter */
    m = iterations / 2;
    for (i = 0; i < 3; i++) {
        for (j = m; j > 0; j--) {
            do_work(&local_sink);
            results[3] += local_sink;
        }
    }
    
    /* Loop 5: Complex decrement pattern with if condition */
    int n = iterations;
    while (n > 0) {
        do_work(&local_sink);
        results[4] += local_sink;
        n--;
    }
    
    /* Final volatile store to ensure all loops execute */
    sink = results[0] + results[1] + results[2] + results[3] + results[4];
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        fprintf(stderr, "Iterations must be >= 20\n");
        return 1;
    }
    
    /* Array to store results - prevents dead code elimination */
    int results[5] = {0};
    
    /* Call the test function multiple times to increase coverage chance */
    test_loops(iterations, results);
    
    /* Use results to prevent optimization */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += results[i];
    }
    
    printf("Total work done: %d\n", total);
    printf("Sink value: %d\n", sink);
    
    return 0;
}
