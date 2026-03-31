/* test_hw_doloop.c - Target coverage for hw-doloop.cc bitmap intersection logic */

#include <stdio.h>
#include <stdint.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N, int* results) {
    volatile int M = N / 2;
    volatile int K = N / 3;
    
    int i, j, k;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    for (i = 0; i < N; i++) {
        results[i] = i * 2;
    }
    
    /* Loop 2: Adjacent but disjoint loop - no block intersection with Loop 1 */
    for (j = 0; j < M; j++) {
        results[j + N] = j * 3;
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* This should trigger: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap) */
    /* Loop 3 will be added to Loop 4's loops list */
    for (k = 0; k < K; k++) {
        /* Loop 4: Outer loop containing Loop 3 */
        int m;
        for (m = 0; m < 2; m++) {
            results[k + N + M + m] = k * 4 + m;
        }
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* This should trigger the else if path: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap) */
    /* They share some basic blocks but neither is a complete subset of the other */
    
    int x = 0;
    int y = 0;
    
    /* Loop 5: do-while loop for CFG variation */
    do {
        results[x + N + M + K] = x * 5;
        
        /* Conditional that creates partial overlap */
        if (x % 3 == 0) {
            /* Jump into Loop 6's body */
            goto partial_overlap;
        }
        
        x++;
    } while (x < N/2);
    
    /* Prevent fall-through to Loop 6 without the goto */
    goto skip_loop6;
    
partial_overlap:
    /* Loop 6: Partially overlapping with Loop 5 */
    /* Some blocks are shared via the goto, others are unique */
    for (y = 0; y < N/3; y++) {
        results[y + N + M + K + N/2] = y * 6;
        
        /* Shared block: This is reachable from both loops */
        if (y % 2 == 0) {
            /* This creates a shared basic block */
            results[y + N + M + K + N/2] += 100;
        }
        
        /* Jump back to Loop 5's continuation */
        if (y == N/6) {
            goto continue_loop5;
        }
    }
    
    /* Unique block for Loop 6 only */
    results[N + M + K + N/2 + N/3] = 999;
    goto loop_end;
    
continue_loop5:
    /* Continuation of Loop 5 after the partial overlap */
    x++;
    if (x < N/2) {
        goto partial_overlap;
    }
    
skip_loop6:
    /* Execute Loop 6 independently as well */
    for (y = 0; y < N/3; y++) {
        results[y + N + M + K + N/2] = y * 7;
    }
    
loop_end:
    /* Final operation to ensure side effects */
    results[0] += results[N-1];
}

/* Prevent inlining to preserve CFG structure */
#ifdef __GNUC__
__attribute__((noinline, target("arch=armv8-a")))
#endif
void another_loop_set(volatile int N, int* results, int offset) {
    /* Loop 7: Another countable loop in a different function */
    /* This ensures multiple loop structures are analyzed */
    int z;
    for (z = 0; z < N; z++) {
        results[offset + z] = z * 8;
        
        /* Nested loop inside - creates hierarchical relationship */
        int w;
        for (w = 0; w < 2; w++) {
            results[offset + z + N + w] = z * 9 + w;
        }
    }
}

int main() {
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    int array_size = N + N/2 + N/3 + N/2 + N/3 + 100;
    int* results = (int*)malloc(array_size * sizeof(int));
    
    if (!results) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array */
    for (int i = 0; i < array_size; i++) {
        results[i] = 0;
    }
    
    /* Call the function with carefully constructed loops */
    test_loop_patterns(N, results);
    
    /* Call another function with more loops */
    another_loop_set(N/2, results, array_size/2);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < array_size; i++) {
        checksum += results[i];
        checksum &= 0xFFFF;  /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Verify some expected values */
    printf("Sample values:\n");
    printf("results[0] = %d\n", results[0]);
    printf("results[N-1] = %d\n", results[N-1]);
    printf("results[N+M+K] = %d\n", results[N + N/2 + N/3]);
    
    free(results);
    return 0;
}
