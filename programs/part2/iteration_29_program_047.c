/* test_hw_loops.c - Target hardware loop optimization coverage */

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
     * Outer loop (i) contains middle loop (j) which contains inner loop (k).
     * Basic blocks: outer_loop > middle_loop > inner_loop (proper subsets)
     */
    for (int i = 0; i < 10; ++i) {
        /* Some computation in outer loop body before inner loops */
        local_sink += i;
        g_array[i] = i;
        
        for (int j = 0; j < 8; ++j) {
            /* Middle loop body */
            local_sink += j;
            
            for (int k = 0; k < 6; ++k) {
                /* Innermost loop body - simple operation */
                local_sink += i * j * k;
                g_sink = local_sink;
            }
            
            /* More middle loop body after inner loop */
            local_sink -= j;
        }
        
        /*
         * PARTIALLY OVERLAPPING LOOP - Critical for coverage!
         * This loop shares the outer loop's basic blocks but is not a subset.
         * It starts inside the outer loop's body but continues differently.
         * This triggers bitmap_intersect_p() = true, but subset checks fail.
         */
        for (int m = 0; m < 5; ++m) {
            /* This loop uses 'i' from outer loop but has different structure */
            local_sink += i + m;
            g_array[m] = local_sink;
            
            /* Different control flow - no inner j/k loops */
            if (m % 2 == 0) {
                local_sink *= 2;
            }
        }
        
        /* More outer loop body after overlapping loop */
        g_array[i + 10] = local_sink;
    }
    
    /*
     * SEPARATE NON-INTERSECTING LOOP
     * This loop doesn't intersect with the nested loops above.
     * Should not trigger any parent-child relationships.
     */
    for (int x = 0; x < 7; ++x) {
        local_sink += x * x;
        g_sink = local_sink;
        
        /* Small inner loop within this separate loop */
        for (int y = 0; y < 3; ++y) {
            local_sink -= y;
            g_array[x * 3 + y] = local_sink;
        }
    }
    
    /* Final sink assignment to prevent dead code elimination */
    g_sink = local_sink;
}

/* 
 * Another function with different nesting pattern to ensure
 * multiple loop structures are analyzed
 */
__attribute__((noinline, noipa))
static void test_alternate_nesting(void) {
    volatile int sink2 = 0;
    
    /* Different loop bounds and structure */
    for (int a = 0; a < 12; a += 2) {  /* Non-unit step */
        sink2 += a;
        
        for (int b = a; b < 10; ++b) {  /* Depends on outer variable */
            sink2 += b;
            
            /* Conditional continue to create more basic blocks */
            if (b % 3 == 0) {
                continue;
            }
            
            sink2 *= 2;
        }
        
        /* Another partially overlapping loop */
        for (int c = 0; c < 4; ++c) {
            sink2 += a + c;
        }
    }
    
    g_sink += sink2;
}

int main(void) {
    printf("Testing hardware loop optimization patterns...\n");
    
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call functions with nested loops */
    test_nested_loops();
    test_alternate_nesting();
    
    /* Use results to prevent optimization */
    printf("Result: %d (array[0]=%d, array[99]=%d)\n", 
           g_sink, g_array[0], g_array[99]);
    
    return 0;
}
