/* test_hw_loops.c - Test program for hardware loop optimization coverage */

/* Prevent unwanted optimizations */
volatile int g_sink = 0;
int g_array[100] = {0};

/* Mark function to prevent inlining and ensure loops stay together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int sink = 0;
    
    /* First: Three perfectly nested loops (i, j, k) */
    /* This creates clear subset relationships: k-blocks ⊂ j-blocks ⊂ i-blocks */
    for (int i = 0; i < 10; ++i) {
        /* Outer loop body start */
        g_array[i] = i;
        
        for (int j = 0; j < 8; ++j) {
            /* Middle loop body start */
            sink = i * j;
            
            for (int k = 0; k < 6; ++k) {
                /* Innermost loop - proper subset of both outer loops */
                g_sink = i + j + k;
                g_array[k] = sink + k;
            }
            /* Middle loop body end */
            g_sink = j;
        }
        /* Outer loop body continues... */
        
        /* Second: Partially overlapping loop using same 'i' variable */
        /* This loop shares blocks with outer i-loop but is not a subset */
        /* It starts inside i-loop body but has different structure */
        for (int m = 0; m < 5; ++m) {
            /* This loop shares the basic block containing 'i' increment */
            /* but has its own body blocks, creating partial overlap */
            sink = i * m * 2;
            g_array[m + 10] = sink;
        }
        /* Outer loop body end */
    }
    
    /* Third: Separate non-intersecting loop */
    /* This should fail the bitmap_intersect_p check and be skipped */
    for (int n = 0; n < 7; ++n) {
        g_sink = n * 3;
        g_array[n + 20] = g_sink;
    }
    
    /* Fourth: Another nested pair with different structure */
    /* Creates another parent-child relationship opportunity */
    for (int a = 0; a < 12; ++a) {
        g_sink = a;
        for (int b = 0; b < 9; ++b) {
            g_array[b] = a + b;
            sink = a * b;
        }
        
        /* Another partially overlapping loop inside a-loop */
        for (int c = 0; c < 4; ++c) {
            /* Shares some blocks with a-loop but not all */
            g_sink = a + c;
        }
    }
}

/* Alternative implementation for architectures without hardware loop support */
__attribute__((noinline, noipa))
void test_simple_loops(void) {
    volatile int sink = 0;
    /* Simple loops that work on any architecture */
    for (int i = 0; i < 10; ++i) {
        g_array[i] = i;
        for (int j = 0; j < 5; ++j) {
            sink = i * j;
        }
    }
}

int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = 0;
    }
    
    /* Use preprocessor to select appropriate test based on target */
#ifdef __riscv__
    /* RISC-V with Ziloop extension */
    test_nested_loops();
#elif defined(__ARC__) || defined(__ARC)
    /* ARC with hardware loops */
    test_nested_loops();
#elif defined(__DSP__) || defined(__Xcore__) || defined(__MICROBLAZE__)
    /* Other targets with hardware loop support */
    test_nested_loops();
#else
    /* Generic fallback - still creates loops but may not trigger hw-doloop */
    test_simple_loops();
#endif
    
    /* Create side effect to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 30; ++i) {
        sum += g_array[i];
    }
    
    g_sink = sum;
    return g_sink % 256;
}
