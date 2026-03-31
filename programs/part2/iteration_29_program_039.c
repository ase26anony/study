/* test_hw_loops.c - Target hardware loop nesting analysis */

/* Prevent unwanted optimizations */
#ifdef __GNUC__
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DO(x) do { volatile int _sink; _sink = (x); } while(0)
#else
#define NOINLINE
#define VOLATILE_DO(x) (void)(x)
#endif

/* Global array to prevent dead code elimination */
volatile int results[100] = {0};

/* Main test function with carefully structured loops */
NOINLINE
void test_nested_loops(void) {
    volatile int sink;
    
    /* 
     * Three perfectly nested loops - creates clear parent-child relationships.
     * Loop A (outer): blocks = {A_header, A_body, A_latch}
     * Loop B (middle): blocks = {B_header, B_body, B_latch} ⊂ A_body
     * Loop C (inner): blocks = {C_header, C_body, C_latch} ⊂ B_body
     */
    for (int i = 0; i < 10; ++i) {           /* Loop A - outermost */
        for (int j = 0; j < 8; ++j) {        /* Loop B - middle */
            for (int k = 0; k < 6; ++k) {    /* Loop C - innermost */
                /* Simple body - creates basic blocks */
                sink = i * j * k;
                results[k] = sink;  /* Prevent optimization */
            }
            /* Middle loop body after inner loop */
            sink = j * 2;
            results[j + 10] = sink;
        }
        
        /*
         * Loop D: Partially overlaps with Loop A but not perfectly nested.
         * Starts inside Loop A's body (after Loop B) but continues
         * with different structure. This creates:
         * - bitmap_intersect_p(A, D) = true (share some blocks)
         * - !bitmap_intersect_compl_p(D, A) = false (D has blocks outside A)
         * - !bitmap_intersect_compl_p(A, D) = false (A has blocks outside D)
         * So neither is subset of the other -> continue
         */
        for (int m = 0; m < 7; ++m) {        /* Loop D - intersecting */
            /* This block is inside Loop A but not identical to Loop A's blocks */
            sink = i * m + 5;
            results[m + 20] = sink;
            
            /* Additional basic block inside Loop D */
            if (m % 2 == 0) {
                sink = m * 3;
                results[m + 30] = sink;
            }
        }
        
        /* More code in Loop A after Loop D */
        sink = i * 100;
        results[i + 40] = sink;
    }
    
    /*
     * Loop E: Separate loop that doesn't intersect with any previous loops.
     * This ensures we have multiple loops to analyze.
     */
    for (int n = 0; n < 5; ++n) {            /* Loop E - separate */
        sink = n * n;
        results[n + 50] = sink;
    }
    
    /*
     * Additional intersecting structure:
     * Loop F and Loop G share blocks but neither is subset of the other.
     */
    for (int p = 0; p < 4; ++p) {            /* Loop F */
        /* Shared block */
        sink = p * 10;
        results[p + 60] = sink;
        
        for (int q = 0; q < 3; ++q) {        /* Loop G - nested in F */
            sink = p * q;
            results[q + 70] = sink;
        }
        
        /* More F blocks after G */
        sink = p * 20;
        results[p + 80] = sink;
        
        /*
         * Loop H: Another intersecting loop inside F but not containing G.
         * Creates more intersection cases.
         */
        for (int r = 0; r < 2; ++r) {        /* Loop H */
            sink = p * r * 30;
            results[r + 90] = sink;
        }
    }
}

/* Main function */
int main(void) {
    /* Initialize to prevent constant propagation */
    for (int i = 0; i < 100; ++i) {
        results[i] = i;
    }
    
    /* Call the test function */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += results[i];
    }
    
    /* Print to create side effect */
#ifdef __GNUC__
    __builtin_printf("Result: %d\n", sum);
#else
    printf("Result: %d\n", sum);
#endif
    
    return sum > 0 ? 0 : 1;
}
