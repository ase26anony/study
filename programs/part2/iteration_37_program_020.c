/* test_hw_loops.c
 * Designed to trigger uncovered bitmap intersection logic in hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_loops.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture support for hardware loops */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int N, int M, int K) {
    volatile int limit1 = N;  /* Prevent constant propagation */
    volatile int limit2 = M;
    volatile int limit3 = K;
    
    int array1[1000] = {0};
    int array2[1000] = {0};
    int array3[1000] = {0};
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (int i = 0; i < limit1; i++) {
        array1[i] = i * 2;
        sum1 += array1[i];
        
        /* Create side effect to prevent dead code elimination */
        asm volatile("" : "+r"(array1[i]) : : "memory");
    }
    
    /* Loop 2: Adjacent but disjoint loop - different basic blocks */
    /* This should make bitmap_intersect_p return false initially */
    int j = 0;
    do {
        array2[j] = j * 3;
        sum2 += array2[j];
        asm volatile("" : "+r"(array2[j]) : : "memory");
        j++;
    } while (j < limit2);
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* Loop 3's blocks should be a subset of Loop 4's blocks */
    for (int outer = 0; outer < limit3; outer++) {
        /* Loop 4: Outer loop containing Loop 3 */
        for (int inner = 0; inner < 5; inner++) {
            array3[outer * 5 + inner] = outer + inner;
            sum3 += array3[outer * 5 + inner];
            asm volatile("" : "+r"(array3[outer * 5 + inner]) : : "memory");
        }
    }
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create complex CFG with goto to achieve partial overlap */
    int x = 0;
    int shared_var = 0;
    
    /* Loop 5 */
    for (x = 0; x < limit1; x++) {
        if (x % 3 == 0) {
            /* This conditional jump creates shared basic blocks */
            goto shared_block;
        }
        array1[x] += x;
        asm volatile("" : "+r"(array1[x]) : : "memory");
        
        shared_block:
        /* Shared block between Loop 5 and Loop 6 */
        shared_var += x;
        asm volatile("" : "+r"(shared_var) : : "memory");
        
        /* Continue with Loop 5 */
        if (x % 2 == 0) {
            continue;
        }
    }
    
    /* Loop 6: Partially overlaps with Loop 5 via shared_block */
    int y = limit1 / 2;
    while (y < limit1) {
        /* Jump to the shared block from Loop 5 */
        if (y % 4 == 0) {
            goto shared_block_2;
        }
        
        array2[y] -= y;
        asm volatile("" : "+r"(array2[y]) : : "memory");
        y++;
        
        shared_block_2:
        /* Another shared block */
        shared_var -= y;
        asm volatile("" : "+r"(shared_var) : : "memory");
        
        /* Conditional continue to create more CFG complexity */
        if (y % 5 != 0) {
            continue;
        }
    }
    
    /* Loop 7: Another loop that shares exit blocks with Loop 8 */
    int z = 0;
    int temp_sum = 0;
    
    /* Loop 7 */
    for (z = 0; z < 10; z++) {
        temp_sum += z;
        if (z == 5) {
            /* Jump to a block that's also in Loop 8's execution path */
            goto exit_shared;
        }
    }
    
    /* Loop 8: Shares exit block with Loop 7 */
    int w = 0;
    while (w < 8) {
        temp_sum -= w;
        w++;
    }
    
    exit_shared:
    /* Shared exit block */
    temp_sum *= 2;
    asm volatile("" : "+r"(temp_sum) : : "memory");
    
    /* Force use of all computed values to prevent optimization */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3), 
                  "r"(shared_var), "r"(temp_sum) : "memory");
}

/* Main function to drive execution */
int main() {
    /* Use volatile to prevent compile-time optimization */
    volatile int N = 100;
    volatile int M = 50;
    volatile int K = 20;
    
    /* Call the function with hardware loop patterns */
    test_loop_patterns(N, M, K);
    
    /* Additional test with different parameters */
    test_loop_patterns(75, 25, 15);
    
    printf("Hardware loop pattern test completed\n");
    
    return 0;
}
