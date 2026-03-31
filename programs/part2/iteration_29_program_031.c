/* test_hw_loops.c - Target hardware loop nesting analysis test */

/* Force no inlining to preserve loop structure */
#define NOINLINE __attribute__((noinline, noipa))

/* Use volatile to prevent optimization */
static volatile int g_sink = 0;
static volatile int g_array[100];

/* Main test function with nested and intersecting loops */
NOINLINE static void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* Three perfectly nested loops - creates clear parent-child relationships */
    for (int i = 0; i < 10; ++i) {
        /* Outer loop body start */
        g_array[i] = i;
        
        for (int j = 0; j < 8; ++j) {
            /* Middle loop body */
            g_array[i + j] = i * j;
            
            for (int k = 0; k < 6; ++k) {
                /* Innermost loop body - simple operation */
                local_sink = i * j * k;
                g_sink = local_sink;
            }
            /* Middle loop tail */
            local_sink += j;
        }
        /* Outer loop continues... */
        
        /* 
         * PARTIALLY OVERLAPPING LOOP
         * This loop shares blocks with the outer 'i' loop but is not 
         * perfectly nested within it. It starts inside the outer loop's
         * body but continues differently.
         * This triggers: bitmap_intersect_p = true, but subset checks fail.
         */
        for (int m = 0; m < 5; ++m) {
            /* Uses 'i' from outer scope but has different control flow */
            if (i < 5) {
                g_array[m] = i + m;
            } else {
                g_array[m] = i - m;
            }
            local_sink += m;
        }
        /* Outer loop tail */
        g_sink = i;
    }
    
    /* 
     * SEPARATE NON-INTERSECTING LOOP
     * This loop doesn't share blocks with the nested loops above.
     * Should not intersect with any previous loops.
     */
    for (int x = 0; x < 7; ++x) {
        g_array[x + 20] = x * 2;
        local_sink += x;
    }
    
    /* Final side effect */
    g_sink = local_sink;
}

/* Another function with different nesting pattern */
NOINLINE static void test_sibling_loops(void) {
    volatile int sink = 0;
    
    /* Two independent loops at same level */
    for (int a = 0; a < 5; ++a) {
        sink += a;
        g_array[a] = a;
    }
    
    for (int b = 0; b < 5; ++b) {
        sink -= b;
        g_array[b + 5] = b;
    }
    
    /* Nested loop inside */
    for (int c = 0; c < 4; ++c) {
        for (int d = 0; d < 3; ++d) {
            sink = c * d;
        }
    }
    
    g_sink = sink;
}

/* Main function */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = 0;
    }
    
    /* Call test functions */
    test_nested_loops();
    test_sibling_loops();
    
    /* Use result to prevent dead code elimination */
    volatile int result = g_sink + g_array[0];
    
    /* Platform-specific output to prevent optimization */
#ifdef __riscv__
    asm volatile ("# Result: %0" : : "r"(result));
#elif defined(__ARC__)
    asm volatile ("# ARC result marker" : : : "memory");
#else
    /* Generic memory barrier */
    asm volatile ("" : : : "memory");
#endif
    
    return result != 0;
}

/* Compilation instructions for different targets:

1. For RISC-V with Ziloop extension:
   riscv64-unknown-elf-gcc -O2 -march=rv64gc_ziloop -fhwloops \
     -fno-unroll-loops -funroll-all-loops \
     -fdump-rtl-hwloops -fdump-rtl-all \
     -S test_hw_loops.c -o test_hw_loops.s

2. For ARC targets:
   arc-elf-gcc -O2 -mloop -ffunction-sections \
     -fno-unroll-loops \
     -fdump-tree-hwloops \
     test_hw_loops.c -o test_hw_loops.elf

3. For generic testing (if hardware loops supported):
   gcc -O3 -fhwloops -fno-unroll-loops \
     -fdump-rtl-hwloops -fdump-rtl-dfinit \
     test_hw_loops.c -o test_hw_loops

Expected coverage behavior:
- The three perfectly nested loops (i, j, k) create parent-child relationships
  where inner loops are subsets of outer loops, triggering:
    !bitmap_intersect_compl_p(inner->block_bitmap, outer->block_bitmap)
  This adds inner as child of outer.

- The partially overlapping loop (m) intersects with outer loop (i) but is not
  a subset, so bitmap_intersect_p returns true but both subset checks fail,
  causing the 'continue' to be taken (lines 429-430).

- The separate non-intersecting loop (x) doesn't intersect at all, so
  bitmap_intersect_p returns false, also taking the 'continue'.

- test_sibling_loops provides additional loop structures for analysis.
*/
