/* test_hw_loops.c - Test program for hardware loop optimization coverage */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure loops stay together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    int i, j, k;
    
    /* ========== Three perfectly nested loops ========== */
    /* Outer loop - will contain inner loops as children */
    for (i = 0; i < 10; ++i) {
        /* Middle loop - child of outer, parent of innermost */
        for (j = 0; j < 8; ++j) {
            /* Innermost loop - child of middle loop */
            for (k = 0; k < 6; ++k) {
                /* Simple operation to prevent dead code elimination */
                g_sink = i * j + k;
                g_array[(i * 8 + j) * 6 + k] = g_sink;
            }
        }
        
        /* ========== Partially overlapping loop ========== */
        /* This loop shares blocks with the outer 'i' loop but is not 
           perfectly nested within it. It starts inside the outer loop body
           but doesn't contain the 'j' loop, creating partial overlap.
           This triggers bitmap_intersect_p() but not the subset checks. */
        for (int m = 0; m < 5; ++m) {
            /* Different operation to create distinct basic blocks */
            g_sink = i * m + 1000;
            g_array[m] = g_sink;
        }
    }
    
    /* ========== Separate non-intersecting loop ========== */
    /* This loop doesn't intersect with any of the above loops */
    for (int n = 0; n < 7; ++n) {
        g_sink = n * 2;
        g_array[n + 50] = g_sink;
    }
    
    /* ========== Another nested pair for more coverage ========== */
    /* Creates another parent-child relationship */
    for (int a = 0; a < 4; ++a) {
        for (int b = 0; b < 3; ++b) {
            g_sink = a - b;
            g_array[a * 3 + b + 20] = g_sink;
        }
    }
}

/* Main function with architecture detection */
int main(void) {
    printf("Testing hardware loop optimization patterns\n");
    
    /* Architecture-specific compilation hints */
#ifdef __riscv__
    printf("Compiled for RISC-V with hardware loop support\n");
#elif defined(__ARC__)
    printf("Compiled for ARC with hardware loop support\n");
#elif defined(__XTENSA__)
    printf("Compiled for Xtensa with hardware loop support\n");
#else
    printf("Compiled for generic target (may not trigger hw loops)\n");
#endif
    
    /* Call the test function */
    test_nested_loops();
    
    /* Use the results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += g_array[i];
    }
    
    printf("Result check: %d (array[0]=%d)\n", sum, g_array[0]);
    return sum > 0 ? 0 : 1;
}
