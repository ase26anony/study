/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of individual loops */
#define NO_OPT __attribute__((optimize("O1")))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;
static volatile int sink = 0;

/* Non-inlineable function to create side effects */
static void __attribute__((noinline, noclone)) 
do_work(int *arr, int idx, int val) {
    arr[idx] = val;
    sink = arr[idx];
}

/* Function containing the loops - compiled with O1 optimization */
NO_OPT static void test_loops(int n, int *arr) {
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        do_work(arr, i % 16, i);
        global_counter++;
    }
    
    /* Loop variant 2: do-while loop with pre-decrement */
    j = n;
    if (j > 0) {
        do {
            do_work(arr, j % 16, j);
            global_counter++;
        } while (--j > 0);
    }
    
    /* Loop variant 3: while loop with post-decrement */
    j = n;
    while (j--) {
        do_work(arr, j % 16, j);
        global_counter++;
    }
    
    /* Loop variant 4: Nested loops with inner decrementing loop */
    for (i = 0; i < 3; i++) {
        int inner = n / 4;
        for (j = inner; j > 0; j--) {
            do_work(arr, (i * j) % 16, i * j);
            global_counter++;
        }
    }
    
    /* Loop variant 5: Complex decrement with arithmetic */
    for (i = n; i > 0; i -= 1) {
        do_work(arr, (i * 2) % 16, i);
        global_counter++;
    }
}

/* Alternative version with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize ("O1")
static void test_loops2(int n, int *arr) {
    int i = n;
    
    /* Another pattern: while with explicit compare */
    while (i > 0) {
        do_work(arr, i % 16, i);
        global_counter++;
        i--;
    }
    
    /* Pattern that might generate (reg + -1) compare 0 */
    i = n;
    while (i-- > 0) {
        do_work(arr, i % 16, i);
        global_counter++;
    }
}
#pragma GCC pop_options

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
    int *arr = malloc(16 * sizeof(int));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 16; i++) {
        arr[i] = i * n;
    }
    
    /* Reset global counter */
    global_counter = 0;
    
    /* Test both loop functions */
    test_loops(n, arr);
    test_loops2(n, arr);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += arr[i];
    }
    
    printf("Iterations: %d, Global counter: %d, Array sum: %d\n", 
           n, global_counter, sum);
    
    free(arr);
    return 0;
}
