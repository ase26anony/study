/* test_hw_loops.c - Designed to trigger specific bitmap intersection logic in GCC's hw-doloop.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force ARM target for hardware loop support */
#ifdef __ARM_ARCH
#define TARGET_ATTR __attribute__((target("arch=armv8-a")))
#else
#define TARGET_ATTR __attribute__((target("arch=armv8-a")))
#endif

/* Prevent inlining to preserve CFG structure */
#define NOINLINE __attribute__((noinline))

/* Volatile variables to prevent constant propagation */
static volatile int N = 100;
static volatile int M = 50;
static volatile int K = 75;

/* Arrays for side effects */
static int array1[200];
static int array2[200];
static int array3[200];

/* Function containing carefully constructed loops */
TARGET_ATTR NOINLINE 
static void test_loop_patterns(void) {
    volatile int i, j, k;
    int sum = 0;
    
    /* Loop 1: Simple countable loop (disjoint from others) */
    for (i = 0; i < N; i++) {
        array1[i] = i * 2;
        /* Simple side effect to prevent dead code elimination */
        asm volatile("" : "+r"(array1[i]) : : "memory");
    }
    
    /* Loop 2: Perfectly nested inside Loop 3 */
    /* This should trigger: loop->loops.safe_push(other) */
    for (j = 0; j < M; j++) {
        /* Loop 3: Outer loop containing Loop 2 */
        for (k = 0; k < K; k++) {
            array2[j + k] = j * k;
            asm volatile("" : "+r"(array2[j + k]) : : "memory");
        }
    }
    
    /* Loop 4: Do-while loop (different structure) */
    i = 0;
    do {
        array3[i] = i * 3;
        asm volatile("" : "+r"(array3[i]) : : "memory");
        i++;
    } while (i < N);
    
    /* Loop 5 and Loop 6: Partially overlapping loops */
    /* This should trigger: other->loops.safe_push(loop) */
    /* Create complex CFG with conditional jumps between loops */
    
    /* Label for partial overlap */
    partial_overlap_start:
    
    /* Loop 5 */
    for (i = 0; i < N; i++) {
        array1[i] += i;
        asm volatile("" : "+r"(array1[i]) : : "memory");
        
        /* Conditional that can jump into Loop 6 */
        if (i == M) {
            /* Jump to label inside Loop 6's conceptual body */
            /* This creates partial overlap in CFG */
            goto inside_loop6;
        }
    }
    
    /* Loop 6 - partially overlaps with Loop 5 via goto */
    for (j = 0; j < M; j++) {
        inside_loop6:
        array2[j] = j * 4;
        asm volatile("" : "+r"(array2[j]) : : "memory");
        
        /* Jump back to Loop 5's region if condition met */
        if (j == K/2) {
            goto partial_overlap_start;
        }
    }
    
    /* Loop 7: Another loop with internal branching */
    /* Creates more complex bitmap for analysis */
    for (i = 0; i < K; i++) {
        if (i % 2 == 0) {
            array3[i] = array1[i] + array2[i];
        } else {
            array3[i] = array1[i] - array2[i];
        }
        asm volatile("" : "+r"(array3[i]) : : "memory");
    }
    
    /* Loop 8: While loop with early exit (different CFG shape) */
    i = 0;
    while (i < N) {
        array1[i] = array1[i] * 2;
        asm volatile("" : "+r"(array1[i]) : : "memory");
        
        if (array1[i] > 1000) {
            break;  /* Creates additional basic block */
        }
        i++;
    }
}

/* Additional function with sibling loops */
TARGET_ATTR NOINLINE
static void sibling_loops(void) {
    volatile int x, y;
    
    /* Two adjacent loops that should be disjoint */
    for (x = 0; x < 10; x++) {
        array1[x + 10] = x;
        asm volatile("" : "+r"(array1[x + 10]) : : "memory");
    }
    
    for (y = 0; y < 10; y++) {
        array2[y + 10] = y * 2;
        asm volatile("" : "+r"(array2[y + 10]) : : "memory");
    }
    
    /* Nested loops with different bounds */
    for (x = 0; x < 5; x++) {
        for (y = 0; y < 3; y++) {
            array3[x * 3 + y] = x + y;
            asm volatile("" : "+r"(array3[x * 3 + y]) : : "memory");
        }
    }
}

int main(void) {
    int i;
    int checksum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < 200; i++) {
        array1[i] = 0;
        array2[i] = 0;
        array3[i] = 0;
    }
    
    /* Call function with complex loop patterns */
    test_loop_patterns();
    
    /* Call function with sibling loops */
    sibling_loops();
    
    /* Compute checksum to ensure all loops executed */
    for (i = 0; i < 200; i++) {
        checksum += array1[i] + array2[i] + array3[i];
    }
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
