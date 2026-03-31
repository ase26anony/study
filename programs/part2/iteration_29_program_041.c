/* test_hwloops.c - Test program for hardware loop optimization coverage */

/* Prevent unwanted optimizations */
volatile int g_sink = 0;
int g_array[100] = {0};

/* Mark function to prevent inlining and ensure loops stay together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int sink = 0;
    
    /* Three perfectly nested loops - creates clear parent-child relationships */
    for (int i = 0; i < 10; ++i) {
        /* Outer loop body start */
        g_array[i] = i;
        
        for (int j = 0; j < 8; ++j) {
            /* Middle loop body */
            sink = i * j;
            
            for (int k = 0; k < 6; ++k) {
                /* Innermost loop body - simple operation */
                g_sink = i + j + k;
                sink = k;
            }
            
            /* Middle loop continuation */
            g_array[j] += sink;
        }
        
        /* Outer loop continuation - partial overlap region */
        
        /* PARTIALLY OVERLAPPING LOOP: 
           Shares blocks with outer loop but not perfectly nested.
           This triggers bitmap_intersect_p() but not subset checks */
        for (int m = 0; m < 5; ++m) {
            /* This loop starts in outer loop's body but continues 
               after the j loop ends, creating partial overlap */
            g_sink = i * m;
            sink = m;
        }
        /* End of partial overlap loop */
        
        /* More outer loop code */
        g_array[i] += sink;
    }
    
    /* SEPARATE NON-INTERSECTING LOOP:
       Should not intersect with any other loop's blocks */
    for (int x = 0; x < 7; ++x) {
        g_sink = x * 2;
        g_array[x] = g_sink;
    }
    
    /* Another set of nested loops at same level */
    for (int a = 0; a < 5; ++a) {
        for (int b = 0; b < 4; ++b) {
            sink = a + b;
            g_sink = sink;
        }
    }
}

/* Main function with side effects to prevent elimination */
int main(void) {
    /* Initialize to prevent constant propagation */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i % 10;
    }
    
    /* Call the test function */
    test_nested_loops();
    
    /* Create observable side effect */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += g_array[i];
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    g_sink = sum;
    
    /* Return value based on computation for verification */
    return (sum > 0) ? 0 : 1;
}
