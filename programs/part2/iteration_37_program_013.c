/* test_hw_doloop.c
 * Designed to trigger specific uncovered lines in GCC's hw-doloop.cc
 * Compile with: gcc -O2 -march=armv8-a -fmodulo-sched -c test_hw_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force architecture support for hardware loops */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#endif
__attribute__((noinline, hot))
void test_loop_patterns(int n, int *results) {
    volatile int limit = n;  /* Prevent constant propagation */
    int i, j, k;
    
    /* Array to ensure side effects */
    volatile int arr[100] = {0};
    
    /* LOOP 1: Simple countable loop - will be analyzed by hw-doloop */
    for (i = 0; i < limit; i++) {
        arr[i % 100] += i;
        results[0] += i;
    }
    
    /* LOOP 2: Adjacent but disjoint loop (no block intersection with Loop 1) */
    for (j = 0; j < limit/2; j++) {
        arr[(j + 50) % 100] -= j;
        results[1] -= j;
    }
    
    /* LOOP 3: Perfectly nested inside conditional structure */
    /* This creates hierarchical relationship where Loop 3 is inside Loop 4 */
    k = 0;
    do {
        /* LOOP 4: Outer loop containing Loop 3 */
        for (i = 0; i < limit/3; i++) {
            /* LOOP 3: Inner loop - perfectly nested */
            for (j = 0; j < 5; j++) {
                arr[(i + j) % 100] += (i * j);
                results[2] += (i * j);
            }
            /* Force side effect to prevent optimization */
            asm volatile("" : : : "memory");
        }
        k++;
    } while (k < 2);
    
    /* LOOP 5 and LOOP 6: Partially overlapping loops using goto */
    /* This creates the partial overlap scenario for bitmap_intersect_compl_p */
    int x = 0;
    
    /* LOOP 5 */
    for (i = 0; i < limit; i++) {
        arr[i % 100] += i * 2;
        results[3] += i * 2;
        
        /* Conditional that can jump into LOOP 6 */
        if (i == limit/2) {
            /* Jump to label inside LOOP 6 */
            goto overlap_point;
        }
        
        /* Normal loop 5 continuation */
        arr[(i + 1) % 100] -= i;
        results[3] -= i;
    }
    
    /* LOOP 6 - partially overlaps with LOOP 5 via goto */
    for (j = 0; j < limit; j++) {
        overlap_point:  /* Label that LOOP 5 can jump to */
        arr[j % 100] += j * 3;
        results[4] += j * 3;
        
        /* Prevent tail merging optimization */
        if (j % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* LOOP 7: Another loop with complex control flow for additional coverage */
    int m = 0;
    int done = 0;
    while (!done) {
        /* Multiple basic blocks within the loop */
        if (m < limit/4) {
            arr[m % 100] += m * 4;
            results[5] += m * 4;
            m++;
        } else {
            /* Different basic block in same loop */
            arr[(m + 10) % 100] -= m;
            results[5] -= m;
            if (m >= limit/2) {
                done = 1;
            }
            m += 2;
        }
        
        /* Force memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
    }
}

/* Additional test function to create more complex CFG relationships */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv8-a")))
#endif
__attribute__((noinline))
void nested_loop_complex(int n, int *res) {
    volatile int v = n;
    int a[100] = {0};
    int i, j, k;
    
    /* Create multiple nested loops with shared blocks */
    
    /* Outer loop A */
    for (i = 0; i < v; i++) {
        /* Middle loop B - partially overlaps with C */
        for (j = 0; j < v/2; j++) {
            a[(i + j) % 100] += 1;
            res[0] += 1;
            
            /* Conditional jump that creates partial overlap */
            if (j == v/4) {
                /* This creates a shared basic block with loop C */
                goto shared_block;
            }
            
            a[(i + j + 1) % 100] -= 1;
            res[0] -= 1;
        }
        
        /* Loop C - shares block via goto */
        for (k = 0; k < v/3; k++) {
            shared_block:  /* Shared label */
            a[(i + k) % 100] += 2;
            res[1] += 2;
            
            /* Differentiate from loop B's block */
            if (k % 3 == 0) {
                a[(i + k + 5) % 100] -= 1;
                res[1] -= 1;
            }
        }
    }
}

int main() {
    int n = 100;  /* Runtime value to prevent constant folding */
    int results[10] = {0};
    int results2[10] = {0};
    
    /* Call the test function multiple times to ensure execution */
    test_loop_patterns(n, results);
    nested_loop_complex(n/2, results2);
    
    /* Compute checksum to ensure all loops executed */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += results[i] + results2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Use results to prevent dead code elimination */
    if (checksum > 1000000) {
        printf("Unexpectedly large checksum\n");
    }
    
    return 0;
}
