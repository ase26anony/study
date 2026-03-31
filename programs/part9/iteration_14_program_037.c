/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates specific loop structures to trigger bitmap intersection
 * logic in hw-doloop.cc lines 429-436.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
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
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - this will be 'other' (subset of outer) */
        for (int j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            a = i + j;
            b = i * 2 - j;
            c = b - a;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            result ^= (a * b) >> (c & 0xF);
        }
        
        /* No code here either to maintain subset relationship */
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
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x + j;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < (N - i); ++k) {
            /* Create more complex operations */
            x = i + k;
            y = k * 2;
            z = x * y - k;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= (z >> 2) & 0xFF;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto (Condition 1)
 * Loops share blocks but neither is subset of the other
 */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    int a, b, c;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        a = i * 2;
        
    shared_block:
        b = a + i;
        asm volatile("" : : "r"(a), "r"(b));
        result += b & 0xF;
        
        /* Loop B - shares the shared_block via goto */
        for (int j = 0; j < 5; ++j) {
            if (j == 2) {
                /* Jump into Loop A's body */
                goto shared_block;
            }
            
            c = i + j * 3;
            asm volatile("" : : "r"(c));
            result ^= c;
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (int outer = 0; outer < N; ++outer) {
        int counter = 0;
        
        /* do-while loop */
        do {
            result += (outer * counter) & 0xFF;
            asm volatile("" : : "r"(counter));
            counter++;
        } while (counter < 3);
        
        /* while loop */
        int inner = 0;
        while (inner < (N - outer)) {
            int temp = outer * inner;
            result ^= temp;
            asm volatile("" : : "r"(temp));
            inner++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with break to shared label */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    
    /* First loop */
    for (int i = 0; i < N; ++i) {
        int val = i * 3;
        
    shared_exit:
        result += val & 0xFF;
        asm volatile("" : : "r"(val));
        
        /* Second loop - can break to shared_exit */
        for (int j = 0; j < 5; ++j) {
            if (i + j > N) {
                /* Break to shared block in first loop */
                val = j * 7;
                goto shared_exit;
            }
            
            int tmp = i * j;
            asm volatile("" : : "r"(tmp));
            result ^= tmp;
        }
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;  /* Range: 10-109 */
    
    printf("Running hardware loop coverage test with N=%d\n", N);
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N);
    total ^= overlapping_loops(N);
    total ^= mixed_loops(N);
    total ^= sibling_loops(N);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return (total & 0xFF) == 0 ? 0 : 1;
}
