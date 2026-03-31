/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture with hardware loop support */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#elif __riscv
__attribute__((target("arch=rv64gc_zba")))
#endif
__attribute__((noinline, hot))
void test_loop_patterns(int N, int *results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    for (i = 0; i < limit; i++) {
        results[i] = i * 2;
        asm volatile("" : : : "memory");  /* Prevent optimization */
    }
    
    /* Loop 2: Adjacent but disjoint loop - no block intersection with Loop 1 */
    int sum = 0;
    for (j = 0; j < limit; j++) {
        sum += results[j];
        asm volatile("" : : : "memory");
    }
    results[0] = sum;
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop structure */
    int outer_idx = 0;
    do {
        /* Loop 4: Outer loop containing Loop 3 */
        int inner_idx = 0;
        while (inner_idx < limit) {
            /* Loop 3: Inner loop - entirely within Loop 4's blocks */
            for (k = 0; k < 5; k++) {
                results[inner_idx] += k;
                asm volatile("" : : : "memory");
            }
            inner_idx++;
        }
        outer_idx++;
    } while (outer_idx < 3);
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create complex CFG with goto creating partial overlap */
    int x = 0;
    
    /* Loop 5: First loop with conditional branch */
    for (x = 0; x < limit; x++) {
        if (x % 3 == 0) {
            /* This goto creates partial overlap with Loop 6 */
            /* Jump to a label inside Loop 6's body */
            goto overlap_point;
        }
        results[x] += x;
        
        /* Continue label for Loop 6's jump back */
        continue_loop5:
        asm volatile("" : : : "memory");
    }
    
    /* Loop 6: Second loop that overlaps with Loop 5 */
    int y = limit - 1;
    while (y >= 0) {
        results[y] -= y;
        
        overlap_point:
        /* Shared basic block between Loop 5 and Loop 6 */
        results[y] *= 2;
        asm volatile("" : : : "memory");
        
        if (y > limit/2) {
            /* Jump back into Loop 5 */
            goto continue_loop5;
        }
        y--;
    }
    
    /* Loop 7: Another adjacent loop */
    int z;
    for (z = 0; z < limit; z++) {
        results[z] = results[z] % 100;
        asm volatile("" : : : "memory");
    }
}

int main() {
    const int N = 100;
    int *array = (int*)malloc(N * sizeof(int));
    if (!array) return 1;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        array[i] = i;
    }
    
    /* Call function with all loop patterns */
    test_loop_patterns(N, array);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum = (checksum + array[i]) % 1000;
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Verify array was modified */
    int modified = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i) modified++;
    }
    printf("Modified elements: %d/%d\n", modified, N);
    
    free(array);
    return 0;
}
