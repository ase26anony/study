/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical loops */
__attribute__((noinline, optimize("O1")))
void test_loops(int iterations, int* results) {
    volatile int sink = 0;  /* Prevent dead code elimination */
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        sink += i;
        results[0] += i;
    }
    
    /* Loop variant 2: Do-while with decrement */
    j = iterations;
    do {
        sink -= j;
        results[1] += j;
    } while (--j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    int k = iterations;
    while (k--) {
        sink *= 2;
        results[2] += k;
    }
    
    /* Loop variant 4: Nested loops with inner decrement */
    int outer = iterations / 10;
    for (i = 0; i < outer; i++) {
        int inner = 10;
        for (j = inner; j > 0; j--) {
            sink ^= (i * j);
            results[3] += (i * j);
        }
    }
    
    /* Force sink to be used */
    results[4] = sink;
}

/* Alternative version with different optimization pragma */
#pragma GCC push_options
#pragma GCC optimize("O1")
__attribute__((noinline))
void test_loops_alt(int iterations, int* results) {
    volatile int counter = iterations;
    int sum = 0;
    
    /* Another decrementing loop pattern */
    while (counter > 0) {
        sum += counter;
        results[5] = sum;
        counter--;
    }
    
    /* Loop with explicit decrement and compare */
    for (int i = iterations; i != 0; i = i - 1) {
        results[6] ^= i;
    }
}
#pragma GCC pop_options

/* Main function with command-line argument */
int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 100) {
        fprintf(stderr, "Iterations should be >= 100 for meaningful test\n");
        iterations = 100;
    }
    
    /* Use non-constant bounds from command line */
    int results[10] = {0};
    
    /* Test both loop functions */
    test_loops(iterations, results);
    test_loops_alt(iterations, results);
    
    /* Print results to prevent optimization */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += results[i];
    }
    printf("Result checksum: %d\n", total);
    
    return 0;
}
