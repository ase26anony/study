/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of individual loops */
#define NOOPT __attribute__((optimize("O1")))

/* Volatile variable to prevent dead code elimination */
static volatile int sink = 0;

/* Non-inlinable function to create side effects */
static void __attribute__((noinline, noclone)) 
do_work(int *arr, int idx, int val) {
    arr[idx] = val;
    sink = arr[idx];
}

/* Function containing the loops we want to test */
NOOPT static void test_loops(int n, int *arr) {
    int i, j;
    
    /* Variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        do_work(arr, i % 100, i);
    }
    
    /* Reset counter */
    int m = n;
    
    /* Variant 2: do-while loop with pre-decrement */
    if (m > 0) {
        do {
            do_work(arr, m % 100, m);
        } while (--m > 0);
    }
    
    /* Reset counter */
    int k = n;
    
    /* Variant 3: while loop with post-decrement */
    while (k--) {
        do_work(arr, k % 100, k);
    }
    
    /* Variant 4: Nested loops with inner decrementing counter */
    for (i = 0; i < 5; i++) {
        int inner = n / 5;
        for (j = inner; j > 0; j--) {
            do_work(arr, (i * j) % 100, i * j);
        }
    }
    
    /* Variant 5: Complex decrement pattern that might generate (reg + -1) */
    int p = n;
    while (p > 0) {
        do_work(arr, p % 100, p);
        p = p - 1;  /* Explicit decrement instead of p-- */
    }
}

/* Main function with command line argument */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 10) {
        fprintf(stderr, "Please provide n > 10\n");
        return 1;
    }
    
    /* Non-constant array to prevent optimization */
    int *arr = malloc(100 * sizeof(int));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    /* Call the function with loops */
    test_loops(n, arr);
    
    /* Compute and print result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    printf("Result: %d (sink=%d)\n", sum, sink);
    
    free(arr);
    return 0;
}
