/* test_hwloops.c - Test hardware loop nesting analysis */

/* Force no inlining to preserve loop structure */
#define NOINLINE __attribute__((noinline, noipa))

/* Use volatile to prevent optimization */
static volatile int g_sink = 0;
static int g_array[100];

/* Main test function with carefully constructed loops */
NOINLINE void test_nested_loops(void) {
    int i, j, k;
    
    /* ============================================
     * PART 1: Perfectly nested loops (3 levels)
     * These create clear parent-child relationships
     * ============================================ */
    
    /* Outer loop - will be parent of j and k loops */
    for (i = 0; i < 10; ++i) {
        /* Middle loop - child of i, parent of k */
        for (j = 0; j < 8; ++j) {
            /* Inner loop - child of j */
            for (k = 0; k < 6; ++k) {
                /* Simple body to create basic blocks */
                g_array[i * 8 + j] += k;
                g_sink = i * j * k;
            }
            
            /* This creates a basic block between k loop and j loop end */
            if (j % 2 == 0) {
                g_sink = j;
            }
        }
        
        /* ============================================
         * PART 2: Partially overlapping loop
         * This loop shares the i loop's header/exit but
         * has different inner structure
         * ============================================ */
        
        /* This loop starts in i's body but isn't nested with j/k loops */
        /* It will intersect with i loop but not be a subset */
        for (int m = 0; m < 5; ++m) {
            /* Different operation to create distinct blocks */
            g_array[i + 10] += m * 2;
            g_sink = i + m;
            
            /* Small conditional to create more blocks */
            if (m == 3) {
                g_sink = 999;
            }
        }
    }
    
    /* ============================================
     * PART 3: Separate loop that doesn't intersect
     * This should fail the bitmap_intersect_p check
     * ============================================ */
    
    /* This loop is completely separate - no block overlap */
    for (int x = 0; x < 7; ++x) {
        g_array[x + 50] = x * x;
        g_sink = x;
    }
    
    /* ============================================
     * PART 4: Another partially overlapping case
     * Loop that shares some blocks but not all
     * ============================================ */
    
    /* First create a simple loop */
    for (int a = 0; a < 12; ++a) {
        g_sink = a * 2;
        
        /* Then a loop that starts here but continues after */
        for (int b = 0; b < 4; ++b) {
            g_array[a + 20] += b;
            
            /* This creates partial overlap:
             * - b loop is inside a loop
             * - But a loop continues after b loop ends
             * - So b is subset of a, but not vice versa
             */
        }
        
        /* a loop continues here after b loop */
        g_sink = a + 100;
    }
}

/* ============================================
 * Target-specific compilation hints
 * ============================================ */

#ifdef __riscv__
/* RISC-V with Ziloop extension */
#define TARGET_SUPPORTS_HWLOOPS 1
#elif defined(__ARC__)
/* ARC with loop extension */
#define TARGET_SUPPORTS_HWLOOPS 1
#elif defined(__XTENSA__)
/* Xtensa with loop extension */
#define TARGET_SUPPORTS_HWLOOPS 1
#else
/* Generic - might still work if compiler supports hwloops */
#define TARGET_SUPPORTS_HWLOOPS 0
#endif

int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = 0;
    }
    
    /* Call our test function */
    test_nested_loops();
    
    /* Use result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += g_array[i];
    }
    
    /* Print to create side effect */
    volatile int result = sum + g_sink;
    
    return result > 0 ? 0 : 1;
}
