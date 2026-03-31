/* test_hw_loops.c - Test program for hardware loop optimization coverage */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and ensure loops are analyzed together */
__attribute__((noinline, noipa))
static void test_nested_loops(void) {
    volatile int sink = 0;
    int arr[10][10][10];
    
    /* Three perfectly nested loops - creates clear parent-child relationships */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            for (int k = 0; k < 10; ++k) {
                /* Simple operation to prevent loop removal */
                arr[i][j][k] = i * j * k;
                sink = arr[i][j][k];
            }
        }
        
        /* Non-nested, intersecting loop - shares blocks with outer loop but not perfectly nested */
        /* This loop starts inside the i-loop body but continues independently */
        for (int m = 0; m < 5; ++m) {
            /* This creates partial overlap: shares the i-loop's blocks but has its own blocks too */
            sink = i * m * 2;
            arr[i][0][m] = sink;
        }
    }
    
    /* Separate non-intersecting loop for variety */
    for (int x = 0; x < 8; ++x) {
        sink = x * x;
    }
    
    /* Another partially overlapping loop structure */
    for (int a = 0; a < 7; ++a) {
        for (int b = 0; b < 3; ++b) {
            sink = a + b;
        }
        
        /* Loop that shares some blocks but not all */
        if (a < 5) {
            for (int c = 0; c < 4; ++c) {
                sink = a * c;
            }
        }
    }
}

/* Alternative implementation with more complex nesting for different architectures */
#ifdef __riscv__
__attribute__((noinline, noipa))
static void test_riscv_hwloops(void) {
    volatile int result = 0;
    int buffer[20][20];
    
    /* Clear parent-child relationship: inner is subset of middle is subset of outer */
    for (int outer = 0; outer < 15; ++outer) {
        result += outer;
        
        for (int middle = 0; middle < 10; ++middle) {
            result += middle;
            
            for (int inner = 0; inner < 5; ++inner) {
                buffer[outer][middle] = outer * middle + inner;
                result = buffer[outer][middle];
            }
        }
        
        /* Partially overlapping loop - triggers bitmap_intersect_p but not subset */
        /* Shares the 'outer' loop header but has different body blocks */
        for (int overlap = 0; overlap < 8; ++overlap) {
            if (overlap % 2 == 0) {
                result += overlap * 2;
            } else {
                result -= overlap;
            }
        }
    }
    
    /* Independent loop that doesn't intersect with above */
    for (int separate = 0; separate < 12; ++separate) {
        result = separate * separate;
    }
}
#endif

#ifdef __ARC__
__attribute__((noinline, noipa))
static void test_arc_hwloops(void) {
    volatile int acc = 0;
    int matrix[5][5][5];
    
    /* Triple nesting with simple operations */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            acc = i + j;
            
            for (int k = 0; k < 5; k++) {
                matrix[i][j][k] = acc * k;
                acc = matrix[i][j][k];
            }
        }
        
        /* Loop that intersects but isn't a subset */
        /* This will trigger the continue path when bitmap_intersect_compl_p fails */
        for (int p = i; p < 5; p++) {
            acc += p * i;
            if (p % 2) {
                acc -= 1;
            }
        }
    }
}
#endif

int main(void) {
    printf("Testing hardware loop optimization patterns...\n");
    
    /* Call the test function to generate the loop structures */
    test_nested_loops();
    
    /* Architecture-specific tests */
#if defined(__riscv__)
    test_riscv_hwloops();
    printf("RISC-V with Ziloop extension test executed\n");
#elif defined(__ARC__)
    test_arc_hwloops();
    printf("ARC with hardware loops test executed\n");
#else
    printf("Generic hardware loop test executed\n");
#endif
    
    /* Create side effect to prevent dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < 100; i++) {
        final_result += i;
    }
    
    printf("Final result (prevent optimization): %d\n", final_result);
    return 0;
}
