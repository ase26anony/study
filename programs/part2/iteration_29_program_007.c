/* test_hw_loops.c
 * Designed to trigger hardware loop nesting analysis in hw-doloop.cc
 * Specifically targets lines 429-436:
 *   bitmap_intersect_p and bitmap_intersect_compl_p checks
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int sink;
volatile int array[100];

/* Force function to not be inlined or optimized away */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    /* First: Three perfectly nested loops - creates clear parent-child relationships */
    for (int i = 0; i < 5; ++i) {          /* Outer loop L1 */
        for (int j = 0; j < 4; ++j) {      /* Middle loop L2 */
            for (int k = 0; k < 3; ++k) {  /* Inner loop L3 */
                /* Simple body to create basic blocks */
                sink = i * j + k;
                array[i * 16 + j * 4 + k] = sink;
            }
        }
        
        /* Second: Partially overlapping loop with L1 
         * This starts inside L1's body but continues after L2 ends
         * Creates intersection but not subset relationship */
        for (int m = 0; m < 3; ++m) {      /* Loop L4 - intersects with L1 but not nested */
            sink = i * m + 2;
            array[i * 10 + m + 50] = sink;
        }
    }
    
    /* Third: Separate loop that doesn't intersect with the nested structure
     * This ensures we have a loop that fails the first bitmap_intersect_p check */
    for (int n = 0; n < 6; ++n) {          /* Loop L5 - no intersection */
        sink = n * 2;
        array[n + 80] = sink;
    }
    
    /* Fourth: Another partially overlapping structure
     * Creates more complex intersection patterns */
    for (int p = 0; p < 4; ++p) {          /* Loop L6 */
        sink = p * 3;
        array[p + 90] = sink;
        
        /* Small inner loop that's a proper subset of L6 */
        for (int q = 0; q < 2; ++q) {      /* Loop L7 - child of L6 */
            sink = p * q + 1;
            array[p * 2 + q + 70] = sink;
        }
    }
}

/* Alternative version with different nesting pattern */
__attribute__((noinline, noipa))
void test_alternate_nesting(void) {
    /* Create a diamond-shaped control flow with loops */
    for (int a = 0; a < 4; ++a) {          /* Loop A */
        if (a % 2 == 0) {
            for (int b = 0; b < 3; ++b) {  /* Loop B - child of A */
                sink = a * b;
                array[a * 3 + b] = sink;
            }
        } else {
            for (int c = 0; c < 2; ++c) {  /* Loop C - also child of A, disjoint from B */
                sink = a + c * 10;
                array[a * 2 + c + 20] = sink;
            }
        }
        
        /* Loop that shares some blocks with A but extends beyond */
        for (int d = 0; d < a + 1; ++d) {  /* Loop D - intersects with A, not subset */
            sink = a * d * 2;
            array[a * 4 + d + 40] = sink;
        }
    }
}

int main(void) {
    /* Initialize array to prevent undefined behavior */
    for (int i = 0; i < 100; ++i) {
        array[i] = 0;
    }
    
    /* Call both test functions to create multiple loop structures */
    test_nested_loops();
    test_alternate_nesting();
    
    /* Create observable side effect */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += array[i];
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
