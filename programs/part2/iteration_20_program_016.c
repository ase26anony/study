/* test_loop_doloop.c - Test program for GCC loop-doloop.cc pattern matching */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of the test function */
__attribute__((noinline, optimize("O1")))
void test_loops(int iterations, int* results) {
    volatile int sink = 0;  /* Volatile to prevent dead code elimination */
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrementing counter */
    for (i = iterations; i > 0; i--) {
        sink += i;  /* Non-trivial operation with side effect */
        results[0] += sink;
    }
    
    /* Loop variant 2: Do-while loop with decrement */
    j = iterations;
    do {
        sink -= j;
        results[1] += sink;
    } while (--j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    int k = iterations;
    while (k--) {
        sink ^= k;  /* Different operation to avoid identical patterns */
        results[2] += sink;
    }
    
    /* Loop variant 4: Nested loops with inner decrementing counter */
    int m = iterations / 2;
    for (i = 0; i < 3; i++) {  /* Outer loop */
        int n = m;
        while (n-- > 0) {      /* Inner decrementing loop */
            sink = sink * 2 + 1;
            results[3] += sink;
        }
    }
    
    /* Store final sink value to ensure all loops execute */
    results[4] = sink;
}

/* Main function with command-line argument */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 10) {
        fprintf(stderr, "Iterations must be > 10\n");
        return 1;
    }
    
    /* Array to collect results - prevents loop elimination */
    int results[5] = {0};
    
    /* Call the test function with loops */
    test_loops(iterations, results);
    
    /* Print results to prevent dead code elimination */
    printf("Results: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return 0;
}
