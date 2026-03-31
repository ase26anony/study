/* hwloop_test.c - Test program for hardware loop nesting analysis */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure loops stay together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * First: Three perfectly nested loops (i, j, k)
     * This creates clear subset relationships:
     * - k-loop blocks ⊆ j-loop blocks ⊆ i-loop blocks
     */
    for (int i = 0; i < 5; ++i) {
        /* Outer loop body start */
        g_array[i] = i * 2;
        
        for (int j = 0; j < 4; ++j) {
            /* Middle loop body start */
            local_sink = i + j;
            
            for (int k = 0; k < 3; ++k) {
                /* Innermost loop - simple operation */
                g_sink = i * j * k;
                g_array[k] += g_sink;
            }
            
            /* Middle loop body end */
            g_sink += j;
        }
        
        /* 
         * Second: Partially overlapping loop with outer i-loop
         * This loop starts inside the i-loop body but continues
         * beyond the j-loop, creating partial block overlap.
         * It will intersect but not be a subset.
         */
        for (int m = 0; m < 3; ++m) {
            /* This loop shares the i-loop's blocks but has its own blocks too */
            g_array[i + 10] += m;
            local_sink = i * m;
        }
        /* Outer loop body end */
        g_sink += i;
    }
    
    /*
     * Third: Separate loop that doesn't intersect with the nested structure
     * This ensures we have a loop that fails the first bitmap_intersect_p check
     */
    for (int x = 0; x < 10; ++x) {
        g_array[x + 20] = x * x;
        local_sink = x;
    }
    
    /*
     * Fourth: Another partially overlapping structure
     * Create a situation where loops share some blocks but aren't nested
     */
    for (int a = 0; a < 6; ++a) {
        g_sink = a;
        
        /* Inner loop that's a proper subset */
        for (int b = 0; b < 4; ++b) {
            g_array[b] += a + b;
        }
        
        /* Another loop at same level as b-loop but not nested in it */
        for (int c = 0; c < 3; ++c) {
            g_sink += c;
            /* This creates intersection with a-loop but not subset relationship */
        }
    }
}

/* Alternative implementation with even clearer block relationships */
__attribute__((noinline, noipa))
void test_clear_nesting(void) {
    volatile int sink = 0;
    
    /* Loop A - outermost */
    for (int a = 0; a < 8; a++) {
        sink = a;
        
        /* Loop B - nested in A (B ⊆ A) */
        for (int b = 0; b < 6; b++) {
            g_array[b] = a + b;
            
            /* Loop C - nested in B (C ⊆ B ⊆ A) */
            for (int c = 0; c < 4; c++) {
                sink = a * b * c;
            }
        }
        
        /* Loop D - shares blocks with A but not nested (D ∩ A ≠ ∅, D ⊄ A, A ⊄ D) */
        /* Starts in A's body but has different continuation */
        for (int d = 0; d < 5; d++) {
            g_sink = a * d;
            if (d % 2) {
                sink += d;  /* Creates additional basic block */
            }
        }
    }
    
    /* Loop E - completely separate (E ∩ A = ∅) */
    for (int e = 0; e < 7; e++) {
        g_array[e + 30] = e;
    }
}

int main(void) {
    /* Initialize to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        g_array[i] = i;
    }
    
    /* Call both test functions to increase coverage chances */
    test_nested_loops();
    test_clear_nesting();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        sum += g_array[i];
    }
    
    printf("Result: %d (g_sink: %d)\n", sum, g_sink);
    return sum > 0 ? 0 : 1;
}
