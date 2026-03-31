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
void test_loop_patterns(int n, int *results) {
    volatile int limit = n;  /* Prevent constant propagation */
    int i, j, k;
    
    /* Loop 1: Simple countable loop (disjoint from others initially) */
    int sum1 = 0;
    for (i = 0; i < limit; i++) {
        /* Simple body with side effect */
        sum1 += i * 2;
        asm volatile("" : : : "memory");  /* Prevent optimization */
    }
    results[0] = sum1;
    
    /* Loop 2: Perfectly nested within Loop 3 */
    int sum2 = 0;
    volatile int inner_limit = limit / 2;
    
    /* Loop 3: Outer loop containing Loop 2 */
    for (j = 0; j < limit; j++) {
        /* Loop 2: Inner loop (perfectly nested) */
        for (k = 0; k < inner_limit; k++) {
            sum2 += j * k;
            asm volatile("" : : : "memory");
        }
        
        /* Additional computation in outer loop */
        sum2 += j;
        asm volatile("" : : : "memory");
    }
    results[1] = sum2;
    
    /* Loop 4 and 5: Partially overlapping loops using goto */
    int sum4 = 0, sum5 = 0;
    volatile int limit4 = limit;
    volatile int limit5 = limit / 3;
    
    /* Label for partial overlap */
    overlap_point:
    
    /* Loop 4: First loop in partial overlap pair */
    for (i = 0; i < limit4; i++) {
        sum4 += i * 3;
        asm volatile("" : : : "memory");
        
        /* Conditional that may jump into Loop 5 */
        if (i == limit4 / 2) {
            /* This creates partial overlap in CFG */
            goto inside_loop5;
        }
        
        /* Normal continuation */
        sum4 += 1;
    }
    
    /* Loop 5: Second loop in partial overlap pair */
    int loop5_counter = 0;
    while (loop5_counter < limit5) {
        inside_loop5:
        sum5 += loop5_counter * 4;
        asm volatile("" : : : "memory");
        
        /* Conditional jump back to Loop 4's region */
        if (loop5_counter == limit5 / 2) {
            goto after_loop4;
        }
        
        loop5_counter++;
    }
    
    after_loop4:
    results[2] = sum4;
    results[3] = sum5;
    
    /* Loop 6: Adjacent but disjoint do-while loop */
    int sum6 = 0;
    int counter = 0;
    volatile int limit6 = limit;
    
    do {
        sum6 += counter * 5;
        asm volatile("" : : : "memory");
        counter++;
    } while (counter < limit6);
    
    results[4] = sum6;
    
    /* Loop 7: Complex nesting with conditional inner loop */
    int sum7 = 0;
    volatile int outer_limit = limit / 4;
    volatile int inner_cond_limit = limit / 5;
    
    for (i = 0; i < outer_limit; i++) {
        sum7 += i;
        
        /* Conditional inner loop - not always executed */
        if (i % 2 == 0) {
            for (j = 0; j < inner_cond_limit; j++) {
                sum7 += i * j;
                asm volatile("" : : : "memory");
            }
        } else {
            sum7 += i * 2;
        }
        
        asm volatile("" : : : "memory");
    }
    
    results[5] = sum7;
}

/* Main function to drive execution */
int main() {
    volatile int N = 100;  /* Runtime value */
    int results[10] = {0};
    
    /* Call the function with all loop patterns */
    test_loop_patterns(N, results);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        checksum += results[i];
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    if (checksum > 0) {
        return 0;
    } else {
        return 1;
    }
}
