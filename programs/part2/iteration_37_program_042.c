/* test_hw_doloop.c
 * Designed to trigger specific uncovered lines in GCC's hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_doloop.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force architecture support for hardware loops */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
static void test_loop_patterns(volatile int N, int *results) {
    volatile int M = N + 10;
    volatile int K = N / 2 + 5;
    
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int i, j, k;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    for (i = 0; i < N; i++) {
        sum1 += i;
        results[i] = i * 2;
    }
    
    /* Loop 2: Adjacent but disjoint loop - no block intersection with Loop 1 */
    for (j = 0; j < M; j++) {
        sum2 += j * 3;
        results[N + j] = j * 4;
    }
    
    /* Loop 3: Perfectly nested inside conditional - subset relationship */
    if (N > 5) {
        /* Loop 3a: Inner loop that's a perfect subset of the if block */
        for (k = 0; k < K; k++) {
            sum3 += k * 5;
            results[k] += k;
        }
    }
    
    /* Loop 4: Do-while loop for CFG variation */
    i = 0;
    do {
        sum4 += results[i];
        i++;
    } while (i < N && i < 20);
    
    /* Loop 5: Creates partial overlap with Loop 6 via goto */
    int counter = 0;
    
    /* Label for partial overlap */
    partial_overlap_start:
    
    /* Loop 5 body - will share some blocks with Loop 6 */
    for (i = 0; i < N/2; i++) {
        if (counter++ > 100) {
            /* This goto creates partial overlap with Loop 6 */
            goto inside_loop6;
        }
        results[i] += i * i;
    }
    
    /* Loop 6: Partially overlaps with Loop 5 */
    for (j = N/2; j < N; j++) {
        inside_loop6:
        results[j] -= j;
        
        /* This creates intersection but not subset relationship */
        if (j == N/2 + 2) {
            /* Jump back to Loop 5's region - creates complex CFG */
            if (counter < 50) {
                counter++;
                goto partial_overlap_start;
            }
        }
    }
    
    /* Loop 7: Another loop that shares exit blocks */
    int limit = N < 30 ? N : 30;
    for (i = 0; i < limit; i++) {
        /* Shared computation with Loop 8's exit path */
        results[i] = results[i] % 256;
    }
    
    /* Loop 8: Shares header/exit blocks but not body */
    i = 0;
    while (i < limit) {
        if (results[i] > 128) {
            /* Early exit that jumps to shared region */
            goto shared_exit;
        }
        results[i] += 1;
        i++;
    }
    
    shared_exit:
    results[0] = sum1 + sum2 + sum3 + sum4;
}

/* Complex nested loops to trigger hierarchical analysis */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a"), noinline))
#endif
static void complex_nesting(int *arr, volatile int size) {
    int i, j, k;
    
    /* Triple nested loop - creates clear hierarchy */
    for (i = 0; i < size; i++) {
        arr[i] = i;
        for (j = 0; j < size/2; j++) {
            arr[i] += j;
            for (k = 0; k < size/4; k++) {
                /* Prevent optimization */
                __asm__ volatile ("" : : "r"(k) : "memory");
                arr[i] -= k;
            }
        }
    }
    
    /* Adjacent loop with shared induction variable */
    for (i = size - 1; i >= 0; i--) {
        arr[i] = arr[i] * 2;
    }
}

/* Main function to drive execution */
int main() {
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    int results[256] = {0};
    int arr[100];
    
    /* Call the function with carefully constructed loops */
    test_loop_patterns(N, results);
    
    /* Additional complex nesting */
    complex_nesting(arr, N/2);
    
    /* Compute checksum to ensure all loops execute */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += results[i];
    }
    for (int i = 0; i < 100; i++) {
        checksum += arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
