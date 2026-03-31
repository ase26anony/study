/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical loops */
#define NOOPT __attribute__((optimize("O1")))

/* Volatile variables to prevent dead code elimination */
volatile int global_sum = 0;
volatile int global_counter = 0;

/* Non-inlinable function to create side effects */
static void __attribute__((noinline, noclone)) 
do_work(int *arr, int idx, int val) {
    arr[idx] = val;
    global_sum += val;
    global_counter++;
}

/* Function containing the loops we want to test */
NOOPT void test_loops(int iterations, int *array) {
    int i, j;
    
    /* Loop 1: Basic for loop with decrementing counter */
    for (i = iterations; i > 0; i--) {
        do_work(array, i % 100, i);
    }
    
    /* Reset counter for next loop */
    int n = iterations;
    
    /* Loop 2: do-while loop with pre-decrement */
    do {
        do_work(array, n % 100, n);
    } while (--n > 0);
    
    /* Reset counter */
    n = iterations;
    
    /* Loop 3: while loop with post-decrement */
    while (n--) {
        do_work(array, n % 100, n);
    }
    
    /* Loop 4: Nested loops with inner decrementing counter */
    for (i = 0; i < 10; i++) {
        int inner = iterations / 10;
        for (j = inner; j > 0; j--) {
            do_work(array, (i * j) % 100, i + j);
        }
    }
    
    /* Loop 5: Complex decrement pattern with if condition */
    n = iterations;
    while (1) {
        do_work(array, n % 100, n);
        if (--n <= 0) break;
    }
}

/* Alternative version with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize ("O1")
void test_loops2(int iterations, int *array) {
    int i = iterations;
    
    /* Loop with explicit comparison to zero */
    while (i > 0) {
        array[i % 100] = i * 2;
        global_sum += array[i % 100];
        i--;
    }
    
    /* Another variant with different decrement style */
    for (i = iterations; i != 0; i -= 1) {
        array[(i * 3) % 100] = i;
        global_counter += array[(i * 3) % 100];
    }
}
#pragma GCC pop_options

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 10) {
        printf("Iterations must be >= 10\n");
        return 1;
    }
    
    /* Non-constant array to prevent optimization */
    int *array = (int*)malloc(100 * sizeof(int));
    if (!array) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 100; i++) {
        array[i] = (i * 3) % 7;
    }
    
    /* Call the test functions */
    test_loops(iterations, array);
    test_loops2(iterations, array);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    printf("Results: array_sum=%d, global_sum=%d, global_counter=%d\n", 
           sum, global_sum, global_counter);
    
    free(array);
    return 0;
}
