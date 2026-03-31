/* test_loop_doloop.c
 * Compile with: gcc -O1 -fdump-rtl-loop2 -fdump-rtl-doloop test_loop_doloop.c -o test_loop
 * Run with: ./test_loop 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization of critical function */
__attribute__((noinline, optimize("O1")))
void test_loops(int n, int* arr, volatile int* sink) {
    int i, j;
    int local_n = n;
    
    /* Loop variant 1: Basic for loop with decrement */
    for (i = n; i > 0; i--) {
        arr[i % 256] = arr[(i - 1) % 256] + 1;
        *sink = arr[i % 256];  /* Volatile store prevents elimination */
    }
    
    /* Loop variant 2: do-while with pre-decrement */
    local_n = n;
    do {
        arr[local_n % 256] = arr[(local_n + 1) % 256] * 2;
        *sink = arr[local_n % 256];
    } while (--local_n > 0);
    
    /* Loop variant 3: while loop with post-decrement */
    local_n = n;
    while (local_n--) {
        arr[local_n % 256] = arr[(local_n + 2) % 256] - 3;
        *sink = arr[local_n % 256];
    }
    
    /* Loop variant 4: Nested loops with decrementing inner loop */
    for (i = 0; i < 10; i++) {
        for (j = n / 10; j > 0; j--) {
            arr[(i + j) % 256] = arr[(i * j) % 256] + i - j;
            *sink = arr[(i + j) % 256];
        }
    }
    
    /* Loop variant 5: Complex decrement with if condition */
    local_n = n;
    while (local_n > 0) {
        if (local_n % 3 == 0) {
            arr[local_n % 256] = 0;
        }
        arr[local_n % 256] += local_n;
        *sink = arr[local_n % 256];
        local_n--;
    }
}

/* Another function with different optimization to increase chances */
__attribute__((noinline, optimize("O1")))
void more_loops(int m, int* brr, volatile int* sink) {
    int k = m;
    
    /* Loop with compound condition */
    while (k > 0) {
        brr[k % 128] = brr[(k - 1) % 128] + k;
        *sink = brr[k % 128];
        k--;
    }
    
    /* Loop with function call in body (prevents some optimizations) */
    for (k = m; k > 0; k--) {
        brr[k % 128] = brr[(k + 1) % 128] * 3;
        *sink = brr[k % 128];
        /* Small memory barrier */
        asm volatile("" ::: "memory");
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <iteration_count>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 20) {
        printf("Please use at least 20 iterations\n");
        return 1;
    }
    
    /* Use volatile to prevent constant propagation */
    volatile int init_val = iterations;
    int n = init_val;
    
    /* Arrays to work on */
    int array1[256] = {0};
    int array2[128] = {0};
    
    /* Volatile sink to prevent dead code elimination */
    volatile int sink = 0;
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < 256; i++) {
        array1[i] = i + 1;
    }
    for (int i = 0; i < 128; i++) {
        array2[i] = i * 2;
    }
    
    /* Call test functions */
    test_loops(n, array1, &sink);
    more_loops(n / 2, array2, &sink);
    
    /* Compute checksum to verify execution */
    int sum1 = 0, sum2 = 0;
    for (int i = 0; i < 256; i++) {
        sum1 += array1[i];
    }
    for (int i = 0; i < 128; i++) {
        sum2 += array2[i];
    }
    
    printf("Result: sum1 = %d, sum2 = %d, sink = %d\n", sum1, sum2, sink);
    
    return 0;
}
