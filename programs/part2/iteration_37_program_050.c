/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */

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
        asm volatile("" ::: "memory");  /* Prevent optimization */
    }
    
    /* Loop 2: Adjacent but disjoint loop (no block intersection with Loop 1) */
    int offset = limit;
    for (j = 0; j < limit; j++) {
        results[offset + j] = j * 3;
        asm volatile("" ::: "memory");
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop structure */
    int outer_start = offset * 2;
    for (k = 0; k < limit / 2; k++) {
        /* Loop 4: Inner loop (perfectly nested) */
        /* This creates hierarchical relationship: Loop 4's blocks are subset of Loop 3's */
        for (i = 0; i < limit / 4; i++) {
            results[outer_start + k * (limit/4) + i] = k * i;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops with goto creating shared blocks */
    /* This triggers the else if path (bitmap_intersect_compl_p both ways false) */
    int idx5 = 0, idx6 = 0;
    volatile int flag = 1;
    
    /* Loop 5 */
    for (i = 0; i < limit; i++) {
        results[1000 + i] = i * 5;
        
        /* Conditional that can jump into Loop 6's body */
        if (flag && i == limit/2) {
            /* Jump to label inside Loop 6 */
            goto overlap_point;
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* Some code between loops to create separate basic blocks */
    results[2000] = 999;
    
    /* Loop 6 */
    for (j = 0; j < limit; j++) {
        results[2000 + j] = j * 7;
        
overlap_point:
        /* This label creates a shared basic block between Loop 5 and Loop 6 */
        results[3000 + j] = i + j;  /* Uses 'i' from Loop 5 */
        asm volatile("" ::: "memory");
        
        /* Break the overlap after first iteration to maintain loop structure */
        if (j == 0 && flag) {
            /* Jump back? Actually, we want partial overlap, not infinite loop */
            /* The CFG now has Loop 5 blocks intersecting with Loop 6 blocks */
            /* but neither is subset of the other due to the goto */
        }
    }
    
    /* Loop 7: do-while loop for CFG variation */
    int counter = 0;
    do {
        results[4000 + counter] = counter * 11;
        asm volatile("" ::: "memory");
        counter++;
    } while (counter < limit/3);
    
    /* Loop 8: Another loop with early exit creating complex CFG */
    for (i = 0; i < limit; i++) {
        results[5000 + i] = i * 13;
        if (i > limit/2) {
            /* Early exit creates additional basic blocks */
            break;
        }
        asm volatile("" ::: "memory");
    }
}

/* Alternative implementation with more explicit partial overlap */
__attribute__((noinline, cold))
void create_partial_overlap(int N, int *arr1, int *arr2) {
    volatile int switch_flag = N % 2;
    int x, y;
    
    /* Two loops that will partially overlap via shared conditional block */
    
    /* First loop */
    for (x = 0; x < N; x++) {
        arr1[x] = x * 17;
        
        /* Shared conditional block - will be part of both loops' bitmaps */
        if (switch_flag) {
            /* This block is entered from both loops */
            arr2[0] = x;  /* Use x from first loop */
            /* Don't jump here - just let both loops contain this code */
        }
        
        asm volatile("" ::: "memory");
    }
    
    /* Some intermediate code */
    arr1[N] = -1;
    
    /* Second loop that also contains the same conditional structure */
    for (y = 0; y < N; y++) {
        arr2[y] = y * 19;
        
        /* Same conditional block - creates intersection */
        if (switch_flag) {
            arr1[0] = y;  /* Use y from second loop */
            /* This creates partial overlap: both loops contain this block */
            /* but have other unique blocks */
        }
        
        asm volatile("" ::: "memory");
    }
}

int main() {
    const int N = 100;
    int *results = (int*)malloc(sizeof(int) * 6000);
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 6000; i++) {
        results[i] = 0;
    }
    
    /* Call the function with carefully constructed loops */
    test_loop_patterns(N, results);
    
    /* Create another set of loops with partial overlap */
    int *arr1 = (int*)malloc(sizeof(int) * (N + 1));
    int *arr2 = (int*)malloc(sizeof(int) * N);
    create_partial_overlap(N, arr1, arr2);
    
    /* Compute checksum to ensure all loops executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < 6000; i++) {
        checksum += results[i];
    }
    for (int i = 0; i <= N; i++) {
        checksum += arr1[i];
    }
    for (int i = 0; i < N; i++) {
        checksum += arr2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    free(results);
    free(arr1);
    free(arr2);
    
    return 0;
}
