/* test_hw_loops.c - Test program for hardware loop optimization coverage */

/* Force no inlining to preserve loop structure */
#define NOINLINE __attribute__((noinline, noipa))

/* Target-specific hardware loop support */
#ifdef __riscv__
#define TARGET_SUPPORTS_HW_LOOPS 1
#elif defined(__ARC__)
#define TARGET_SUPPORTS_HW_LOOPS 1
#elif defined(__XTENSA__)
#define TARGET_SUPPORTS_HW_LOOPS 1
#elif defined(__NDS32__)
#define TARGET_SUPPORTS_HW_LOOPS 1
#else
/* For testing with generic GCC if hardware loops are enabled */
#define TARGET_SUPPORTS_HW_LOOPS 1
#endif

/* Global variables to prevent optimization */
volatile int g_sink = 0;
int g_array[100] = {0};

/* Main test function with carefully structured loops */
NOINLINE
static void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * Three perfectly nested loops - creates clear parent-child relationships.
     * Loop A (outer): blocks = {A_header, A_body, A_latch}
     * Loop B (middle): blocks = {B_header, B_body, B_latch} ⊆ A_body
     * Loop C (inner): blocks = {C_header, C_body, C_latch} ⊆ B_body
     */
    for (int i = 0; i < 10; ++i) {           /* Loop A - outermost */
        for (int j = 0; j < 8; ++j) {        /* Loop B - middle */
            for (int k = 0; k < 6; ++k) {    /* Loop C - innermost */
                /* Simple body to create basic blocks */
                local_sink = i * j * k;
                g_array[(i * 8 + j) % 100] += k;
            }
            /* Middle loop tail - part of Loop B but not Loop C */
            g_sink += j;
        }
        
        /*
         * Loop D: Partially overlaps with Loop A
         * - Shares some blocks with Loop A (the outer i loop)
         * - But NOT a perfect subset: 
         *   - Starts inside Loop A's body (after Loop B)
         *   - Continues beyond where Loop B was
         *   - bitmap_intersect_p will return true
         *   - But neither is a subset of the other
         *   - Should trigger the continue path at line 431
         */
        for (int d = 0; d < 5; ++d) {        /* Loop D - partially overlapping */
            /* This loop shares the outer i loop's scope but isn't nested within j */
            local_sink = i * d + 1000;
            g_array[(i * 5 + d) % 100] -= 1;
        }
        
        /* More code in Loop A after Loop D */
        g_sink -= i;
    }
    
    /*
     * Loop E: Separate loop that doesn't intersect with any previous loops
     * - Should not trigger any parent-child relationship with A/B/C/D
     * - Provides additional variety for bitmap analysis
     */
    for (int e = 0; e < 12; ++e) {           /* Loop E - completely separate */
        local_sink = e * 2;
        g_array[e % 100] = e;
    }
    
    /*
     * Additional complexity: Loop F that's nested but with early exit
     * Creates more interesting control flow for analysis
     */
    for (int f_outer = 0; f_outer < 7; ++f_outer) {  /* Loop F outer */
        for (int f_inner = 0; f_inner < 4; ++f_inner) { /* Loop F inner */
            if (f_inner == 2) {
                /* Early continue - creates additional basic blocks */
                continue;
            }
            local_sink = f_outer * f_inner;
        }
        
        /* 
         * Loop G: Another partially overlapping loop inside F
         * Will intersect with F but not be a perfect subset
         */
        for (int g = 0; g < 3; ++g) {        /* Loop G - partial overlap with F */
            local_sink = f_outer * g * 10;
        }
    }
    
    /* Final sink to prevent dead code elimination */
    g_sink = local_sink;
}

/* Helper to ensure loops aren't optimized away */
NOINLINE
static int compute_checksum(void) {
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += g_array[i];
    }
    return sum + g_sink;
}

int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i % 10;
    }
    
    /* Execute the test loops */
    test_nested_loops();
    
    /* Compute and print result to create observable side effect */
    int result = compute_checksum();
    
    /* Use result to prevent optimization */
    volatile int output = result;
    
    return output % 256;
}
