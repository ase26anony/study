/* test_hw_loops.c
 * This program creates specific loop nesting patterns to trigger
 * hardware loop optimization analysis, particularly the bitmap
 * intersection and subset checks in hw-doloop.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function as noinline to ensure loops are analyzed together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int sink;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear parent-child relationships
     * Outer loop (i) contains middle loop (j)
     * Middle loop (j) contains inner loop (k)
     * ============================================ */
    for (int i = 0; i < 10; ++i) {
        /* Basic block A: Start of outer loop body */
        sink = i * 2;
        g_array[i] = sink;
        
        for (int j = 0; j < 8; ++j) {
            /* Basic block B: Start of middle loop body */
            sink = i + j;
            g_array[i + j] = sink;
            
            for (int k = 0; k < 6; ++k) {
                /* Basic block C: Innermost loop body */
                sink = i * j * k;
                g_sink = sink;
                /* Simple operation to create basic block */
                if (sink > 100) {
                    g_array[0] = 1;  /* Never taken, but creates branch */
                }
            }
            
            /* Basic block D: After inner k loop, still in j loop */
            sink = j * 3;
            g_array[j] = sink;
        }
        
        /* ============================================
         * PART 2: Partially overlapping loop
         * This loop shares basic blocks with the outer i loop
         * but is NOT a perfect subset
         * It will trigger bitmap_intersect_p() = true
         * but bitmap_intersect_compl_p() = true for both
         * ============================================ */
        for (int m = 0; m < 5; ++m) {
            /* Basic block E: Partially overlapping loop body
             * This block is inside the i loop but outside the j loop
             * It shares the outer loop's basic block A context
             * but has different loop structure
             */
            sink = i * m;
            g_sink = sink;
            
            /* Additional operation to create distinct basic blocks */
            if (m % 2 == 0) {
                g_array[m] = i;
            } else {
                g_array[m + 10] = i * 2;
            }
        }
        
        /* Basic block F: Still in i loop, after partial overlap loop */
        sink = i * 10;
        g_sink = sink;
    }
    
    /* ============================================
     * PART 3: Separate non-intersecting loop
     * This loop doesn't share blocks with the nested loops
     * Provides variety in the loop analysis
     * ============================================ */
    for (int x = 0; x < 7; ++x) {
        /* Basic block G: Completely separate loop */
        sink = x * x;
        g_array[x + 20] = sink;
        
        /* Small inner loop to create another nesting level */
        for (int y = 0; y < 3; ++y) {
            /* Basic block H: Inner to the separate loop */
            sink = x + y;
            g_sink = sink;
        }
    }
    
    /* ============================================
     * PART 4: Another partially overlapping structure
     * Creates more complex intersection patterns
     * ============================================ */
    for (int p = 0; p < 4; ++p) {
        /* Basic block I */
        sink = p * 5;
        g_sink = sink;
        
        /* This inner loop shares structure with earlier loops */
        for (int q = 0; q < 3; ++q) {
            /* Basic block J: Similar to block C but in different context */
            sink = p * q;
            g_array[q + 30] = sink;
        }
        
        /* Another loop that starts here but extends beyond */
        for (int r = 0; r < 2; ++r) {
            /* Basic block K */
            sink = p + r;
            g_sink = sink;
            
            /* This creates partial overlap with the q loop's blocks */
            if (r == 1) {
                g_array[40] = p;  /* Basic block L */
            }
        }
    }
}

/* Main function with architecture detection */
int main(void) {
    printf("Starting hardware loop test...\n");
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the function with complex loop nesting */
    test_nested_loops();
    
    /* Use the results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 50; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (g_sink = %d)\n", sum, g_sink);
    return sum > 0 ? 0 : 1;
}

/* ============================================
 * Architecture-specific adaptations
 * Uncomment the appropriate section for your target
 * ============================================ */

/*
 * For RISC-V with Ziloop extension:
 * Compile with: -O2 -march=rv64gc_ziloop -fhwloops -fno-unroll-loops
 */
#ifdef __riscv__
__attribute__((target("arch=+ziloop")))
void riscv_hwloop_hint(void) {
    /* Empty function to hint at hardware loop availability */
    asm volatile ("# HWLOOP hint");
}
#endif

/*
 * For ARC targets:
 * Compile with: -O2 -mloop -ffunction-sections
 */
#ifdef __arc__
__attribute__((optimize("-O2 -mloop")))
void arc_hwloop_hint(void) {
    asm volatile ("# ARC LOOP hint");
}
#endif

/*
 * Generic fallback with optimization hints
 */
#ifndef __riscv__
#ifndef __arc__
__attribute__((optimize("O3", "no-unroll-loops")))
void generic_optimization_hint(void) {
    /* Try to encourage hardware loop optimization if available */
    asm volatile ("# Generic loop optimization hint");
}
#endif
#endif
