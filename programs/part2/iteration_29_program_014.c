/* test_hw_loops.c - Test program for hardware loop nesting analysis */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and ensure loops remain distinct */
volatile int g_sink = 0;
int g_array[100] = {0};

/* Mark function to prevent inlining and ensure all loops are analyzed together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * Three perfectly nested loops - creates clear parent-child relationships.
     * Loop A (outer): blocks = {A_header, A_body, A_latch}
     * Loop B (middle): blocks = {B_header, B_body, B_latch} ⊆ A_body
     * Loop C (inner): blocks = {C_header, C_body, C_latch} ⊆ B_body
     */
    for (int i = 0; i < 10; ++i) {          /* Loop A - outermost */
        for (int j = 0; j < 8; ++j) {       /* Loop B - middle */
            for (int k = 0; k < 6; ++k) {   /* Loop C - innermost */
                /* Simple body to create basic blocks */
                local_sink = i * j * k;
                g_array[(i * 8 + j) % 100] += k;
            }
        }
        
        /*
         * Loop D - Partially overlaps with Loop A but not perfectly nested.
         * Shares some blocks with Loop A but has additional blocks.
         * This triggers bitmap_intersect_p() = true, but
         * bitmap_intersect_compl_p() in both directions will be false,
         * causing the 'continue' path to be taken.
         */
        for (int m = 0; m < 5; ++m) {       /* Loop D - intersecting */
            local_sink = i * m;
            g_array[(i * 5 + m) % 100] -= 1;
        }
    }
    
    /*
     * Loop E - Separate loop that doesn't intersect with any other loop.
     * This provides variety in the loop analysis.
     */
    for (int n = 0; n < 12; ++n) {          /* Loop E - separate */
        local_sink = n * 2;
        g_array[n % 100] = n;
    }
    
    /* Additional complexity to prevent over-simplification */
    for (int p = 0; p < 7; ++p) {           /* Loop F - another separate loop */
        for (int q = 0; q < 3; ++q) {       /* Loop G - nested in F */
            local_sink = p * q;
            g_array[(p * 3 + q) % 100] += p;
        }
    }
    
    g_sink = local_sink;
}

/* Main function with side effects to prevent dead code elimination */
int main(void) {
    printf("Starting hardware loop test...\n");
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the function containing all loop structures */
    test_nested_loops();
    
    /* Create observable side effect */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (g_sink = %d)\n", sum, g_sink);
    return sum > 0 ? 0 : 1;
}
