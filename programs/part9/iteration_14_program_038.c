/* test_hwloop.c
 * 
 * This test is designed to trigger specific bitmap intersection logic in GCC's
 * hardware loop optimization pass (hw-doloop.cc). It creates nested and adjacent
 * loops with specific block relationships to cover the uncovered lines 429-436.
 *
 * Compilation for coverage:
 *   gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c -o test_hwloop.o
 *   gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a test_hwloop.c -o test_hwloop_executable
 *
 * Run the executable to generate profile data:
 *   ./test_hwloop_executable
 *
 * Target requirements: Architecture with hardware loop support (ARM, RISC-V with Ziloop, PowerPC)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units for coverage */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2)
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    int a, b, c;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* Inner loop - this will be 'other' (subset of outer) */
        for (int j = 0; j < i % 5 + 1; ++j) {
            /* Create register pressure */
            a = i * j;
            b = a + j;
            c = b - i;
            result ^= (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
        }
        
        /* No code here ensures inner loop is perfect subset */
    }
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3)
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 2; ++j) {
            x = i + j;
            y = x * 3;
            result += y;
            asm volatile("" : : "r"(x), "r"(y));
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < i % 3 + 1; ++k) {
            z = i * k;
            result ^= z;
            asm volatile("" : : "r"(z));
        }
    }
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto (Condition 1)
 * Creates loops that intersect but neither is subset of the other
 */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    int p, q, r;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        p = i * 2;
        
    shared_label:
        /* This block will be shared between loops */
        q = p + 1;
        result += q;
        
        /* Loop B - shares the block at shared_label via goto */
        for (int j = 0; j < 3; ++j) {
            if (j == 1 && i < N/2) {
                goto shared_label;  /* Jump into Loop A's body */
            }
            r = j * 3;
            result ^= r;
            asm volatile("" : : "r"(r));
        }
        
        asm volatile("" : : "r"(p), "r"(q));
    }
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    int a, b, c, d;
    
    /* Outer while loop */
    int i = 0;
    while (i < N) {
        /* First inner do-while loop */
        int j = 0;
        do {
            a = i + j;
            b = a * 2;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
            j++;
        } while (j < 3);
        
        /* Second inner for loop (could be 'loop' or 'other') */
        for (int k = 0; k < i % 4 + 1; ++k) {
            c = i * k;
            d = c - 1;
            result ^= d;
            asm volatile("" : : "r"(c), "r"(d));
        }
        
        i++;
    }
    
    /* Additional loop after to create more CFG complexity */
    for (int m = 0; m < 5; ++m) {
        result += m * 7;
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with partial overlap via break */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    int x, y, z;
    
    /* Loop X */
    for (int i = 0; i < N; ++i) {
        x = i * 3;
        
        if (i == N/2) {
            /* This break target creates shared block */
            goto shared_break;
        }
        
        y = x + 5;
        result += y;
        asm volatile("" : : "r"(x), "r"(y));
    }
    
    /* Loop Y - shares the break target block */
    for (int j = 0; j < N/2; ++j) {
        if (j == N/4) {
            shared_break:
            z = j * 7;
            result ^= z;
            asm volatile("" : : "r"(z));
            continue;
        }
        
        int w = j * 2;
        result += w;
        asm volatile("" : : "r"(w));
    }
    
    return result & 0xFF;
}

/* Main function to drive all test cases */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 100) + 10;
    int N2 = (seed % 80) + 15;
    int N3 = (seed % 60) + 20;
    int N4 = (seed % 40) + 25;
    int N5 = (seed % 20) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total += perfect_nesting(N1);
    total += reverse_nesting(N2);
    total += overlapping_loops(N3);
    total += mixed_loops(N4);
    total += sibling_loops(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
