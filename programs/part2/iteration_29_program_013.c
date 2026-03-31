/* test_hw_loops.c
 * This program creates nested loop structures to trigger hardware loop
 * optimization analysis, specifically targeting bitmap intersection
 * and subset relationship checks in hw-doloop.cc.
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int sink;
volatile int arr[100];

/* Mark function to prevent inlining and ensure all loops are analyzed together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    /* Three perfectly nested loops - creates clear parent-child relationships */
    for (int i = 0; i < 10; ++i) {          /* Outer loop L1 */
        for (int j = 0; j < 8; ++j) {       /* Middle loop L2 */
            for (int k = 0; k < 6; ++k) {   /* Inner loop L3 */
                /* Simple body to create basic blocks */
                sink = i * j * k;
                arr[k] = sink;
            }
            /* Additional statement in middle loop body */
            sink = j * 2;
        }
        
        /* Partially overlapping loop - shares some blocks with L1 but not perfectly nested */
        /* This starts in L1's body but continues differently */
        for (int m = 0; m < 5; ++m) {       /* Overlapping loop L4 */
            /* This loop shares the outer i loop's scope but has different structure */
            sink = i * m + 100;
            arr[m + 10] = sink;
            
            /* Small conditional to create additional basic blocks */
            if (m % 2 == 0) {
                sink = m * 2;
            }
        }
    }
    
    /* Separate non-intersecting loop - should not intersect with any above */
    for (int n = 0; n < 7; ++n) {           /* Isolated loop L5 */
        sink = n * 3;
        arr[n + 20] = sink;
    }
    
    /* Another set of nested loops at same level */
    for (int a = 0; a < 5; ++a) {           /* Loop L6 */
        for (int b = 0; b < 4; ++b) {       /* Loop L7 (child of L6) */
            sink = a + b;
            arr[b + 30] = sink;
        }
    }
}

/* Alternative implementation for architectures without hardware loop support */
__attribute__((noinline, noipa))
void test_nested_loops_fallback(void) {
    /* Simpler version for generic targets */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 4; ++j) {
            sink = i * j;
        }
    }
}

int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        arr[i] = 0;
    }
    
    /* Call the loop test function */
#ifdef __riscv__
    /* For RISC-V with Ziloop extension */
    test_nested_loops();
#elif defined(__ARC__) || defined(__arc__)
    /* For ARC with hardware loops */
    test_nested_loops();
#elif defined(__XTENSA__) || defined(__dsp__) || defined(__hwloop__)
    /* For other targets with hardware loop support */
    test_nested_loops();
#else
    /* Generic fallback */
    test_nested_loops_fallback();
#endif
    
    /* Use result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 20; ++i) {
        sum += arr[i];
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
