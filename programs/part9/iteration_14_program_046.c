/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop hierarchies to trigger the bitmap
 * intersection logic in hw-doloop.cc lines 429-436.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' */
        for (int j = 0; j < 5; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = c * 3;
            
            /* Prevent optimization */
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= (a * b) >> (c & 3);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a + 1;
            result += b;
            asm volatile ("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (int k = 0; k < 4; ++k) {
            int x = i + k;
            int y = x * 2;
            int z = y - x;
            result ^= z;
            asm volatile ("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto
 * This ensures bitmap intersection but neither is subset
 */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        int a = i * 2;
        
        /* Loop B - partially overlaps with Loop A via goto */
        for (int j = 0; j < 3; ++j) {
            if (i > N/2 && j == 1) {
                /* Jump into Loop A's body */
                goto shared_block;
            }
            result += i + j;
        }
        
        shared_block:
        result ^= a;
        
        /* Mixed loop type: do-while inside for */
        int m = 0;
        do {
            int b = m * i;
            result += b;
            asm volatile ("" : : "r"(b));
            m++;
        } while (m < 2);
    }
    
    return result & 0xFF;
}

/* Function 4: Complex hierarchy with while loop */
NOINLINE int complex_hierarchy(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop as outer */
    while (i < N) {
        /* for loop inside while */
        for (int j = 0; j < 4; ++j) {
            /* Another nested for */
            for (int k = 0; k < 2; ++k) {
                int a = i + j + k;
                int b = a * 3;
                int c = b >> 1;
                result ^= c;
                asm volatile ("" : : "r"(a), "r"(b), "r"(c));
            }
        }
        
        /* Sibling loop to the for above */
        int m = 0;
        while (m < 3) {
            result += m * i;
            m++;
        }
        
        i++;
    }
    
    return result & 0xFF;
}

/* Function 5: Disjoint loops with shared basic blocks via switch */
NOINLINE int switch_shared_blocks(int N) {
    int result = 0;
    
    for (int i = 0; i < N; ++i) {
        switch (i % 3) {
            case 0: {
                /* Loop A */
                for (int j = 0; j < 2; ++j) {
                    result += i * j;
                }
                /* Shared basic block after loop */
                result += 1;
                break;
            }
            case 1: {
                /* Loop B - shares the "result += 1" block via fallthrough */
                for (int k = 0; k < 3; ++k) {
                    result -= i + k;
                }
                result += 1;  /* Shared block */
                break;
            }
            default:
                result ^= i;
                break;
        }
    }
    
    return result & 0xFF;
}

/* Main function to drive all test cases */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N + 1);
    total ^= overlapping_loops(N + 2);
    total ^= complex_hierarchy(N + 3);
    total ^= switch_shared_blocks(N + 4);
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", total & 255);
    
    return 0;
}
