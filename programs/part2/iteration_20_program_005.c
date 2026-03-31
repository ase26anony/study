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
        sink += i;
        results[0] += i;
    }
    
    /* Loop variant 2: Do-while with pre-decrement */
    j = n;
    do {
        sink -= j;
        results[1] -= j;
    } while (--j > 0);
    
    /* Loop variant 3: While loop with post-decrement */
    j = n;
    while (j--) {
        sink *= 2;
        results[2] += sink;
    }
    
    /* Loop variant 4: Nested loops with inner decrement */
    for (i = 0; i < 5; i++) {
        int k = n / 2;
        while (k > 0) {
            sink ^= k;
            results[3] ^= k;
            k--;
        }
    }
    
    /* Force use of sink */
    results[4] = sink;
}

/* Another function with different optimization to increase chances */
__attribute__((noinline, optimize("O1")))
void more_loops(int n, int* arr) {
    volatile int acc = 0;
    
    /* Loop with compound condition */
    int i = n;
    while (i) {
        arr[i % 10] += i;
        acc += arr[i % 10];
        i--;
    }
    
    /* Loop with if inside */
    for (i = n; i > 0; i -= 1) {
        if (i & 1) {
            acc ^= i;
        } else {
            acc |= i;
        }
        arr[5] = acc;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 10) {
        printf("Please use n > 10\n");
        return 1;
    }
    
    /* Use volatile to prevent constant propagation */
    volatile int iterations = n;
    
    int results[10] = {0};
    int arr[10] = {0};
    
    /* Call test functions multiple times */
    test_loops(iterations, results);
    more_loops(iterations / 2, arr);
    
    /* Combine results to prevent elimination */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += results[i] + arr[i];
    }
    
    printf("Result: %d\n", total);
    return 0;
}
