/* test_hw_loops.c
 * Designed to trigger specific uncovered lines in GCC's hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_loops.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture support for hardware loops */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N) {
    volatile int i, j, k;
    int array1[1000], array2[1000], array3[1000];
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Initialize arrays to prevent optimization */
    for (int idx = 0; idx < 1000; idx++) {
        array1[idx] = idx;
        array2[idx] = idx * 2;
        array3[idx] = idx * 3;
    }
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < N; i++) {
        sum1 += array1[i % 1000];
        /* Simple side effect to prevent dead code elimination */
        asm volatile("" : "+r"(sum1) : : "memory");
    }
    
    /* Loop 2: Adjacent but disjoint loop (no block intersection with Loop 1) */
    for (j = 0; j < N + 5; j++) {
        sum2 += array2[j % 1000];
        asm volatile("" : "+r"(sum2) : : "memory");
    }
    
    /* Loop 3: Perfectly nested within Loop 4 */
    /* Outer loop */
    for (k = 0; k < N / 2; k++) {
        /* Inner loop - entirely contained within outer loop's blocks */
        for (i = 0; i < 10; i++) {
            sum3 += array3[(k * 10 + i) % 1000];
            asm volatile("" : "+r"(sum3) : : "memory");
        }
    }
    
    /* Loop 4 and Loop 5: Partially overlapping loops */
    /* Create a more complex CFG with conditional branching between loops */
    
    /* Loop 4 */
    int m = 0;
    int limit4 = N / 3;
    
    /* Label for partial overlap */
    loop4_start:
    if (m >= limit4) goto loop4_end;
    
    sum1 += array1[m % 1000];
    asm volatile("" : "+r"(sum1) : : "memory");
    
    /* Conditional that may jump into Loop 5 */
    if (m % 3 == 0) {
        /* Jump to Loop 5's body - creating partial overlap */
        goto loop5_middle;
    }
    
    m++;
    goto loop4_start;
    
    loop4_end:
    
    /* Loop 5 - partially overlaps with Loop 4 via goto */
    int n = 0;
    int limit5 = N / 2;
    
    while (n < limit5) {
        /* Entry point for partial overlap from Loop 4 */
        loop5_middle:
        sum2 += array2[n % 1000];
        asm volatile("" : "+r"(sum2) : : "memory");
        
        n++;
        
        /* Another conditional that could jump back to Loop 4 */
        if (n % 4 == 0 && m < limit4) {
            /* This creates the partial overlap scenario */
            goto loop4_start;
        }
    }
    
    /* Loop 6: Do-while loop variation */
    int p = 0;
    do {
        sum3 += array3[p % 1000];
        asm volatile("" : "+r"(sum3) : : "memory");
        p++;
    } while (p < N / 4);
    
    /* Use results to prevent optimization */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
}

/* Another function with different loop patterns */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void more_loop_patterns(volatile int M) {
    int arr[500];
    int total = 0;
    
    /* Initialize */
    for (int i = 0; i < 500; i++) {
        arr[i] = i;
    }
    
    /* Loop A */
    for (int a = 0; a < M; a++) {
        total += arr[a % 500];
    }
    
    /* Loop B - shares some blocks with Loop C via switch */
    int b = 0;
    shared_block:
    if (b >= M / 2) goto end_b;
    
    total += arr[b % 500] * 2;
    
    switch (b % 3) {
        case 0:
            /* Fall through to Loop C */
            goto loop_c_entry;
        case 1:
            b++;
            goto shared_block;
        default:
            b += 2;
            goto shared_block;
    }
    
    end_b:
    
    /* Loop C - partially overlaps with Loop B */
    int c = 0;
    loop_c_entry:
    if (c >= M / 3) goto end_c;
    
    total += arr[c % 500] * 3;
    
    if (c % 2 == 0 && b < M / 2) {
        /* Jump back to shared block */
        goto shared_block;
    }
    
    c++;
    goto loop_c_entry;
    
    end_c:
    
    asm volatile("" : : "r"(total) : "memory");
}

int main() {
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    volatile int M = 75;
    
    /* Call functions with hardware loop patterns */
    test_loop_patterns(N);
    more_loop_patterns(M);
    
    /* Create checksum to ensure execution */
    int checksum = N + M;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
