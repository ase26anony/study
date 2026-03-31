/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop_doloop
 * For RISC-V: gcc -O1 -march=rv64gc -fdump-rtl-expand test_loop_doloop.c -o test_loop_doloop
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of loops */
#define NOINLINE __attribute__((noinline, noclone))

/* Use volatile to prevent dead code elimination */
static volatile int sink = 0;

/* Array to work on - prevents loop elimination */
static int array[1024];

/* Function with specific optimization level to generate clean RTL */
NOINLINE __attribute__((optimize("O1")))
void test_loops(int n, int *arr) {
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        /* Non-trivial operation that can't be optimized away */
        arr[i % 1024] = i;
        sink = arr[i % 1024];  /* Volatile store ensures side effect */
    }
    
    /* Loop variant 2: do-while loop with decrement */
    j = n;
    do {
        arr[j % 1024] = j * 2;
        sink = arr[j % 1024];
    } while (--j > 0);
    
    /* Loop variant 3: while loop with post-decrement */
    int k = n;
    while (k--) {
        arr[k % 1024] = k * 3;
        sink = arr[k % 1024];
    }
    
    /* Loop variant 4: Nested loops with decrementing inner loop */
    for (i = 0; i < 5; i++) {
        int inner = n / 2;
        while (inner-- > 0) {
            arr[(i * inner) % 1024] = i + inner;
            sink = arr[(i * inner) % 1024];
        }
    }
    
    /* Loop variant 5: Another for loop with different structure */
    for (int m = n; m != 0; m = m - 1) {
        arr[m % 1024] = m * m;
        sink = arr[m % 1024];
    }
}

/* Alternative function with different signature to avoid inlining */
NOINLINE __attribute__((optimize("O1")))
int process_data(int iterations) {
    int local_array[256];
    int sum = 0;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 3 + 1;
    }
    
    /* Decrementing loop that should generate (reg + -1) COMPARE 0 */
    for (int i = iterations; i > 0; i--) {
        /* Complex enough to not be optimized away */
        sum += local_array[i % 256];
        local_array[i % 256] ^= sum;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 10) {
        fprintf(stderr, "Iterations must be > 10\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 1024; i++) {
        array[i] = i * 7 + 3;
    }
    
    /* Call test function with loops */
    test_loops(n, array);
    
    /* Call another function with similar pattern */
    int result = process_data(n / 2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d, Sink: %d\n", result, sink);
    
    return 0;
}
