/* test_hw_loops.c
 * This program creates nested loop structures to trigger hardware loop
 * optimization analysis, specifically targeting bitmap intersection
 * and subset relationship checks in hw-doloop.cc.
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and inter-procedural analysis */
#ifdef __GNUC__
#define NO_INLINE __attribute__((noinline, noipa))
#else
#define NO_INLINE
#endif

/* Target-specific hardware loop support */
#if defined(__riscv) || defined(__riscv__)
#define USE_HW_LOOPS 1
#elif defined(__ARC__)
#define USE_HW_LOOPS 1
#elif defined(__XTENSA__)
#define USE_HW_LOOPS 1
#else
/* For testing with generic GCC that might have hardware loop support */
#define USE_HW_LOOPS 1
#endif

/* Function containing the complex loop structure */
NO_INLINE static void test_nested_loops(void) {
    int i, j, k;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * This creates clear parent-child relationships
     * where inner loops are proper subsets of outer loops
     * ============================================ */
    
    /* Outer loop - will be parent of both j and k loops */
    for (i = 0; i < 10; ++i) {
        /* Middle loop - child of i loop, parent of k loop */
        for (j = 0; j < 8; ++j) {
            /* Innermost loop - child of both i and j loops */
            for (k = 0; k < 6; ++k) {
                /* Simple operation to create basic blocks */
                g_array[i * 8 + j] += k;
                g_sink = i * j * k;
            }
            /* Additional operation in j loop body (not in k loop) */
            g_sink += j;
        }
        
        /* ============================================
         * PART 2: Partially overlapping loop
         * This loop shares the i loop's header and some blocks
         * but is NOT a perfect subset - it continues after
         * the j loop ends, creating partial overlap
         * ============================================ */
        
        /* This loop starts in the i loop body but has different structure */
        for (int m = 0; m < 5; ++m) {
            /* Operation that creates shared basic blocks with i loop */
            g_sink = i * m;
            
            /* Additional operation not in other loops */
            if (m % 2 == 0) {
                g_array[m] += i;
            }
        }
        
        /* More operations in i loop after the m loop */
        g_sink += i * 2;
    }
    
    /* ============================================
     * PART 3: Separate loop that doesn't intersect
     * This should fail the bitmap_intersect_p check
     * ============================================ */
    
    /* Completely separate loop (no block intersection with previous loops) */
    for (int x = 0; x < 7; ++x) {
        g_array[x + 50] = x * x;
        g_sink = x;
    }
    
    /* ============================================
     * PART 4: Another nested structure with different bounds
     * Creates additional parent-child relationships
     * ============================================ */
    
    /* Another outer loop */
    for (int a = 0; a < 5; ++a) {
        /* Inner loop that's a proper subset */
        for (int b = 0; b < 4; ++b) {
            g_sink = a + b;
            g_array[a * 4 + b] = a * b;
        }
        
        /* Another partially overlapping loop */
        for (int c = 0; c < 3; ++c) {
            /* Shares some blocks with a loop but not all */
            if (c < a) {
                g_sink = a - c;
            }
        }
    }
}

/* Main function */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the function with complex loop structure */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (sink: %d)\n", sum, g_sink);
    
    return 0;
}
