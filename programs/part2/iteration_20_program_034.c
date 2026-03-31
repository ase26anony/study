/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical function */
__attribute__((noinline, optimize("O1")))
void test_loops(int n, int* arr) {
    volatile int sink = 0;
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        arr[i % 100] = i * 2;
        sink = arr[(i + 1) % 100];  /* Volatile access prevents elimination */
    }
    
    /* Loop variant 2: Do-while with pre-decrement */
    j = n;
    do {
        arr[j % 100] = j * 3;
        sink = arr[(j + 2) % 100];
    } while (--j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    j = n;
    while (j--) {
        arr[j % 100] = j * 4;
        sink = arr[(j + 3) % 100];
    }
    
    /* Loop variant 4: Nested loops with inner decrementing counter */
    for (i = 0; i < 5; i++) {
        int inner = n / 10;
        while (inner-- > 0) {
            arr[(i + inner) % 100] = i * inner;
            sink = arr[(i + inner + 1) % 100];
        }
    }
    
    /* Ensure sink is used to prevent dead code elimination */
    arr[0] = sink;
}

/* Another function with different optimization level to test more patterns */
__attribute__((noinline))
void test_more_loops(int n, int* arr) {
    #pragma GCC optimize("O1")
    volatile int sink = 0;
    
    /* Loop with explicit decrement operation */
    for (int k = n; k != 0; k = k - 1) {
        arr[k % 100] = k * 5;
        sink = arr[(k + 4) % 100];
    }
    
    /* Loop with compound decrement */
    int m = n;
    while (m) {
        arr[m % 100] = m * 6;
        sink = arr[(m + 5) % 100];
        m -= 1;
    }
    
    arr[1] = sink;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iteration_count>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 10) {
        fprintf(stderr, "Iteration count must be > 10\n");
        return 1;
    }
    
    /* Use non-constant array to prevent optimization */
    int* arr = (int*)malloc(100 * sizeof(int));
    if (!arr) return 1;
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* Test both functions */
    test_loops(n, arr);
    test_more_loops(n, arr);
    
    /* Compute and print result to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    printf("Result: %d\n", sum);
    
    free(arr);
    return 0;
}
