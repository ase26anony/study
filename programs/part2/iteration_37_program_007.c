/* test_hw_doloop.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force architecture that supports hardware loops */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#elif __riscv
__attribute__((target("arch=rv64gc_zba")))
#endif
__attribute__((noinline, hot))
void test_loop_patterns(int n, int* results) {
    volatile int limit1 = n + 3;  /* Prevent constant propagation */
    volatile int limit2 = n + 5;
    volatile int limit3 = n + 7;
    volatile int limit4 = n + 11;
    
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int i, j, k, m;
    
    /* Loop 1: Simple countable loop - will be in its own basic blocks */
    for (i = 0; i < limit1; i++) {
        sum1 += i * 2;
        /* Memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
    }
    results[0] = sum1;
    
    /* Loop 2: Perfectly nested inside conditional - creates subset relationship */
    if (limit2 > 0) {
        j = 0;
        do {
            sum2 += j * 3;
            /* Loop 3: Perfectly nested loop - should be subset of Loop 2's blocks */
            for (k = 0; k < 5; k++) {
                sum3 += k * (j + 1);
                asm volatile("" : : : "memory");
            }
            j++;
            asm volatile("" : : : "memory");
        } while (j < limit2);
    }
    results[1] = sum2;
    results[2] = sum3;
    
    /* Loop 4: Partially overlapping with Loop 5 via goto - triggers bitmap_intersect_compl_p */
    m = 0;
    int flag = 0;
    
    /* Label for partial overlap */
    partial_overlap_start:
    
    /* Loop 4 body start */
    while (m < limit3) {
        sum4 += m * 4;
        asm volatile("" : : : "memory");
        
        /* Conditional that creates partial overlap with Loop 5 */
        if (flag == 0 && m > limit3/2) {
            flag = 1;
            /* Jump into Loop 5's body - creates CFG overlap */
            goto enter_loop5;
        }
        m++;
    }
    /* Loop 4 end */
    
    /* Loop 5: Partially overlaps with Loop 4 via the goto above */
    int p = 0;
    int sum5 = 0;
    
    /* This label is inside Loop 5 but reachable from Loop 4 */
    enter_loop5:
    
    for (p = 0; p < limit4; p++) {
        sum5 += p * 5;
        asm volatile("" : : : "memory");
        
        /* Jump back to Loop 4's start - creates mutual partial overlap */
        if (p == limit4/2 && flag == 1) {
            goto partial_overlap_start;
        }
    }
    
    results[3] = sum4;
    results[4] = sum5;
    
    /* Loop 6: Adjacent but disjoint from all others - no intersection */
    int q;
    int sum6 = 0;
    for (q = 0; q < n; q++) {
        sum6 += q * 6;
        asm volatile("" : : : "memory");
    }
    results[5] = sum6;
    
    /* Loop 7: Another perfectly nested structure */
    int r, s;
    int sum7 = 0, sum8 = 0;
    for (r = 0; r < 3; r++) {
        sum7 += r;
        asm volatile("" : : : "memory");
        
        /* Loop 8: Nested inside Loop 7 - subset relationship */
        for (s = 0; s < 2; s++) {
            sum8 += s * r;
            asm volatile("" : : : "memory");
        }
    }
    results[6] = sum7;
    results[7] = sum8;
}

/* Helper to ensure loops aren't optimized away */
volatile int global_counter = 0;

int main() {
    const int N = 100;
    int* results = (int*)malloc(8 * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 8; i++) {
        results[i] = i;
    }
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N, results);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += results[i];
        global_counter += results[i];  /* Volatile side effect */
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    free(results);
    return 0;
}
