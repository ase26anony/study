/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of individual loops */
#define NO_OPT __attribute__((optimize("O1")))

/* Volatile variables to prevent dead code elimination */
static volatile int g_volatile_sink = 0;
static volatile int g_volatile_counter = 0;

/* External function to create side effects */
NO_OPT static void side_effect(int x) {
    g_volatile_sink += x;
}

/* Function containing the loops we want to test */
NO_OPT static void test_loops(int iterations, int* array) {
    int i, j;
    int n = iterations;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        array[i % 256] = i;  /* Non-constant array index */
        side_effect(1);
    }
    
    /* Reset counter */
    n = iterations;
    
    /* Loop variant 2: do-while with pre-decrement */
    if (n > 0) {
        do {
            array[n % 256] = n * 2;
            side_effect(2);
        } while (--n > 0);
    }
    
    /* Reset counter */
    n = iterations;
    
    /* Loop variant 3: while loop with post-decrement */
    while (n--) {
        array[(n + 1) % 256] = n * 3;
        side_effect(3);
    }
    
    /* Loop variant 4: Nested loops with inner decrementing counter */
    n = iterations / 2;
    for (i = 0; i < 5; i++) {
        int inner = n;
        while (inner > 0) {
            array[inner % 256] = i * inner;
            side_effect(4);
            inner--;
        }
    }
    
    /* Loop variant 5: Complex decrement pattern that might generate (reg + -1) */
    n = iterations;
    for (i = n; i != 0; i = i - 1) {
        array[i % 256] = i * i;
        side_effect(5);
    }
}

/* Another test function with different optimization attributes */
__attribute__((noinline, optimize("O1"))) 
static int test_more_loops(int n, int* arr) {
    int sum = 0;
    int counter = n;
    
    /* This should generate clean decrement-and-branch */
    while (counter > 0) {
        arr[counter % 128] = counter;
        sum += arr[(counter + 1) % 128];
        counter--;
    }
    
    return sum;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iteration_count>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        fprintf(stderr, "Need at least 20 iterations\n");
        return 1;
    }
    
    /* Non-constant sized array to prevent optimization */
    int* array = (int*)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        array[i] = i + 1;
    }
    
    /* Test the loops */
    test_loops(iterations, array);
    
    /* Test another function */
    int result = test_more_loops(iterations / 2, array);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d, Sink: %d\n", result, g_volatile_sink);
    
    /* Verify array was modified */
    int check = 0;
    for (int i = 0; i < 256; i++) {
        check += array[i];
    }
    printf("Array checksum: %d\n", check);
    
    free(array);
    return 0;
}
