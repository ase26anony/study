/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */

#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int N, int *results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k, m;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < limit; i++) {
        results[i] = i * 2;
        sum1 += results[i];
    }
    
    /* Loop 2: Adjacent but disjoint loop - shares no blocks with Loop 1 */
    /* This should make bitmap_intersect_p return false initially */
    for (j = 0; j < limit; j++) {
        results[j + N] = j * 3;
        sum2 += results[j + N];
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* Loop 4 entirely contains Loop 3 */
    /* This should trigger: loop4->loops.safe_push(loop3) */
    m = 0;
    do {
        /* Loop 3 - inner loop */
        for (k = 0; k < 5; k++) {
            results[m * 5 + k + 2*N] = m * k;
            sum3 += results[m * 5 + k + 2*N];
        }
        m++;
    } while (m < limit);
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create complex CFG with goto to achieve partial overlap */
    /* This should trigger the else if path: other->loops.safe_push(loop) */
    int x = 0;
    int y = 0;
    
    /* Loop 5 */
    for (x = 0; x < limit; x++) {
        if (x % 3 == 0) {
            /* Jump into Loop 6's body, creating partial overlap */
            goto overlap_point;
        }
        results[x + 3*N] = x * x;
        sum4 += results[x + 3*N];
        continue;
        
overlap_point:
        /* Loop 6 - starts here, overlaps with Loop 5 */
        for (y = 0; y < limit/2; y++) {
            results[y + 4*N] = x + y;
            sum4 += results[y + 4*N];
            
            /* Jump back to Loop 5 */
            if (y == limit/4) {
                goto back_to_loop5;
            }
        }
        /* Never reached when jumping back */
        continue;
        
back_to_loop5:
        /* Continue Loop 5 */
        results[x + 3*N] = x * 7;
        sum4 += results[x + 3*N];
    }
    
    /* Loop 7: Another simple loop to create more adjacency */
    /* Mixed do-while to vary CFG structure */
    int z = 0;
    do {
        results[z + 5*N] = z * 11;
        z++;
    } while (z < limit);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3), "r"(sum4) : "memory");
}

/* Main driver to ensure execution */
int main() {
    const int N = 100;
    int *array = (int*)malloc(6 * N * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array */
    for (int i = 0; i < 6 * N; i++) {
        array[i] = 0;
    }
    
    /* Call the function with all the loop patterns */
    test_loop_patterns(N, array);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 6 * N; i++) {
        checksum ^= array[i];  /* XOR checksum */
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(array);
    return 0;
}
