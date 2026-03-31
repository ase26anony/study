/* test_hw_loops.c - Target hardware loop bitmap intersection analysis */

/* Force no inlining to preserve loop structure */
#define NOINLINE __attribute__((noinline, noipa))

/* Use volatile to prevent optimization */
static volatile int g_sink = 0;
static volatile int g_array[100];

/* Main test function with carefully structured loops */
NOINLINE static void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * Three perfectly nested loops - creates clear subset relationships.
     * Outer loop (i) contains middle loop (j) which contains inner loop (k).
     * This should establish parent-child relationships in hw-doloop analysis.
     */
    for (int i = 0; i < 5; ++i) {
        g_array[i] = i;  /* Basic block A - part of outer loop only */
        
        for (int j = 0; j < 4; ++j) {
            g_array[10 + j] = j;  /* Basic block B - part of middle loop */
            
            for (int k = 0; k < 3; ++k) {
                /* Basic block C - innermost loop body */
                local_sink = i * j * k;
                g_array[20 + k] = local_sink;
            }
            
            /* Basic block D - still in middle loop, after inner loop */
            g_sink += j;
        }
        
        /* 
         * PARTIALLY OVERLAPPING LOOP - shares some blocks with outer loop
         * but not perfectly nested. This triggers bitmap_intersect_p = true
         * but subset checks fail (continue path).
         * 
         * This loop starts in outer loop's body but continues independently.
         * It shares block A with outer loop but has its own blocks too.
         */
        for (int m = 0; m < 3; ++m) {
            /* Basic block E - part of overlapping loop */
            g_array[30 + m] = i * m;
            
            /* Additional unique block for overlapping loop */
            if (m % 2 == 0) {
                /* Basic block F - only in overlapping loop */
                local_sink += m;
            }
        }
        
        /* Basic block G - back in outer loop after overlapping loop */
        g_sink -= i;
    }
    
    /* 
     * SEPARATE NON-INTERSECTING LOOP - should not intersect with any
     * of the above loops, so bitmap_intersect_p = false (continue).
     */
    for (int n = 0; n < 10; ++n) {
        /* Basic block H - completely separate */
        g_array[40 + n] = n * 2;
    }
    
    /* Another set of nested loops for more parent-child testing */
    for (int x = 0; x < 4; ++x) {
        for (int y = 0; y < 3; ++y) {
            /* Basic block I - inner of second nest */
            local_sink = x - y;
            g_array[50 + x * 3 + y] = local_sink;
        }
    }
}

/* Main function with architecture guards */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = 0;
    }
    
    /* 
     * Conditionally compile hardware-loop-specific code.
     * In practice, compile with appropriate target flags.
     */
#if defined(__riscv__) || defined(__riscv)
    /* RISC-V with Ziloop extension */
    test_nested_loops();
#elif defined(__arc__) || defined(__ARC)
    /* ARC with hardware loops */
    test_nested_loops();
#elif defined(__XTENSA__)
    /* Xtensa with hardware loops */
    test_nested_loops();
#elif defined(__mips__) && defined(__mips_isa_rev) && (__mips_isa_rev >= 6)
    /* MIPS with microMIPS hardware loops */
    test_nested_loops();
#else
    /* Generic fallback - still test but hardware loops may not be used */
    test_nested_loops();
#endif
    
    /* Use result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 50; ++i) {
        sum += g_array[i];
    }
    
    return sum % 256;  /* Non-zero return to indicate execution */
}
