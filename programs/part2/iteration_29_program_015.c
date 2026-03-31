/* test_hw_loops.c */
/* Compile with: gcc -O2 -fhwloops -fno-unroll-loops -fdump-rtl-hwloops test_hw_loops.c */

/* Force hardware loop optimization for supported targets */
#ifdef __riscv__
#define TARGET_SUPPORTS_HW_LOOPS 1
#elif defined(__ARC__)
#define TARGET_SUPPORTS_HW_LOOPS 1
#elif defined(__XTENSA__)
#define TARGET_SUPPORTS_HW_LOOPS 1
#else
/* Assume we're testing with hardware loop support enabled */
#define TARGET_SUPPORTS_HW_LOOPS 1
#endif

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;
volatile int arr[100] = {0};

/* Function containing all loop structures - marked noinline to ensure analysis */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int sink;
    
    /* 
     * First: Three perfectly nested loops (i, j, k)
     * This creates clear subset relationships:
     * - k-loop blocks ⊆ j-loop blocks ⊆ i-loop blocks
     */
    for (int i = 0; i < 10; ++i) {
        /* Outer loop body start */
        arr[i] = i;  /* Simple operation to create basic blocks */
        
        for (int j = 0; j < 8; ++j) {
            /* Middle loop body */
            sink = i * j;
            
            for (int k = 0; k < 6; ++k) {
                /* Innermost loop - proper subset of both outer loops */
                arr[k] = i + j + k;
                global_sink += arr[k];
            }
            
            /* More code in j-loop after inner k-loop */
            sink = j * 2;
        }
        
        /* 
         * Second: Partially overlapping loop using same 'i' variable
         * This loop shares blocks with the outer i-loop but is NOT
         * a perfect subset because it starts inside the i-loop body
         * but continues differently.
         * 
         * This should trigger:
         * 1. bitmap_intersect_p = true (they share some blocks)
         * 2. bitmap_intersect_compl_p checks both fail
         * 3. continue statement executes
         */
        for (int m = 0; m < 5; ++m) {
            /* This loop body partially overlaps with i-loop blocks */
            if (m % 2 == 0) {
                arr[m] = i * m;  /* Uses 'i' from outer scope */
            } else {
                sink = m;
            }
        }
        
        /* More code in i-loop after the partially overlapping loop */
        arr[i] *= 2;
    }
    
    /*
     * Third: Separate loop that doesn't intersect with the nested structure
     * This provides another candidate for the bitmap analysis
     */
    for (int x = 0; x < 12; ++x) {
        /* Completely separate loop - no block intersection with above loops */
        global_sink += x * x;
        arr[x + 20] = x;
    }
    
    /*
     * Fourth: Another set of nested loops at different nesting depth
     * to create more parent-child relationships
     */
    for (int a = 0; a < 7; ++a) {
        sink = a;
        
        /* Two-level nesting (not three like first example) */
        for (int b = 0; b < 5; ++b) {
            arr[a * 5 + b] = a + b;
            global_sink += arr[a * 5 + b];
        }
        
        /* Another partially overlapping loop inside a-loop */
        for (int c = 0; c < 3; ++c) {
            if (a > c) {
                sink = a - c;
            }
        }
    }
}

/* Main function to call our test and produce side effects */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        arr[i] = i;
    }
    
    /* Call the function with all our loop structures */
    test_nested_loops();
    
    /* Use the results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += arr[i];
    }
    
    /* Print to create observable side effect */
    printf("Result: %d (global_sink: %d)\n", sum, global_sink);
    
    return sum > 0 ? 0 : 1;
}
