/* test_hw_doloop.c - Target coverage for hw-doloop.cc lines 429-436 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture support for hardware loops */
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
    for (j = 0; j < limit; j++) {
        results[j + N] = j * 3;
        asm volatile("" : : : "memory");
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop structure */
    k = 0;
    do {
        /* Loop 4: Outer loop containing Loop 3 */
        for (i = 0; i < limit/2; i++) {
            /* Loop 3: Inner loop - all blocks are subset of Loop 4's blocks */
            for (j = 0; j < 5; j++) {
                results[k++] = i * j;
                asm volatile("" : : : "memory");
            }
        }
    } while (k < N * 2);
    
    /* Loop 5 and Loop 6: Partially overlapping loops with goto creating shared blocks */
    /* This creates the bitmap_intersect_p && bitmap_intersect_compl_p case */
    int shared_counter = 0;
    
    /* Loop 5: First partially overlapping loop */
    for (i = 0; i < limit; i++) {
        results[i] += i;
        asm volatile("" : : : "memory");
        
        /* Conditional that may jump into Loop 6 */
        if (i % 3 == 0 && shared_counter < limit/2) {
            /* Jump label inside Loop 6's body */
            goto overlap_point;
        }
        
        /* Normal Loop 5 continuation */
        results[i] *= 2;
        
        /* Entry point from Loop 6's goto */
        reentry_point:
        asm volatile("" : : : "memory");
        continue;
        
        /* This creates a basic block that belongs to both loops */
        overlap_point:
        shared_counter++;
        results[N + i] = shared_counter;
        
        /* Jump back to Loop 5 */
        goto reentry_point;
    }
    
    /* Loop 6: Second partially overlapping loop */
    /* Shares the overlap_point block with Loop 5 */
    for (j = limit/2; j < limit; j++) {
        results[j] = j - limit/2;
        asm volatile("" : : : "memory");
        
        /* Conditional jump into Loop 5's body */
        if (j % 4 == 0) {
            goto overlap_point;
        }
        
        results[j] += shared_counter;
    }
    
    /* Loop 7: Another loop with complex control flow for additional coverage */
    int toggle = 0;
    while (toggle < limit) {
        /* Create conditional blocks that might be shared in analysis */
        if (toggle % 2) {
            /* This block might be analyzed as intersecting with other loops */
            for (i = 0; i < 3; i++) {
                results[toggle + i] ^= 0xFF;
            }
            goto skip_increment;
        }
        
        results[toggle] |= 0xAA;
        
        skip_increment:
        toggle++;
        
        /* Small inner loop that might create nesting relationship */
        int inner = 0;
        do {
            results[toggle + inner] += inner;
            inner++;
        } while (inner < 2);
    }
}

/* Main function to drive execution */
int main() {
    const int N = 100;
    int *results = (int*)malloc(N * 10 * sizeof(int));
    
    if (!results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array */
    for (int i = 0; i < N * 10; i++) {
        results[i] = i;
    }
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N, results);
    
    /* Compute checksum to ensure all loops executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < N * 10; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    free(results);
    return 0;
}
