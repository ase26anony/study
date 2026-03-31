/* test_hwloop_coverage.c
 *
 * This test is designed to trigger specific bitmap intersection logic in GCC's
 * hardware loop optimization pass (hw-doloop.cc). It creates nested and adjacent
 * loops with specific block relationships to cover the uncovered lines:
 *
 *   if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))
 *     continue;
 *   if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))
 *     loop->loops.safe_push (other);
 *   else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))
 *     other->loops.safe_push (loop);
 *
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop_coverage.c
 * Then link and run to generate coverage data.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE
int perfect_nesting(int N) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < N; ++i) {
        /* No code here - ensures other is subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(d));
            
            result ^= d;
        }
        
        /* No code here either - maintains subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE
int reverse_nesting(int N) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (k = 0; k < (N / 2); ++k) {
            /* Register pressure */
            int x = i + k;
            int y = x * 3;
            int z = y - x;
            result ^= (x * y) >> (z & 3);
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE
int overlapping_with_goto(int N) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A (will be 'loop' or 'other') */
    for (i = 0; i < N; ++i) {
        result += i * 2;
        
    loop_body:
        /* Shared block label */
        if (i & 1) {
            result ^= i;
        }
    }
    
    /* Loop B that jumps into Loop A's body */
    for (j = 0; j < N; ++j) {
        /* Create some computation */
        int a = j * 3;
        result += a;
        
        if (j == N/2) {
            /* Jump into Loop A's body - creates intersection */
            goto loop_body;
        }
        
        /* More computation */
        int b = a * 2;
        asm volatile("" : : "r"(b));
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE
int mixed_loop_types(int N) {
    int result = 0;
    int i, j;
    
    /* Outer for loop */
    for (i = 0; i < N; ++i) {
        /* do-while loop inside */
        j = 0;
        do {
            /* Heavy register pressure */
            int a = i + j;
            int b = a * a;
            int c = b % 17;
            int d = c * i;
            int e = d ^ j;
            int f = e << 2;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
            
            result += f;
            j++;
        } while (j < 5);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 3) {
            result ^= (i * k);
            asm volatile("" : : "r"(k));
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with shared condition check */
NOINLINE
int sibling_loops(int N) {
    int result = 0;
    int i, j;
    
    /* First loop */
    for (i = 0; i < N; ++i) {
        int a = i * 2;
        result += a;
        
        /* Shared condition block */
        if (a > N) {
            result -= 1;
        }
    }
    
    /* Second loop that also uses the shared condition */
    for (j = 0; j < N/2; ++j) {
        int b = j * 3;
        result ^= b;
        
        /* Same condition block - creates intersection */
        if (b > N) {
            result += 1;
        }
        
        /* Extra computation to differentiate blocks */
        int c = b * j;
        asm volatile("" : : "r"(c));
    }
    
    return result & 0xFF;
}

/* Main driver that calls all functions */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc > 1 ? atoi(argv[1]) : global_seed;
    int N = (seed % 100) + 10;  /* Ensure N is between 10 and 109 */
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N);
    total ^= overlapping_with_goto(N);
    total ^= mixed_loop_types(N);
    total ^= sibling_loops(N);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
