/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that might eliminate loops */
volatile int sink;

/* Non-inlinable function to create side effects */
static void __attribute__((noinline, noclone)) 
do_work(int *arr, int idx, int val) {
    arr[idx] = val;
    sink = arr[idx];
}

/* Function containing the loops we want to test */
static void __attribute__((noinline, optimize("O1")))
test_loops(int iterations, int *arr) {
    int i, j;
    int n = iterations;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        do_work(arr, i - 1, i);
    }
    
    /* Reset counter */
    n = iterations;
    
    /* Loop variant 2: do-while with pre-decrement */
    if (n > 0) {
        do {
            do_work(arr, n - 1, n);
        } while (--n > 0);
    }
    
    /* Reset counter */
    n = iterations;
    
    /* Loop variant 3: while loop with post-decrement */
    while (n--) {
        do_work(arr, n, n + 1);
    }
    
    /* Loop variant 4: Nested loops with inner decrementing counter */
    n = iterations / 2;
    for (i = 0; i < 5; i++) {
        int inner = n;
        while (inner > 0) {
            do_work(arr, inner + i, inner);
            inner--;
        }
    }
    
    /* Loop variant 5: Complex decrement pattern */
    n = iterations;
    for (i = n; i != 0; i -= 1) {
        do_work(arr, i - 1, i * 2);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 10) {
        fprintf(stderr, "Iterations must be >= 10\n");
        return 1;
    }
    
    /* Use dynamic allocation to prevent constant propagation */
    int *array = (int*)malloc(iterations * 2 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < iterations * 2; i++) {
        array[i] = i * 3;
    }
    
    test_loops(iterations, array);
    
    /* Compute and print a result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += array[i];
    }
    
    printf("Result: %d\n", sum);
    
    free(array);
    return 0;
}
