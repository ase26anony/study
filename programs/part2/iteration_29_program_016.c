/* test_hw_loops.c - Test program for hardware loop bitmap intersection analysis */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure loops stay together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int sink_local = 0;
    
    /* 
     * Three perfectly nested loops - creates clear subset relationships:
     * Loop k's blocks ⊆ Loop j's blocks ⊆ Loop i's blocks
     */
    for (int i = 0; i < 10; ++i) {
        /* Basic block A - start of outer loop */
        sink_local += i;
        
        for (int j = 0; j < 8; ++j) {
            /* Basic block B - middle loop */
            sink_local += j;
            
            for (int k = 0; k < 6; ++k) {
                /* Basic block C - innermost loop */
                sink_local += k;
                g_array[i * 8 + j] = sink_local; /* Prevent dead code elimination */
            }
            
            /* Basic block D - after inner loop, still in middle loop */
            sink_local *= 2;
        }
        
        /* 
         * PARTIALLY OVERLAPPING LOOP - shares some blocks with outer loop
         * This loop starts in the outer loop's body but continues after 
         * the j loop ends, creating partial overlap.
         * It will intersect bitmap with outer loop but not be a subset.
         */
        for (int m = 0; m < 5; ++m) {
            /* Basic block E - partially overlapping loop body */
            sink_local -= m;
            g_array[i] = sink_local; /* Uses outer loop's i variable */
        }
        /* Basic block F - still in outer loop after partial loop */
        sink_local = sink_local > 0 ? sink_local : -sink_local;
    }
    
    /* 
     * SEPARATE NON-INTERSECTING LOOP 
     * This loop doesn't share any basic blocks with the nested loops above.
     * It ensures we have multiple loops to analyze.
     */
    for (int x = 0; x < 7; ++x) {
        /* Basic block G - completely separate loop */
        g_sink += x * x;
    }
    
    /* One more simple loop at same nesting level */
    for (int y = 0; y < 3; ++y) {
        /* Basic block H - another separate loop */
        g_sink -= y;
    }
}

/* Alternative version with even clearer block structure */
__attribute__((noinline, noipa))
void test_alternative_nesting(void) {
    volatile int counter = 0;
    
    /* Outer loop L1 */
    for (int a = 0; a < 5; ++a) {
        counter += a;
        
        /* Inner loop L2 - proper subset of L1 */
        for (int b = 0; b < 4; ++b) {
            counter += b;
            
            /* Innermost loop L3 - proper subset of L2 */
            for (int c = 0; c < 3; ++c) {
                counter += c;
                g_array[a * 16 + b * 4 + c] = counter;
            }
        }
        
        /* 
         * Loop L4: Partially overlaps with L1
         * Starts in L1's body, shares some blocks but not all
         * bitmap_intersect_p will be true
         * bitmap_intersect_compl_p(L4, L1) will be true (L4 has blocks not in L1)
         * bitmap_intersect_compl_p(L1, L4) will be true (L1 has blocks not in L4)
         * So neither is subset of the other -> continue
         */
        for (int d = 0; d < 2; ++d) {
            counter -= d;
            g_sink = counter;
        }
    }
}

int main(void) {
    /* Initialize to prevent constant propagation */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the test functions */
    test_nested_loops();
    test_alternative_nesting();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (sink: %d)\n", sum, g_sink);
    return 0;
}
