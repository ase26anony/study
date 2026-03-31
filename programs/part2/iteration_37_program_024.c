/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */

#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N) {
    volatile int array1[1024], array2[1024], array3[1024];
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    volatile int i, j, k;
    
    /* Initialize arrays with non-deterministic pattern */
    for (i = 0; i < 1024; i++) {
        array1[i] = i * 3;
        array2[i] = i * 5;
        array3[i] = i * 7;
    }
    
    /* Loop 1: Simple countable loop - will be analyzed for hardware loop */
    for (i = 0; i < N; i++) {
        sum1 += array1[i];
        /* Memory clobber to prevent optimization */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Loop 2: Adjacent but disjoint loop (no basic block overlap with Loop 1) */
    for (j = 0; j < N; j++) {
        sum2 += array2[j];
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Loop 3: Perfectly nested inside conditional - creates hierarchical relationship */
    /* This should trigger: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap) */
    if (N > 10) {
        for (k = 0; k < N/2; k++) {
            sum3 += array3[k];
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Loop 4: Do-while loop for CFG variation */
    i = 0;
    do {
        array1[i] += sum1;
        __asm__ volatile ("" : : : "memory");
        i++;
    } while (i < N);
    
    /* Loop 5 and 6: Partially overlapping loops using goto */
    /* This should trigger the else-if path: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap) */
    
    /* Loop 5 - outer loop with conditional jump into Loop 6 */
    for (i = 0; i < N; i++) {
        sum1 += array1[i] * 2;
        
        /* Conditional that creates partial overlap */
        if (array1[i] > N/2) {
            /* Jump into Loop 6's body - creates CFG overlap */
            goto partial_overlap_entry;
        }
        
        __asm__ volatile ("" : : : "memory");
        
        /* Label inside Loop 5's body that Loop 6 can jump to */
        loop5_continue:
        sum2 += array2[i];
    }
    
    /* Loop 6 - partially overlaps with Loop 5 via goto */
    j = 0;
    while (j < N) {
        partial_overlap_entry:
        sum3 += array3[j];
        
        /* Jump back into Loop 5's body */
        if (j % 3 == 0) {
            goto loop5_continue;
        }
        
        __asm__ volatile ("" : : : "memory");
        j++;
    }
    
    /* Loop 7: Another loop with complex exit condition */
    for (i = N-1; i >= 0; i--) {
        array1[i] -= sum1;
        if (array1[i] < 0) {
            break;  /* Creates additional basic blocks */
        }
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = sum1 + sum2 + sum3;
    (void)dummy;
}

/* Main function to ensure execution */
int main() {
    volatile int N = 100;  /* Volatile to prevent constant propagation */
    
    /* Call the test function multiple times with different N values */
    test_loop_patterns(N);
    test_loop_patterns(N/2);
    test_loop_patterns(N*2);
    
    /* Compute checksum to ensure all loops executed */
    volatile int checksum = 0;
    for (volatile int i = 0; i < 10; i++) {
        checksum += i;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
