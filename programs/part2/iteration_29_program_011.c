/* test_hw_loops.c - Test program for hardware loop optimization coverage */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int global_sink = 0;
volatile int array[100];

/* Mark function as noinline to ensure loops are analyzed together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int sink;
    
    /* Requirement 1: Three perfectly nested countable loops */
    /* Outer loop - will be parent of middle and inner loops */
    for (int i = 0; i < 5; ++i) {
        /* Middle loop - child of outer, parent of inner */
        for (int j = 0; j < 4; ++j) {
            /* Inner loop - child of middle and outer */
            for (int k = 0; k < 3; ++k) {
                /* Simple operation to prevent optimization */
                sink = i * j + k;
                array[(i * 16 + j * 4 + k) % 100] = sink;
            }
        }
        
        /* Requirement 2: Non-nested, intersecting loop */
        /* This loop shares blocks with outer loop but is not perfectly nested */
        /* It starts inside outer loop body but continues independently */
        for (int m = 0; m < 3; ++m) {
            /* This creates partial overlap with outer loop's blocks */
            sink = i * m + 7;
            array[(i * 10 + m) % 100] = sink;
            
            /* The key: This loop's blocks intersect with outer loop's blocks
               but it's not a subset (has blocks outside outer loop's scope) */
        }
    }
    
    /* Requirement 2 continued: Another intersecting loop structure */
    /* Reset i for clarity */
    int outer_i;
    for (outer_i = 0; outer_i < 4; ++outer_i) {
        /* First inner loop */
        for (int inner_j = 0; inner_j < 3; ++inner_j) {
            sink = outer_i * inner_j;
            array[(outer_i * 3 + inner_j) % 100] = sink;
        }
        
        /* This creates a diamond-like structure where blocks partially overlap */
        if (outer_i % 2 == 0) {
            for (int alt_k = 0; alt_k < 2; ++alt_k) {
                sink = outer_i + alt_k * 10;
                array[(outer_i * 2 + alt_k) % 100] = sink;
            }
        }
    }
    
    /* Requirement 3: Separate non-intersecting loop */
    /* This loop should not intersect with any other loop's blocks */
    for (int separate = 0; separate < 10; ++separate) {
        sink = separate * 2;
        array[separate] = sink;
    }
}

/* Main function with compilation guidance */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        array[i] = 0;
    }
    
    /* Call the test function */
    test_nested_loops();
    
    /* Create side effect to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += array[i];
    }
    
    printf("Result: %d\n", sum);
    return 0;
}

/* Alternative version with architecture-specific compilation */
#ifdef __riscv__
/* RISC-V specific hardware loop test */
__attribute__((noinline, noipa))
void test_riscv_hwloops(void) {
    volatile int sink;
    
    /* Complex nesting to trigger bitmap analysis */
    for (int a = 0; a < 8; a++) {
        /* Child loop A */
        for (int b = 0; b < 6; b++) {
            sink = a * b;
        }
        
        /* Partially overlapping loop - shares some blocks but not all */
        for (int c = 0; c < 4; c++) {
            sink = a + c;
            if (c % 2) {
                /* Creates additional basic blocks */
                for (int d = 0; d < 2; d++) {
                    sink = a * c * d;
                }
            }
        }
    }
    
    /* Another independent loop nest */
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 5; y++) {
            for (int z = 0; z < 5; z++) {
                sink = x * y * z;
            }
        }
    }
}
#endif /* __riscv__ */
