/* test_hwloops.c
 * Target: Hardware with low-overhead loop support (RISC-V Ziloop, ARC, etc.)
 * Compile with: -O2 -fhwloops -fno-unroll-loops
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int sink;
volatile int array[100];

/* Mark function to prevent inlining and ensure all loops are analyzed together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    /* Three perfectly nested loops - creates clear parent-child relationships */
    for (int i = 0; i < 10; ++i) {          /* Outer loop L1 */
        for (int j = 0; j < 8; ++j) {       /* Middle loop L2 */
            for (int k = 0; k < 6; ++k) {   /* Inner loop L3 */
                /* Simple body to create basic blocks */
                sink = i * j * k;
                array[k] = sink;
            }
            /* Additional statement in middle loop body */
            sink = j;
        }
        
        /* PARTIALLY OVERLAPPING LOOP - shares blocks with L1 but not perfectly nested */
        /* This loop starts in L1's body but continues differently */
        for (int m = 0; m < 5; ++m) {       /* Overlapping loop L4 */
            /* Shares some control flow with L1 but not subset */
            sink = i * m;
            if (m % 2) {
                array[m] = sink;
            }
        }
    }
    
    /* SEPARATE NON-INTERSECTING LOOP - should not intersect with any above */
    for (int n = 0; n < 12; ++n) {          /* Isolated loop L5 */
        sink = n * 2;
        array[n % 10] = sink;
    }
    
    /* Additional intersecting but not subset loop structure */
    /* Creates another intersection case */
    for (int p = 0; p < 7; ++p) {           /* Loop L6 */
        sink = p;
        for (int q = 0; q < 3; ++q) {       /* Loop L7 - nested in L6 */
            array[q] = p * q;
        }
        /* This creates intersection but L7 is subset of L6 */
    }
}

/* Main function with architecture detection */
int main(void) {
    /* Initialize array to prevent optimization */
    for (int i = 0; i < 100; ++i) {
        array[i] = 0;
    }
    
    /* Call the test function */
    test_nested_loops();
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", sink + array[0]);
    
    return 0;
}

/* Architecture-specific compilation guidance */
#ifdef __riscv__
/* RISC-V with Ziloop extension */
/* Compile with: -O2 -march=rv64gc_ziloop -fhwloops -fno-unroll-loops */
#endif

#ifdef __ARC__
/* ARC with hardware loops */
/* Compile with: -O2 -mloop -ffunction-sections -fno-unroll-loops */
#endif

/* Generic fallback for testing */
#ifndef __riscv__
#ifndef __ARC__
/* Use attribute to suggest hardware loops if supported */
__attribute__((optimize("O2")))
#endif
#endif
