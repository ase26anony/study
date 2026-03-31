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
void __attribute__((noinline, noclone)) side_effect(int x) {
    global_sink += x;
}

/* Function containing the loops we want to test */
NOOPT void test_loops(int iterations, int* array) {
    int i, j;
    int n = iterations;
    
    /* Loop 1: Basic for loop with decrementing counter */
    for (i = n; i > 0; i--) {
        /* Non-trivial operation with side effect */
        array[i % 256] = i;
        side_effect(array[i % 256]);
    }
    
    /* Reset counter */
    n = iterations;
    
    /* Loop 2: do-while loop with decrement */
    do {
        array[n % 256] = n * 2;
        side_effect(array[n % 256]);
    } while (--n > 0);
    
    /* Reset counter */
    n = iterations;
    
    /* Loop 3: while loop with post-decrement */
    while (n--) {
        array[n % 256] = n * 3;
        side_effect(array[n % 256]);
    }
    
    /* Loop 4: Nested loops with inner decrementing counter */
    for (i = 10; i > 0; i--) {
        int inner = iterations / 10;
        for (j = inner; j > 0; j--) {
            array[(i + j) % 256] = i * j;
            side_effect(array[(i + j) % 256]);
        }
    }
    
    /* Loop 5: Complex decrement pattern that might generate (reg + -1) compare 0 */
    n = iterations;
    while (n > 0) {
        array[n % 256] = n;
        side_effect(array[n % 256]);
        n = n - 1;  /* Explicit decrement, not n-- */
    }
}

/* Another test function with different optimization attributes */
__attribute__((noinline, optimize("O1"))) 
int test_more_loops(int limit, int* buf) {
    int sum = 0;
    int k = limit;
    
    /* Loop with counter in register */
    register int counter asm ("r12") = k;
    while (counter > 0) {
        buf[counter % 128] = counter;
        sum += buf[counter % 128];
        counter--;
    }
    
    return sum;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        fprintf(stderr, "Iterations should be >= 20 for meaningful test\n");
        iterations = 100;
    }
    
    /* Non-constant array to prevent optimization */
    int* array = (int*)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3 + iterations;
    }
    
    /* Call the test function */
    test_loops(iterations, array);
    
    /* Call second test function */
    int result = test_more_loops(iterations / 2, array);
    
    /* Use results to prevent elimination */
    printf("Result: %d, Global sink: %d, Array[100]=%d\n", 
           result, global_sink, array[100]);
    
    free(array);
    return 0;
}
