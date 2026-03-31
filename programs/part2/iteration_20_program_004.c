/* test_loop_doloop.c - Test program for GCC loop-doloop.cc pattern matching */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization from removing loops */
static volatile int sink = 0;
static int global_sum = 0;

/* External function to prevent inlining */
void __attribute__((noinline, noclone)) 
external_side_effect(int *arr, int idx, int val) {
    arr[idx] = val;
    sink = arr[idx]; /* Volatile access ensures side effect */
}

/* Test function with multiple decrementing loop variants */
__attribute__((noinline, optimize("O1")))
void test_loops(int iterations, int *arr) {
    int i, j;
    int n = iterations;
    
    /* Variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        external_side_effect(arr, i % 100, i);
        global_sum += arr[i % 100];
    }
    
    /* Reset counter */
    n = iterations;
    
    /* Variant 2: do-while loop with pre-decrement */
    do {
        external_side_effect(arr, n % 100, n);
        global_sum += arr[n % 100];
    } while (--n > 0);
    
    /* Reset counter */
    n = iterations;
    
    /* Variant 3: while loop with post-decrement */
    while (n--) {
        external_side_effect(arr, n % 100, n);
        global_sum += arr[n % 100];
    }
    
    /* Variant 4: Nested loops with inner decrementing counter */
    n = iterations / 2;
    for (i = 0; i < 5; i++) {
        int inner = n;
        while (inner > 0) {
            external_side_effect(arr, (i * inner) % 100, inner);
            global_sum += arr[(i * inner) % 100];
            inner--;
        }
    }
    
    /* Variant 5: Complex decrement with if condition */
    n = iterations;
    for (j = n; j > 0; j--) {
        if (j % 3 == 0) {
            external_side_effect(arr, j % 100, j * 2);
            global_sum += arr[j % 100];
        } else {
            external_side_effect(arr, j % 100, j);
            global_sum -= arr[j % 100];
        }
    }
}

/* Alternative test with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize("O1")
__attribute__((noinline, noclone))
void test_loops_alt(int iterations, int *arr) {
    int n = iterations;
    int k = 0;
    
    /* Another variant: while with compound condition */
    while (n-- > 0 && k < 1000) {
        arr[k % 100] = n;
        sink = arr[k % 100];
        global_sum += n;
        k++;
    }
}
#pragma GCC pop_options

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        fprintf(stderr, "Iterations must be >= 20\n");
        return 1;
    }
    
    /* Non-constant array to prevent optimization */
    int *array = (int*)malloc(100 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 100; i++) {
        array[i] = (i * 3) % 7;
    }
    
    /* Call test functions */
    test_loops(iterations, array);
    test_loops_alt(iterations, array);
    
    /* Use results to prevent dead code elimination */
    printf("Result: global_sum = %d, sink = %d, array[0] = %d\n", 
           global_sum, sink, array[0]);
    
    free(array);
    return 0;
}
