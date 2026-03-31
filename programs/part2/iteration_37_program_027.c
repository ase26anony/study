/* test_hw_doloop.c - Target coverage for hw-doloop.cc bitmap intersection logic */
#include <stdio.h>
#include <stdlib.h>

/* Force architecture that supports hardware loops */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#endif
__attribute__((noinline, hot))
void test_loop_patterns(int N, int *results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    for (i = 0; i < limit; i++) {
        results[i] = i * 2;
        asm volatile("" ::: "memory");  /* Prevent optimization */
    }
    
    /* Loop 2: Adjacent but disjoint loop - no block intersection with Loop 1 */
    int sum = 0;
    for (j = 0; j < limit; j++) {
        sum += results[j];
        asm volatile("" ::: "memory");
    }
    results[0] = sum;
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop (Loop 4) */
    int outer_limit = limit / 2;
    for (k = 0; k < outer_limit; k++) {
        /* Loop 3: Inner loop - entirely contained within Loop 4's blocks */
        for (i = 0; i < 5; i++) {
            results[k] += i;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create a more complex CFG with conditional jumps between loops */
    int flag = 0;
    int counter1 = 0, counter2 = 0;
    
    /* Loop 5: do-while style for CFG variation */
    do {
        results[counter1 % N] += counter1;
        asm volatile("" ::: "memory");
        
        /* Conditional that can jump into Loop 6's body */
        if (flag && (counter1 % 3 == 0)) {
            /* Jump to label inside Loop 6 */
            goto partial_overlap;
        }
        
        counter1++;
    } while (counter1 < limit);
    
    /* Loop 6: Partially overlaps with Loop 5 via goto */
    for (counter2 = 0; counter2 < limit; counter2++) {
        partial_overlap:
        results[counter2 % N] -= counter2;
        asm volatile("" ::: "memory");
        
        /* Another conditional that can jump back */
        if (flag && (counter2 % 4 == 0)) {
            /* This creates partial overlap - Loop 5 and Loop 6 share
               some basic blocks but neither is subset of the other */
            flag = !flag;
        }
    }
    
    /* Loop 7: Another loop that shares exit blocks with Loop 8 */
    int temp = 0;
    for (i = 0; i < limit; i++) {
        temp += results[i];
        if (temp > 1000) {
            /* Early exit that leads into Loop 8's initialization */
            goto shared_exit_path;
        }
    }
    
    /* Loop 8: Shares exit path with Loop 7 */
    shared_exit_path:
    for (j = 0; j < 10; j++) {
        results[j % N] = temp;
        asm volatile("" ::: "memory");
    }
}

/* Main driver that ensures all loops execute */
int main() {
    const int N = 100;
    int *array = (int*)malloc(N * sizeof(int));
    int *results = (int*)malloc(N * sizeof(int));
    
    if (!array || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < N; i++) {
        array[i] = i + 1;
        results[i] = 0;
    }
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N, results);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += results[i];
        checksum ^= array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(array);
    free(results);
    return 0;
}
