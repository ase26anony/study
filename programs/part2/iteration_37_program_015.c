/* test_hw_doloop.c - Target coverage for hw-doloop.cc lines 429-436 */

#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int N, int* results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will have its own basic blocks */
    for (i = 0; i < limit; i++) {
        results[i] = i * 2;
        sum1 += results[i];
    }
    
    /* Loop 2: Adjacent but disjoint loop - no block intersection with Loop 1 */
    for (j = 0; j < limit; j++) {
        results[j + N] = j * 3;
        sum2 += results[j + N];
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* This should trigger: !bitmap_intersect_compl_p(other, loop) */
    /* So loop->loops.safe_push(other) */
    int outer_count = limit / 2;
    for (k = 0; k < outer_count; k++) {
        /* Loop 4: Outer loop containing Loop 3 */
        int inner_count = limit / 4;
        for (int m = 0; m < inner_count; m++) {
            results[k * inner_count + m] += k + m;
            sum3 += results[k * inner_count + m];
        }
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* This should trigger the else if path: !bitmap_intersect_compl_p(loop, other) */
    /* So other->loops.safe_push(loop) */
    int counter5 = 0;
    int counter6 = 0;
    
    /* Create shared basic block through goto */
    shared_block:
    {
        /* Volatile to prevent optimization */
        volatile int shared_value = counter5 + counter6;
        results[shared_value % N] = shared_value;
    }
    
    /* Loop 5: Contains goto to shared block */
    while (counter5 < limit) {
        if (counter5 % 3 == 0) {
            goto shared_block;  /* Jump to block also reachable from Loop 6 */
        }
        results[counter5] += 7;
        counter5++;
    }
    
    /* Loop 6: Also contains goto to same shared block */
    do {
        if (counter6 % 4 == 0) {
            goto shared_block;  /* Same shared block as Loop 5 */
        }
        results[counter6 + N/2] += 11;
        counter6++;
    } while (counter6 < limit);
    
    /* Loop 7: Complex overlapping with conditional inner structure */
    /* Creates more complex intersection pattern */
    int x = 0;
    int y = 0;
    
    for (x = 0; x < limit; x++) {
        /* Conditional that sometimes executes Loop 8's body */
        if (x % 2 == 0) {
            /* This block belongs to both Loop 7 and Loop 8 */
            for (y = 0; y < 2; y++) {
                results[x * 2 + y] *= 2;
            }
            /* Early continue creates more CFG edges */
            continue;
        }
        results[x] -= 1;
    }
    
    /* Loop 8: Overlaps with Loop 7 through the conditional */
    /* This creates bitmap intersection but neither is subset */
    int z = 0;
    while (z < limit) {
        if (z % 3 == 0) {
            /* Jump into what was Loop 7's conditional block */
            for (y = 0; y < 2; y++) {
                results[z * 2 + y] += 3;
            }
        } else {
            results[z] += 5;
        }
        z++;
    }
    
    /* Final side effect to prevent dead code elimination */
    results[0] = sum1 + sum2 + sum3;
}

/* Main function to drive execution */
int main() {
    const int N = 100;
    int* array = (int*)malloc(N * 4 * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array */
    for (int i = 0; i < N * 4; i++) {
        array[i] = i;
    }
    
    /* Call the function with all the loop patterns */
    test_loop_patterns(N, array);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < N * 4; i++) {
        checksum += array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(array);
    return 0;
}
