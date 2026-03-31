/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of loops */
#define NO_OPT __attribute__((optimize("O1")))

/* Volatile variables to prevent dead code elimination */
volatile int global_sum = 0;
volatile int sink = 0;

/* Non-inlinable function to create side effects */
static void __attribute__((noinline, noclone)) 
do_work(int *arr, int idx, int val) {
    arr[idx] = val;
    sink = arr[idx];
}

/* Function containing the loops we want to test */
NO_OPT static void test_loops(int n, int *arr) {
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        do_work(arr, i - 1, i);
        global_sum += arr[i - 1];
    }
    
    /* Reset counter */
    int m = n;
    
    /* Loop variant 2: do-while with pre-decrement */
    if (m > 0) {
        do {
            do_work(arr, m - 1, m * 2);
            global_sum -= arr[m - 1];
        } while (--m > 0);
    }
    
    /* Reset counter */
    int k = n;
    
    /* Loop variant 3: while loop with post-decrement */
    while (k--) {
        do_work(arr, k, k * 3);
        global_sum ^= arr[k];
    }
    
    /* Loop variant 4: Nested loops with inner decrementing loop */
    int outer = n / 10;
    if (outer < 1) outer = 1;
    
    for (i = 0; i < outer; i++) {
        int inner = n;
        while (inner > 0) {
            do_work(arr, inner - 1, i * inner);
            global_sum += i * arr[inner - 1];
            inner--;
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 10) {
        fprintf(stderr, "Need n > 10, got %d\n", n);
        return 1;
    }
    
    /* Use heap allocation to avoid constant propagation */
    int *arr = (int*)malloc(n * sizeof(int));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < n; i++) {
        arr[i] = (i * 7) % 13;
    }
    
    /* Call the function with loops */
    test_loops(n, arr);
    
    /* Print results to prevent optimization */
    printf("Result: global_sum = %d, sink = %d, arr[0] = %d\n", 
           global_sum, sink, arr[0]);
    
    free(arr);
    return 0;
}
