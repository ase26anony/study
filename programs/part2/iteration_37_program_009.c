/* test_hw_loops.c
 * Designed to trigger specific uncovered lines in GCC's hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_loops.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Mark function as hot to encourage hardware loop optimization */
__attribute__((hot, noinline, target("arch=armv8-a")))
void test_loop_patterns(int N, int *results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k;
    
    /* Loop 1: Simple countable loop - will be analyzed for hardware loops */
    for (i = 0; i < limit; i++) {
        results[i] = i * 2;
        /* Memory clobber to prevent optimization */
        asm volatile("" ::: "memory");
    }
    
    /* Loop 2: Adjacent but disjoint loop (shares no basic blocks with Loop 1) */
    int offset = limit;
    for (j = 0; j < limit; j++) {
        results[offset + j] = j * 3;
        asm volatile("" ::: "memory");
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop structure */
    int outer_start = offset * 2;
    for (k = 0; k < limit / 2; k++) {
        /* Loop 4: Inner loop - perfectly nested within Loop 3's body */
        int inner;
        for (inner = 0; inner < limit / 4; inner++) {
            results[outer_start + k * (limit/4) + inner] = k + inner;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops with goto */
    /* This creates the partial overlap scenario for bitmap_intersect_compl_p */
    int idx5 = 0, idx6 = 0;
    volatile int trigger = limit / 3;
    
    /* Loop 5: First loop in partial overlap pair */
    do {
        results[1000 + idx5] = idx5 * 5;
        asm volatile("" ::: "memory");
        
        /* Conditional that may jump into Loop 6 */
        if (idx5 == trigger) {
            goto overlap_label;  /* Jump into Loop 6's body */
        }
        
        idx5++;
    } while (idx5 < limit);
    
    /* Loop 6: Second loop in partial overlap pair */
    /* Note: This loop's entry point is after the goto label */
    idx6 = 0;
overlap_loop_start:
    while (idx6 < limit) {
overlap_label:  /* Label that Loop 5 can jump to */
        results[2000 + idx6] = idx6 * 7;
        asm volatile("" ::: "memory");
        
        /* Jump back to Loop 5's continuation? No, that would create irreducible flow.
           Instead, we'll have separate continuation */
        if (idx6 > trigger && idx5 < limit) {
            /* This creates shared basic blocks between the loops */
            results[3000] = idx5 + idx6;
            asm volatile("" ::: "memory");
        }
        
        idx6++;
        
        /* After jumping here from Loop 5, we need to prevent
           infinite loops - only continue if we haven't finished */
        if (idx5 >= limit && idx6 < limit) {
            continue;
        }
        break;
    }
    
    /* Loop 7: Another loop with complex control flow for additional overlap */
    int m = 0;
    int n = 0;
    
    /* Loop 7a and 7b: Two loops that share some blocks via switch */
    for (m = 0; m < limit; m++) {
        switch (m % 3) {
            case 0:
                results[4000 + m] = m;
                /* Fall through to shared block */
            case 1:
                /* Shared block between the two conceptual loops */
                results[5000] = m * 2;
                asm volatile("" ::: "memory");
                if (n < limit / 2) {
                    /* This creates another loop-like structure */
                    results[6000 + n] = n;
                    asm volatile("" ::: "memory");
                    n++;
                }
                break;
            case 2:
                results[7000 + m] = m * 3;
                asm volatile("" ::: "memory");
                break;
        }
    }
}

/* Alternative approach using nested loops with breaks to different scopes */
__attribute__((noinline, target("arch=armv8-a")))
void test_complex_overlap(int N, int *results) {
    volatile int limit = N;
    int x = 0, y = 0;
    int done_x = 0, done_y = 0;
    
    /* Two loops that partially overlap via shared conditional block */
    
    /* First loop structure */
    for (x = 0; x < limit && !done_x; x++) {
        results[8000 + x] = x * 11;
        asm volatile("" ::: "memory");
        
        /* Shared conditional block - also part of second loop */
        if (x > limit / 2) {
shared_block:
            results[9000] = x + y;
            asm volatile("" ::: "memory");
            
            /* This goto creates partial overlap */
            if (y < limit && !done_y) {
                y++;
                goto shared_block;  /* Jump back to shared block */
            }
        }
    }
    
    /* Second loop continuation */
    while (y < limit && !done_y) {
        results[10000 + y] = y * 13;
        asm volatile("" ::: "memory");
        y++;
        
        /* Re-enter shared block under some conditions */
        if (y == limit / 2) {
            goto shared_block;
        }
    }
}

int main() {
    const int N = 1000;
    int *results = (int*)malloc(sizeof(int) * 20000);
    if (!results) return 1;
    
    /* Initialize array */
    for (int i = 0; i < 20000; i++) {
        results[i] = 0;
    }
    
    /* Call the function with carefully constructed loops */
    test_loop_patterns(N, results);
    test_complex_overlap(N, results);
    
    /* Compute checksum to ensure all loops executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < 20000; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    free(results);
    return 0;
}
