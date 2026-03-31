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
volatile int global_sink = 0;

/* External function to create side effects */
NOOPT void __attribute__((noinline, noclone)) 
do_work(int *arr, int idx, int val) {
    arr[idx] = val;
    global_sink += val;
}

/* Main test function with various decrementing loops */
NOOPT void __attribute__((noinline))
test_loops(int n, int *arr) {
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        do_work(arr, i - 1, i);
    }
    
    int m = n;
    
    /* Loop variant 2: Do-while with pre-decrement */
    if (m > 0) {
        do {
            do_work(arr, m - 1, m);
        } while (--m > 0);
    }
    
    int k = n;
    
    /* Loop variant 3: While loop with post-decrement */
    while (k--) {
        do_work(arr, k, k + 1);
    }
    
    /* Loop variant 4: Nested loops with decrementing inner loop */
    for (i = 0; i < 5; i++) {
        int inner = n / 2;
        for (j = inner; j > 0; j--) {
            do_work(arr, i * 10 + j - 1, i + j);
        }
    }
    
    /* Loop variant 5: Complex decrement with arithmetic */
    int p = n * 2;
    for (; p > 0; p -= 1) {
        if (p % 3 == 0) {
            do_work(arr, p % 20, p);
        }
    }
}

/* Alternative test with different optimization boundary */
#pragma GCC push_options
#pragma GCC optimize("O1")
static void test_loops_alt(int n, int *arr) {
    int i = n;
    
    /* Another pattern: while with compound condition */
    while (i > 0) {
        arr[i] = i * 2;
        global_sink += arr[i];
        i--;
    }
    
    /* Do-while with explicit comparison */
    int j = n;
    if (j > 0) {
        do {
            arr[j] = j * 3;
            global_sink -= arr[j];
            j = j - 1;
        } while (j != 0);
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
    
    /* Use non-constant size array */
    int *arr = (int *)malloc(n * 5 * sizeof(int));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    memset(arr, 0xAA, n * 5 * sizeof(int));
    
    /* Call test functions */
    test_loops(n, arr);
    test_loops_alt(n, arr);
    
    /* Compute checksum to prevent optimization */
    int sum = 0;
    for (int i = 0; i < n * 5 && i < 1000; i++) {
        sum += arr[i];
    }
    
    printf("Result: %d (global_sink=%d)\n", sum, global_sink);
    
    free(arr);
    return 0;
}
