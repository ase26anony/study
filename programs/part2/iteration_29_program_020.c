/* test_hw_loops.c - Target hardware loop nesting analysis */

/* Prevent unwanted optimizations */
#pragma GCC optimize("no-unroll-loops")
#pragma GCC optimize("no-peel-loops")

/* Global volatile variables to prevent dead code elimination */
volatile int global_sink = 0;
volatile int results[100] = {0};

/* Force this function to stay intact for loop analysis */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int sink;
    
    /* 
     * Three perfectly nested loops - creates clear parent-child relationships.
     * Loop A (outer): blocks = {A_header, A_body, A_latch}
     * Loop B (middle): blocks = {B_header, B_body, B_latch} ⊆ A_body
     * Loop C (inner): blocks = {C_header, C_body, C_latch} ⊆ B_body
     */
    for (int i = 0; i < 10; ++i) {          /* Loop A - outer */
        for (int j = 0; j < 8; ++j) {       /* Loop B - middle */
            for (int k = 0; k < 6; ++k) {   /* Loop C - inner */
                /* Simple body - creates basic blocks */
                sink = i * j + k;
                results[k] = sink;
            }
            /* Middle loop body continuation */
            sink = j * 2;
            results[j + 10] = sink;
        }
        /* Outer loop body continuation */
        sink = i * 3;
        results[i + 20] = sink;
        
        /*
         * Loop D: Partially overlaps with Loop A but not perfectly nested.
         * Starts inside Loop A's body but continues after Loop B ends.
         * Blocks intersect with Loop A but are not a subset.
         * This triggers: bitmap_intersect_p = true
         * but both subset checks fail -> continue
         */
        for (int d = 0; d < 5; ++d) {       /* Loop D - intersecting */
            sink = i * d + 100;
            results[d + 30] = sink;
        }
    }
    
    /* 
     * Loop E: Separate loop that doesn't intersect with any other.
     * Provides variety for the analysis.
     */
    for (int e = 0; e < 7; ++e) {           /* Loop E - separate */
        sink = e * 7;
        results[e + 40] = sink;
    }
    
    /*
     * Additional complexity: Loop F that shares some blocks with
     * the outer loop structure but has different control flow.
     */
    for (int i = 0; i < 10; ++i) {          /* Loop F - uses same var */
        if (i % 2 == 0) {
            for (int j = 0; j < 3; ++j) {   /* Loop G - conditional inner */
                sink = i * j * 2;
                results[j + 50] = sink;
            }
        } else {
            sink = i * 5;
            results[i + 60] = sink;
        }
    }
}

/* Main function with side effects */
int main(void) {
    /* Initialize */
    for (int i = 0; i < 100; ++i) {
        results[i] = i;
    }
    
    /* Call the loop-heavy function */
    test_nested_loops();
    
    /* Create observable side effect */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += results[i];
    }
    
    global_sink = sum;
    
    /* Print to prevent optimization */
    volatile int output = global_sink;
    
    return output % 256;
}

/* 
 * Compilation recommendations for specific targets:
 * 
 * For RISC-V with Ziloop extension:
 *   riscv64-unknown-elf-gcc -O2 -march=rv64gc_ziloop -fhwloops \
 *     -fno-unroll-loops -funroll-all-loops \
 *     -fdump-rtl-hwloops -fdump-rtl-loop2 \
 *     -S test_hw_loops.c -o test_hw_loops.s
 * 
 * For ARC targets:
 *   arc-elf-gcc -O2 -mloop -ffunction-sections \
 *     -fno-unroll-loops -fdump-tree-hwloops \
 *     test_hw_loops.c -o test_hw_loops.elf
 * 
 * For generic testing (if hardware loops supported):
 *   gcc -O3 -fhwloops -fno-unroll-loops \
 *     -fdump-rtl-all -fdump-rtl-pro_and_epilogue \
 *     test_hw_loops.c -o test_hw_loops
 */
