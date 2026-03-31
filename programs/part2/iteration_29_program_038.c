/* test_hw_loops.c - Program to trigger hardware loop nesting analysis */
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to keep loop structures intact */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int sink = 0;
    int arr[10][10][10] = {0};
    
    /* Three perfectly nested loops - creates clear parent-child relationships */
    for (int i = 0; i < 10; ++i) {           /* Outer loop - will be parent of j and k loops */
        for (int j = 0; j < 10; ++j) {       /* Middle loop - child of i, parent of k */
            for (int k = 0; k < 10; ++k) {   /* Innermost loop - child of j */
                /* Simple operation to prevent optimization */
                arr[i][j][k] = i + j + k;
                sink = arr[i][j][k];
            }
        }
        
        /* PARTIALLY OVERLAPPING LOOP - shares blocks with outer i loop but not perfectly nested */
        /* This starts in the i loop body but continues differently */
        for (int m = 0; m < 5; ++m) {        /* Partially overlaps with i loop */
            /* Different operation to create distinct basic blocks */
            sink = i * m + 2;
            arr[i][0][m] = sink;
        }
    }
    
    /* SEPARATE NON-INTERSECTING LOOP - should not intersect with any above */
    /* This comes after all i loop iterations complete */
    for (int x = 0; x < 8; ++x) {
        sink = x * 3;
        arr[0][0][x] = sink;
    }
    
    /* Additional complexity: another set of nested loops at same level */
    /* This creates multiple candidate loops for analysis */
    for (int a = 0; a < 7; ++a) {
        for (int b = 0; b < 7; ++b) {
            sink = a * b;
            arr[a][b][0] = sink;
        }
    }
}

/* Alternative version for architectures without hardware loop support */
__attribute__((noinline, noipa))
void test_nested_loops_fallback(void) {
    volatile int sink = 0;
    int arr[5][5][5] = {0};
    
    /* Simpler version for generic targets */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            for (int k = 0; k < 5; ++k) {
                arr[i][j][k] = i + j + k;
            }
        }
        
        /* Partially overlapping loop */
        for (int m = 0; m < 3; ++m) {
            sink = i * m;
            arr[i][0][m] = sink;
        }
    }
    
    /* Separate loop */
    for (int x = 0; x < 4; ++x) {
        sink = x * 2;
        arr[0][0][x] = sink;
    }
}

int main(void) {
    volatile int result = 0;
    
    /* Use architecture-specific compilation */
#ifdef __riscv__
    /* RISC-V with Ziloop extension */
    printf("Testing on RISC-V with hardware loop support\n");
    test_nested_loops();
    result = 1;
#elif defined(__ARC__) || defined(__ARC)
    /* ARC with loop extension */
    printf("Testing on ARC with hardware loop support\n");
    test_nested_loops();
    result = 2;
#elif defined(__XTENSA__)
    /* Xtensa with loop extension */
    printf("Testing on Xtensa with hardware loop support\n");
    test_nested_loops();
    result = 3;
#elif defined(__MSP430__)
    /* MSP430 with hardware multiplier */
    printf("Testing on MSP430\n");
    test_nested_loops();
    result = 4;
#else
    /* Generic fallback - still creates loops but may not trigger hw-doloop */
    printf("Testing generic version\n");
    test_nested_loops_fallback();
    result = 5;
#endif
    
    printf("Result: %d\n", result);
    return result;
}
