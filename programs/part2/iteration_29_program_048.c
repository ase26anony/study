/* test_hwloops.c
 * This program creates nested loops to trigger hardware loop optimization
 * analysis, specifically testing bitmap intersection logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int sink = 0;
volatile int result = 0;
int array[100] = {0};

/* Mark function to prevent inlining and ensure loops stay together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    int i, j, k;
    
    /* Three perfectly nested loops - creates clear subset relationships */
    for (i = 0; i < 10; ++i) {          /* Outer loop - contains all inner blocks */
        for (j = 0; j < 8; ++j) {       /* Middle loop - subset of outer */
            for (k = 0; k < 6; ++k) {   /* Inner loop - subset of middle */
                /* Simple operation to prevent loop removal */
                array[(i * 8 + j) * 6 + k] = i + j + k;
                sink = i * j * k;  /* Volatile store */
            }
        }
        
        /* PARTIALLY OVERLAPPING LOOP - shares blocks with outer loop but not nested */
        /* This starts in the outer loop body but continues differently */
        for (j = 0; j < 5; ++j) {       /* Uses same 'j' variable */
            /* Different body from the perfectly nested loops */
            array[i * 5 + j] += i * j;
            sink = i + j;  /* Volatile store */
        }
    }
    
    /* Another separate loop that doesn't intersect with the nested structure */
    /* This should fail the bitmap_intersect_p check */
    for (i = 0; i < 7; ++i) {
        array[i * 2] = i * 3;
        sink = i;  /* Volatile store */
    }
    
    /* Additional non-nested loop that shares NO blocks with previous loops */
    /* This ensures we test the continue path when no intersection */
    int x, y;
    for (x = 0; x < 4; ++x) {
        for (y = 0; y < 3; ++y) {
            array[x * 3 + y + 50] = x - y;  /* Use different array region */
            sink = x * y;  /* Volatile store */
        }
    }
}

/* Alternative version with more explicit partial overlap */
__attribute__((noinline, noipa))
void test_partial_overlap(void) {
    int a, b, c;
    
    /* Create a situation where loops partially overlap in blocks */
    for (a = 0; a < 5; ++a) {
        /* Block A1 */
        sink = a;
        
        for (b = 0; b < 4; ++b) {
            /* Block B - part of both loops? */
            sink = a + b;
            
            /* Partial loop that starts here but extends beyond */
            if (b == 2) {
                for (c = 0; c < 3; ++c) {
                    /* This loop shares block B with the 'b' loop */
                    /* but is not a perfect subset */
                    sink = a * b * c;
                }
            }
        }
        
        /* Block A2 - part of 'a' loop but not the partial 'c' loop */
        sink = a * 2;
    }
}

int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        array[i] = i;
    }
    
    /* Call our test functions */
    test_nested_loops();
    test_partial_overlap();
    
    /* Compute a result to prevent dead code elimination */
    result = 0;
    for (int i = 0; i < 100; ++i) {
        result += array[i];
    }
    
    printf("Result: %d\n", result);
    printf("Sink: %d\n", sink);
    
    return 0;
}
