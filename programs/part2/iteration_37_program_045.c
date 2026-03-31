/* test_hw_doloop.c
 * Designed to trigger uncovered lines in hw-doloop.cc (lines 429-436)
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture target for ARM hardware loop support */
#ifdef __GNUC__
__attribute__((target("arch=armv8-a")))
#endif
void test_loop_patterns(volatile int N, int* results) {
    volatile int limit = N;  /* Prevent constant propagation */
    int i, j, k;
    
    /* Array for dummy computations */
    int array[1000];
    for (int idx = 0; idx < 1000; idx++) array[idx] = idx;
    
    /* LOOP 1: Simple countable loop - will be analyzed by hw-doloop */
    int sum1 = 0;
    for (i = 0; i < limit; i++) {
        /* Simple body with side effect */
        sum1 += array[i % 1000];
        asm volatile("" : : : "memory");  /* Prevent optimization */
    }
    results[0] = sum1;
    
    /* LOOP 2: Adjacent but disjoint loop - different basic blocks */
    int sum2 = 0;
    for (j = 0; j < limit/2; j++) {
        sum2 += array[(j * 2) % 1000];
        asm volatile("" : : : "memory");
    }
    results[1] = sum2;
    
    /* LOOP 3: Perfectly nested inside conditional - creates hierarchy */
    int sum3 = 0;
    if (limit > 10) {
        /* LOOP 3a: Inner loop that will be nested inside the if's blocks */
        for (k = 0; k < limit/3; k++) {
            sum3 += array[(k * 3) % 1000];
            asm volatile("" : : : "memory");
        }
    }
    results[2] = sum3;
    
    /* LOOP 4: do-while loop for CFG variation */
    int sum4 = 0;
    int cnt = 0;
    do {
        sum4 += array[cnt % 1000];
        asm volatile("" : : : "memory");
        cnt++;
    } while (cnt < limit/4);
    results[3] = sum4;
    
    /* CRITICAL SECTION: Create partial overlap between loops 5 and 6 */
    int sum5 = 0, sum6 = 0;
    int shared_counter = 0;
    
    /* LOOP 5: First loop with conditional branch to LOOP 6's blocks */
    for (i = 0; i < limit; i++) {
        sum5 += array[i % 1000];
        asm volatile("" : : : "memory");
        
        /* Conditional that creates partial overlap */
        if (i % 3 == 0 && i < limit - 5) {
            /* Jump to LOOP 6's processing block */
            goto process_in_both_loops;
        }
        
        continue_loop5:
        /* More loop 5 specific code */
        sum5 += 1;
    }
    
    /* LOOP 6: Second loop that shares blocks with LOOP 5 */
    for (j = 5; j < limit - 5; j++) {
        sum6 += array[j % 1000];
        asm volatile("" : : : "memory");
        
        process_in_both_loops:
        /* This block belongs to BOTH loops 5 and 6 */
        shared_counter++;
        array[shared_counter % 1000] = (sum5 + sum6) % 256;
        
        if (j > 10) {
            /* Jump back to loop 5 if we came from there */
            goto continue_loop5;
        }
        
        /* More loop 6 specific code */
        sum6 += 2;
    }
    
    results[4] = sum5;
    results[5] = sum6;
    results[6] = shared_counter;
    
    /* LOOP 7: Another loop to ensure multiple loop analysis */
    int sum7 = 0;
    for (i = 0; i < limit/2 + 5; i++) {
        /* Complex enough to not be optimized away */
        sum7 = (sum7 * 31 + array[i % 1000]) & 0xFFFF;
        asm volatile("" : : : "memory");
    }
    results[7] = sum7;
}

/* Main function to drive execution */
int main() {
    volatile int N = 100;  /* Volatile to prevent compile-time evaluation */
    int results[10] = {0};
    
    /* Call the function with carefully constructed loops */
    test_loop_patterns(N, results);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFFFF;
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Use results to prevent dead code elimination */
    if (checksum > 1000) {
        return 0;
    } else {
        return 1;
    }
}
