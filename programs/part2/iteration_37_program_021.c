/* test_hw_loops.c
 * Designed to trigger specific uncovered lines in GCC's hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_loops.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture support for hardware loops */
#ifdef __GNUC__
#define HOT __attribute__((hot, noinline))
#else
#define HOT
#endif

/* Volatile variables to prevent constant propagation */
volatile int N = 100;
volatile int M = 50;
volatile int K = 75;

/* Arrays for side effects */
int array1[200];
int array2[200];
int array3[200];

/* Function containing all loop patterns */
HOT void test_loop_patterns(void) {
    int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < N; i++) {
        array1[i] = i * 2;
        sum1 += array1[i];
    }
    
    /* Loop 2: Adjacent but disjoint loop (no block intersection with Loop 1) */
    for (j = 0; j < M; j++) {
        array2[j] = j * 3;
        sum2 += array2[j];
    }
    
    /* Loop 3: Perfectly nested inside conditional - creates subset relationship */
    if (sum1 > 0) {
        /* Loop 3a: Inner loop that's a perfect subset of the if block */
        for (k = 0; k < K; k++) {
            array3[k] = k * 4;
            sum3 += array3[k];
        }
    }
    
    /* Loop 4: Do-while loop for CFG variation */
    int cnt = 0;
    do {
        array1[cnt % N] += cnt;
        cnt++;
    } while (cnt < N);
    
    /* Loop 5: Creates partial overlap with Loop 6 via goto */
    int x = 0;
    int y = 0;
    
    /* Start of Loop 5 */
loop5_start:
    for (x = 0; x < N/2; x++) {
        array1[x] += x;
        
        /* Conditional that creates partial overlap */
        if (array1[x] % 3 == 0) {
            /* This goto jumps into Loop 6's body */
            goto loop6_inner;
        }
        
        array2[x % M] += x;
    }
    
    /* Loop 6: Partially overlaps with Loop 5 via the goto */
    for (y = 0; y < M; y++) {
        array2[y] += y;
        
loop6_inner:
        /* Shared block - both loops can reach this */
        array3[y % K] = array1[y % N] + array2[y];
        
        if (y > N/4) {
            /* Jump back to Loop 5 if needed */
            if (array3[y % K] % 2 == 0) {
                /* This creates the partial overlap */
                goto loop5_continue;
            }
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    return;
    
loop5_continue:
    /* Continuation of Loop 5 after partial overlap */
    for (; x < N; x++) {
        array1[x] += x * 2;
    }
}

/* Additional function with more complex nesting */
HOT void nested_loops_complex(void) {
    volatile int limit = 80;
    int a, b, c;
    int temp[100] = {0};
    
    /* Outer loop */
    for (a = 0; a < limit; a++) {
        /* Middle loop - subset of outer */
        for (b = 0; b < a; b++) {
            temp[b] = a * b;
            
            /* Innermost loop - subset of middle */
            for (c = 0; c < b; c++) {
                temp[c] += array1[c % N];
            }
        }
        
        /* Another inner loop at same level as middle loop */
        for (b = limit - 1; b >= 0; b--) {
            array2[b] += temp[b % 100];
        }
    }
    
    /* Loop with early exit that creates interesting CFG */
    int idx = 0;
    while (idx < limit) {
        array3[idx] = idx * idx;
        
        if (array3[idx] > 1000) {
            /* Early exit creates different block structure */
            break;
        }
        
        /* Nested while inside while */
        int inner = 0;
        while (inner < 10) {
            array1[(idx + inner) % N] += inner;
            inner++;
        }
        
        idx++;
    }
}

/* Main function to ensure execution */
int main(void) {
    int i;
    
    /* Initialize arrays */
    for (i = 0; i < 200; i++) {
        array1[i] = 0;
        array2[i] = 0;
        array3[i] = 0;
    }
    
    /* Call the function with loop patterns */
    test_loop_patterns();
    
    /* Call additional function for more coverage */
    nested_loops_complex();
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (i = 0; i < 200; i++) {
        checksum += array1[i] + array2[i] + array3[i];
        checksum &= 0xFFFF; /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Use results to prevent dead code elimination */
    if (checksum > 1000) {
        printf("Patterns executed successfully\n");
    }
    
    return checksum != 0 ? 0 : 1;
}
