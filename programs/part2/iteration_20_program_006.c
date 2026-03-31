/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical loops */
#define NOOPT __attribute__((optimize("O1")))

/* Volatile variable to prevent dead code elimination */
static volatile int sink = 0;

/* External array to prevent constant propagation */
extern int external_array[];

/* Function to isolate loop RTL generation */
NOOPT void test_loops(int iterations, int* arr) {
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation with side effect */
        arr[i % 256] = i;
        sink += arr[(i + 1) % 256];
    }
    
    /* Loop variant 2: Do-while with pre-decrement */
    int n = iterations;
    do {
        arr[n % 256] = n * 2;
        sink += arr[(n + 2) % 256];
    } while (--n > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    int m = iterations;
    while (m--) {
        arr[m % 256] = m * 3;
        sink += arr[(m + 3) % 256];
    }
    
    /* Loop variant 4: Nested loops with decrementing inner counter */
    for (i = 10; i > 0; i--) {
        for (j = iterations % 100; j > 0; j--) {
            arr[(i + j) % 256] = i * j;
            sink += arr[(i * j) % 256];
        }
    }
    
    /* Loop variant 5: Complex decrement pattern */
    int k = iterations;
    while (k > 0) {
        arr[k % 256] = sink;
        sink = arr[(k - 1) % 256];
        k--;
    }
}

/* Alternative function with different optimization attributes */
__attribute__((noinline, optimize("O1"))) 
void test_loops_alt(int iterations, int* arr) {
    /* Another set of loops to increase pattern matching chances */
    int x = iterations;
    
    /* Loop with compound condition */
    for (int y = x; y > 0; y -= 1) {
        arr[y % 128] ^= 0x5A;
        sink += arr[(y + 5) % 128];
    }
    
    /* Loop with manual decrement */
    int z = x;
    while (1) {
        arr[z % 128] = z;
        sink++;
        z--;
        if (z <= 0) break;
    }
}

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
    
    /* Dynamic allocation prevents compile-time optimization */
    int* array = (int*)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3 + iterations;
    }
    
    /* Call test functions multiple times */
    test_loops(iterations, array);
    test_loops_alt(iterations / 2, array);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (array[0]=%d, array[255]=%d)\n", 
           sink, array[0], array[255]);
    
    free(array);
    return 0;
}
