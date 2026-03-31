/* test_hw_loops.c
 * Designed to trigger hardware loop optimization bitmap intersection analysis
 * Compile with: -O2 -fhwloops -fno-unroll-loops (on supported targets)
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure all loops are analyzed together */
__attribute__((noinline, noipa))
static void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* Three perfectly nested loops - creates clear subset relationships */
    for (int i = 0; i < 10; ++i) {           /* Outer loop - largest block set */
        for (int j = 0; j < 8; ++j) {        /* Middle loop - subset of outer */
            for (int k = 0; k < 6; ++k) {    /* Innermost loop - subset of both */
                /* Simple operation to create basic blocks */
                local_sink = i * j + k;
                g_array[(i * 8 + j) * 6 + k] = local_sink;
            }
        }
        
        /* PARTIALLY OVERLAPPING LOOP - shares blocks with outer loop 
         * but not perfectly nested. This triggers bitmap_intersect_p = true
         * but subset checks fail (bitmap_intersect_compl_p = true for both)
         */
        for (int m = 0; m < 5; ++m) {
            /* This loop starts in outer loop's body but continues 
             * independently - creates partial overlap */
            local_sink = i * m;
            g_array[i * 5 + m] = local_sink + 1000;
        }
    }
    
    /* Separate non-intersecting loop - provides variety in analysis */
    for (int x = 0; x < 7; ++x) {
        local_sink = x * 2;
        g_array[50 + x] = local_sink;
    }
    
    /* Another set of nested loops at different depth */
    for (int a = 0; a < 5; ++a) {
        for (int b = 0; b < 4; ++b) {
            local_sink = a - b;
            g_array[60 + a * 4 + b] = local_sink;
        }
    }
    
    g_sink = local_sink;
}

/* Main function with architecture detection */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        g_array[i] = i;
    }
    
    /* Call the loop-intensive function */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += g_array[i];
    }
    
    printf("Result: %d (sink: %d)\n", sum, g_sink);
    return sum > 0 ? 0 : 1;
}

/* Architecture-specific compilation guidance */
#ifdef __riscv__
/* RISC-V with Ziloop extension */
__attribute__((target("arch=rv64gc_ziloop")))
void riscv_hwloop_hint(void) {
    /* Empty function to suggest Ziloop extension */
}
#endif

#ifdef __ARC__
/* ARC with loop extension */
__attribute__((target("mloop")))
void arc_hwloop_hint(void) {
    /* Empty function to suggest loop extension */
}
#endif
