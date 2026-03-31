/* test_hw_loops.c
 * This program creates specific loop nesting patterns to trigger
 * hardware loop optimization analysis, particularly the bitmap
 * intersection and subset checks in hw-doloop.cc lines 429-436.
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure all loops are analyzed together */
__attribute__((noinline, noipa))
static void test_nested_loops(void) {
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
                g_array[(i * 8 + j) % 100] += local_sink;
            }
        }
        
        /*
         * Loop D: Partially overlaps with Loop A but not perfectly nested.
         * Starts inside Loop A's body (after Loop B) but continues
         * with different structure. This creates:
         * - bitmap_intersect_p(A, D) = true (they share some blocks)
         * - !bitmap_intersect_compl_p(D, A) = false (D has blocks outside A)
         * - !bitmap_intersect_compl_p(A, D) = false (A has blocks outside D)
         * So neither is subset of the other -> continue path taken.
         */
        for (int m = 0; m < 5; ++m) {       /* Loop D - intersecting but not nested */
            local_sink = i * m;
            g_array[(i * 5 + m) % 100] += local_sink;
            
            /* Small inner loop within D to create more complex structure */
            for (int n = 0; n < 3; ++n) {   /* Loop E - child of D */
                g_sink += n;
            }
        }
    }
    
    /*
     * Loop F: Separate loop that doesn't intersect with any previous loops.
     * This provides variety in the loop analysis.
     */
    for (int p = 0; p < 7; ++p) {           /* Loop F - separate, non-intersecting */
        local_sink = p * 2;
        g_array[p % 100] += local_sink;
    }
    
    /* Another set of nested loops with different structure */
    for (int x = 0; x < 4; ++x) {           /* Loop G */
        for (int y = 0; y < 3; ++y) {       /* Loop H - child of G */
            g_sink += x + y;
            
            /* Loop I: Another intersecting but not perfectly nested loop */
            for (int z = 0; z < 2; ++z) {   /* Loop I - child of H */
                g_array[(x + y + z) % 100] += 1;
            }
            
            /* J: Loop that starts in H but extends differently */
            int temp = y;
            while (temp < 3) {              /* Loop J - intersects with H but not subset */
                g_sink += temp;
                temp++;
            }
        }
    }
    
    /* Final operation to ensure all loops have side effects */
    g_sink = local_sink;
}

/* Main function with architecture detection */
int main(void) {
    printf("Testing hardware loop optimization patterns\n");
    
    /* Architecture-specific compilation hints */
#if defined(__riscv__) && defined(__riscv_ziloop)
    printf("Compiled for RISC-V with Ziloop extension\n");
#elif defined(__ARC__)
    printf("Compiled for ARC with hardware loops\n");
#elif defined(__XTENSA__)
    printf("Compiled for Xtensa with hardware loops\n");
#else
    printf("Compiled for generic target (may not trigger hw-doloop)\n");
#endif
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        g_array[i] = i;
    }
    
    /* Call the function with complex loop nesting */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += g_array[i];
    }
    
    printf("Result: sink=%d, array_sum=%d\n", g_sink, sum);
    return sum > 0 ? 0 : 1;
}
