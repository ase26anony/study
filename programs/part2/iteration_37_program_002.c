/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture that supports hardware loops */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#elif __riscv
__attribute__((target("arch=rv64gc_zba")))
#endif
__attribute__((noinline, hot))
void test_loop_patterns(int N, int* results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k, m;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    for (i = 0; i < limit; i++) {
        results[i] = i * 2;
        asm volatile("" ::: "memory");  /* Prevent optimization */
    }
    
    /* Loop 2: Adjacent but disjoint loop (no block intersection with Loop 1) */
    for (j = 0; j < limit; j++) {
        results[j + N] = j * 3;
        asm volatile("" ::: "memory");
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop structure */
    k = 0;
    do {
        /* Loop 4: Outer loop containing Loop 3 */
        for (m = 0; m < limit/2; m++) {
            /* Loop 3: Inner loop - entirely within Loop 4's blocks */
            for (i = 0; i < 5; i++) {
                results[k] += i;
                asm volatile("" ::: "memory");
            }
            k++;
        }
    } while (k < limit);
    
    /* Create partial overlap scenario for bitmap_intersect_compl_p logic */
    /* Loop 5 and Loop 6 will share some blocks but not be subsets */
    int flag = 0;
    int counter5 = 0, counter6 = 0;
    
    /* Loop 5: First loop in overlapping pair */
    for (i = 0; i < limit; i++) {
        results[i] += 1;
        
        /* Conditional that creates shared block */
        if (flag == 0) {
            /* This block will be shared with Loop 6 */
            results[i] *= 2;
            
            /* Jump to Loop 6's body - creating CFG overlap */
            if (i == limit/2) {
                goto overlap_entry;  /* Creates shared basic block */
            }
        }
        counter5++;
    }
    
    /* Loop 6: Second loop that overlaps with Loop 5 */
    j = 0;
overlap_entry:  /* Label that both loops can reach */
    do {
        /* This block is shared when entered from Loop 5's goto */
        results[j] += 3;
        
        if (j > limit/2) {
            /* Exit the overlap region */
            break;
        }
        
        j++;
    } while (j < limit);
    
    /* Continue Loop 6 from where we left off */
    for (; j < limit; j++) {
        results[j] -= 1;
        counter6++;
        asm volatile("" ::: "memory");
    }
    
    /* Another overlapping pattern with different structure */
    int x = 0, y = 0;
    
    /* Loop 7: while loop structure */
    while (x < limit) {
        /* Loop 8: for loop inside while - partial overlap candidate */
        for (y = 0; y < limit/3; y++) {
            results[x + y] = x * y;
            
            /* Conditional exit to outer loop */
            if (results[x + y] > 1000) {
                x++;  /* Changes outer loop counter */
                goto partial_overlap;  /* Creates intersecting CFG */
            }
        }
        x++;
    }
    
partial_overlap:
    /* This block belongs to both Loop 7 and Loop 8's CFG */
    results[0] = x + y;
    
    /* Loop 9: Final simple loop to ensure all loops are analyzed */
    for (i = 0; i < 10; i++) {
        volatile int* ptr = &results[i % N];
        *ptr += i;
        asm volatile("" ::: "memory");
    }
}

int main() {
    const int N = 100;
    int* results = (int*)malloc(N * 10 * sizeof(int));
    
    if (!results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array */
    for (int i = 0; i < N * 10; i++) {
        results[i] = i;
    }
    
    /* Call function with carefully constructed loops */
    test_loop_patterns(N, results);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < N * 10; i++) {
        checksum += results[i];
        checksum &= 0xFFFF;  /* Prevent overflow */
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Verify some expected patterns */
    printf("Sample values: %d, %d, %d\n", results[0], results[N/2], results[N-1]);
    
    free(results);
    return 0;
}
