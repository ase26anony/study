/* test_hw_loops.c - Target coverage for hw-doloop.cc lines 429-436 */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(int n) {
    volatile int limit = n;  /* Prevent constant propagation */
    int array1[1000], array2[1000], array3[1000];
    int i, j, k;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Initialize arrays with volatile to prevent optimization */
    volatile int init = 1;
    for (i = 0; i < 1000; i++) {
        array1[i] = init + i;
        array2[i] = init + i * 2;
        array3[i] = init + i * 3;
    }
    
    /* Loop 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < limit; i++) {
        sum1 += array1[i];
        /* Memory clobber to prevent dead code elimination */
        asm volatile("" : : : "memory");
    }
    
    /* Loop 2: Adjacent but disjoint loop (no block intersection with Loop 1) */
    for (j = 0; j < limit; j++) {
        sum2 += array2[j];
        asm volatile("" : : : "memory");
    }
    
    /* Loop 3: Perfectly nested inside Loop 4 */
    /* First, create outer loop (Loop 4) */
    k = 0;
    do {
        /* Loop 3: Inner loop - entirely contained within Loop 4 */
        for (i = 0; i < 5; i++) {
            sum3 += array3[k] + i;
            asm volatile("" : : : "memory");
        }
        k++;
    } while (k < limit);
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* Create complex CFG with conditional jumps between loops */
    int x = 0, y = 0;
    int flag = limit % 2;
    
    /* Loop 5 */
    for (i = 0; i < limit; i++) {
        if (flag) {
            /* This creates a basic block inside Loop 5 */
            sum1 += array1[i] * 2;
            /* Jump to label inside Loop 6 - creates partial overlap */
            goto overlap_point;
        } else {
            sum1 += array1[i];
        }
        asm volatile("" : : : "memory");
        
        /* Normal continuation of Loop 5 */
        continue;
        
    overlap_point:
        /* This label is inside both Loop 5 and Loop 6 */
        /* Loop 6 starts here - creates partial overlap */
        for (j = 0; j < 3; j++) {
            sum2 += array2[j] + i;
            asm volatile("" : : : "memory");
            /* Jump back to Loop 5 - creates the overlap */
            if (j == 1) goto back_to_loop5;
        }
        
        /* Continue in Loop 6 if not jumping back */
        y++;
        if (y >= limit) break;
        continue;
        
    back_to_loop5:
        /* Back to Loop 5 */
        x++;
        if (x >= limit) break;
    }
    
    /* Loop 7: Another loop that shares some blocks with Loop 8 via switch */
    int mode = 0;
    for (i = 0; i < limit; i++) {
        switch (mode) {
            case 0:
                sum3 += array3[i];
                mode = 1;
                break;
            case 1:
                /* Jump to shared block with Loop 8 */
                goto shared_block;
            default:
                sum3 += i;
                break;
        }
        asm volatile("" : : : "memory");
        
        if (i % 2 == 0) {
            mode = 0;
        }
        continue;
        
    shared_block:
        /* This block is shared between Loop 7 and Loop 8 */
        sum1 += sum2;
        asm volatile("" : : : "memory");
        
        /* Loop 8: Overlaps with Loop 7 through shared_block */
        for (j = i; j < limit && j < i + 3; j++) {
            sum2 += array2[j] * array1[j];
            asm volatile("" : : : "memory");
        }
        
        /* Return to Loop 7 */
        if (i < limit - 1) {
            i++;
            continue;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int result = sum1 + sum2 + sum3;
    (void)result;
}

/* Main function to drive execution */
int main(int argc, char **argv) {
    int N = 100;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N > 1000) N = 1000;
        if (N < 10) N = 10;
    }
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N);
    
    /* Compute and print a checksum to ensure execution */
    printf("HW Loop Pattern Test Complete (N=%d)\n", N);
    
    return 0;
}
