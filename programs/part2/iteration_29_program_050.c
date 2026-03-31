/* test_hw_loops.c - Test program for hardware loop nesting analysis */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and ensure loops stay intact */
#ifdef __GNUC__
#define NOINLINE __attribute__((noinline, noipa))
#else
#define NOINLINE
#endif

/* Target-specific hardware loop support */
#if defined(__riscv) || defined(__riscv__)
#define TARGET_SUPPORTS_HWLOOPS 1
#elif defined(__arc__) || defined(__ARC__)
#define TARGET_SUPPORTS_HWLOOPS 1
#elif defined(__XTENSA__) || defined(__tensa__)
#define TARGET_SUPPORTS_HWLOOPS 1
#else
/* Assume generic target might support hardware loops */
#define TARGET_SUPPORTS_HWLOOPS 1
#endif

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Main test function with carefully structured loops */
NOINLINE
static void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * 1. Three perfectly nested countable loops (i, j, k)
     *    Creates clear subset relationships: k-blocks ⊂ j-blocks ⊂ i-blocks
     */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 8; ++j) {
            for (int k = 0; k < 6; ++k) {
                /* Simple operation that can't be optimized away */
                local_sink = i * j + k;
                g_array[(i * 8 + j) * 6 + k] = local_sink;
            }
        }
        
        /*
         * 2. Non-nested, partially intersecting loop
         *    This loop shares the outer i-loop's blocks but is not a subset
         *    because it continues after the j-loop ends.
         *    Triggers: bitmap_intersect_p = true
         *              bitmap_intersect_compl_p = true (both ways)
         *              So neither subset condition is met → continue
         */
        for (int m = 0; m < 5; ++m) {
            /* Operation in intersecting but non-nested loop */
            local_sink = i * m + 100;
            g_array[i * 5 + m] += local_sink;
        }
    }
    
    /*
     * 3. Separate loop that doesn't intersect with the nested structure
     *    This should not trigger any parent-child relationship with the above loops
     */
    for (int x = 0; x < 12; ++x) {
        local_sink = x * x;
        g_array[x] = local_sink;
    }
    
    /*
     * 4. Additional partially overlapping loop structure
     *    Creates more complex intersection patterns
     */
    for (int a = 0; a < 7; ++a) {
        /* First inner loop */
        for (int b = 0; b < 4; ++b) {
            local_sink = a + b;
            g_sink = local_sink;
        }
        
        /* Loop that overlaps with 'a' but not perfectly nested */
        if (a < 5) {
            for (int c = 0; c < 3; ++c) {
                local_sink = a * c;
                g_array[a * 3 + c] = local_sink;
            }
        }
    }
    
    /* Ensure side effect */
    g_sink = local_sink;
}

/* Secondary function to create additional loop contexts */
NOINLINE
static void test_sibling_loops(void) {
    volatile int sink = 0;
    
    /* Two independent loops at same nesting level */
    for (int p = 0; p < 9; ++p) {
        sink = p * 2;
        g_array[p] = sink;
    }
    
    for (int q = 0; q < 9; ++q) {
        sink = q * 3;
        g_array[q + 10] = sink;
    }
    
    /* Loop that will be sibling to one above but child of none */
    for (int r = 0; r < 4; ++r) {
        for (int s = 0; s < 3; ++s) {
            sink = r * s;
            g_array[r * 3 + s + 20] = sink;
        }
    }
}

int main(void) {
    /* Initialize to prevent constant propagation */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call test functions multiple times to ensure they're not optimized away */
    for (int iter = 0; iter < 3; ++iter) {
        test_nested_loops();
        test_sibling_loops();
    }
    
    /* Compute checksum to ensure all loops executed */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (g_sink = %d)\n", sum, g_sink);
    return 0;
}
