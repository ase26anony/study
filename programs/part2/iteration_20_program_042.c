/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical function */
__attribute__((noinline, optimize("O1")))
void test_loops(int n, int* results) {
    volatile int sink = 0;  /* Prevent dead code elimination */
    int i, j;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        sink += i;  /* Side effect to prevent removal */
        results[0] += i;
    }
    
    /* Loop variant 2: Do-while with decrement */
    j = n;
    do {
        sink -= j;  /* Different side effect */
        results[1] += j;
    } while (--j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    j = n;
    while (j--) {
        sink ^= j;  /* Another side effect */
        results[2] += j;
    }
    
    /* Loop variant 4: Nested loops with inner decrement */
    for (i = 0; i < 3; i++) {
        j = n / 3;
        while (j-- > 0) {
            sink |= j;  /* Bitwise operation prevents optimization */
            results[3] += j;
        }
    }
    
    /* Force use of sink to prevent elimination */
    results[4] = sink;
}

/* Another function with different optimization level to test pattern */
__attribute__((noinline))
void test_more_loops(int n, int* arr, int size) {
    #pragma GCC optimize("O1")
    int i, j;
    
    /* Loop with array access - harder to optimize away */
    for (i = n; i > 0; i--) {
        arr[i % size] = i;
    }
    
    /* Complex decrement pattern */
    j = n;
    while (j) {
        arr[(j * 7) % size] += j;
        j--;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 10) {
        printf("Please provide n > 10\n");
        return 1;
    }
    
    int results[5] = {0};
    int* array = (int*)malloc(n * sizeof(int));
    
    if (!array) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < n; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Test both functions */
    test_loops(n, results);
    test_more_loops(n, array, n);
    
    /* Print results to prevent dead code elimination */
    printf("Results: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    /* Use array to prevent elimination */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    printf("Array sum: %d\n", sum);
    
    free(array);
    return 0;
}
