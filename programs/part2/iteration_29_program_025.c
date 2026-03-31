/* test_hwloops.c - Target hardware loop nesting analysis */

/* Prevent unwanted optimizations */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DO(x) do { volatile int _sink; _sink = (x); } while(0)

/* Target-specific guards */
#if defined(__riscv__) || defined(__ARC__) || defined(__DSP__) || defined(__Xcore__)
  #define ENABLE_HW_LOOPS 1
#else
  #define ENABLE_HW_LOOPS 0
  #pragma message "Compiling for generic target - hardware loop optimization may not trigger"
#endif

/* Global array to prevent elimination */
static volatile int results[100];

/* Main test function with carefully structured loops */
NOINLINE static void test_nested_loops(void) {
    /* Three perfectly nested loops - creates clear parent-child relationships */
    for (int i = 0; i < 5; ++i) {          /* Outer loop L1 */
        for (int j = 0; j < 4; ++j) {      /* Middle loop L2 (child of L1) */
            for (int k = 0; k < 3; ++k) {  /* Inner loop L3 (child of L2) */
                /* Simple body - single basic block */
                VOLATILE_DO(i * 100 + j * 10 + k);
                results[k] = i + j + k;
            }
            /* This block is in L2 but not in L3 */
            VOLATILE_DO(j * 10);
        }
        /* This block is in L1 but not in L2/L3 */
        
        /* PARTIALLY OVERLAPPING LOOP - shares some blocks with L1 but not subset */
        /* This loop starts inside L1's body but isn't nested within L2/L3 */
        for (int m = 0; m < 3; ++m) {      /* Loop L4 - intersects with L1 but not subset */
            VOLATILE_DO(i * 50 + m);
            results[m + 10] = i * m;
        }
        /* End of L4 - still in L1 */
        
        VOLATILE_DO(i * 1000);
    }
    
    /* Separate intersecting loop structure - more complex relationship */
    /* This creates intersection without subset relationship */
    for (int x = 0; x < 4; ++x) {          /* Loop L5 */
        VOLATILE_DO(x * 2);
        
        if (x < 2) {
            /* Nested loop inside conditional - creates different block structure */
            for (int y = 0; y < 3; ++y) {  /* Loop L6 - child of L5 when x<2 */
                VOLATILE_DO(x * 100 + y);
                results[y + 20] = x + y;
            }
        } else {
            /* Different path - still in L5 but not in L6 */
            VOLATILE_DO(x * 200);
        }
        
        /* Another loop that shares the L5 header but has different body */
        for (int z = 0; z < 2; ++z) {      /* Loop L7 - sibling of L6? */
            VOLATILE_DO(x * 300 + z);
            results[z + 30] = x * z;
        }
    }
    
    /* Isolated loop - no intersection with others */
    for (int w = 0; w < 10; ++w) {         /* Loop L8 - isolated */
        VOLATILE_DO(w * 999);
        results[w + 40] = w;
    }
}

/* Alternative test with switch-based loop variation */
NOINLINE static void test_loop_variants(void) {
    int counter = 0;
    
    /* Loop with multiple exits/continues to create multiple basic blocks */
    for (int i = 0; i < 8; ++i) {          /* Loop A */
        if (i % 3 == 0) {
            VOLATILE_DO(i * 111);
            continue;  /* Creates separate basic block */
        }
        
        for (int j = 0; j < i; ++j) {      /* Loop B - size depends on i */
            if (j % 2 == 0) {
                VOLATILE_DO(i * 1000 + j);
                results[j] = i - j;
            } else {
                /* Different block in Loop B */
                VOLATILE_DO(j * 777);
            }
        }
        
        /* Another loop that starts here - intersects with A but not B */
        for (int k = 0; k < 2; ++k) {      /* Loop C */
            VOLATILE_DO(i * 2000 + k);
        }
    }
}

int main(void) {
    /* Initialize to prevent constant propagation */
    for (int i = 0; i < 100; ++i) {
        results[i] = i;
    }
    
    /* Call test functions */
    test_nested_loops();
    
#if ENABLE_HW_LOOPS
    /* Call second test if hardware loops supported */
    test_loop_variants();
#endif
    
    /* Use results to prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < 50; ++i) {
        sum += results[i];
    }
    
    /* Prevent printf optimization if not needed */
    VOLATILE_DO(sum);
    
    return sum > 0 ? 0 : 1;
}
