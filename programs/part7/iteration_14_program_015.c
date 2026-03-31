/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    int a, b, c;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' (subset of loop) */
        for (int j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            a = i + j;
            b = a * 2;
            c = b - i;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            result ^= (a * b) >> (c & 7);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x + 1;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (int k = 0; k < (N - i); ++k) {
            x = i + k;
            y = x * 3;
            z = y - k;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= (x * y) >> (z & 7);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto (Condition 1) */
NOINLINE int partial_overlap(int N) {
    int result = 0;
    int p, q, r;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < N; ++i) {
    loop_a_body:
        p = i * 2;
        q = p + 1;
        asm volatile("" : : "r"(p), "r"(q));
        result += p - q;
        
        /* Loop B - will be 'other' */
        for (int j = 0; j < 5; ++j) {
            r = i + j;
            
            /* Jump into loop A's body to create intersection */
            if (j == 3 && (result & 1)) {
                goto loop_a_body;  /* Creates shared basic block */
            }
            
            asm volatile("" : : "r"(r));
            result ^= r;
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with break to shared label */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    int a, b, c;
    
    /* do-while inside for */
    for (int i = 0; i < N; ++i) {
        int count = 0;
        
        /* do-while loop */
        do {
            a = i + count;
            b = a * count;
            asm volatile("" : : "r"(a), "r"(b));
            result += b;
            count++;
        } while (count < 3);
        
    shared_label:
        c = result & 15;
        asm volatile("" : : "r"(c));
        
        /* while loop that can break to shared label */
        int k = 0;
        while (k < (N - i)) {
            if (k == 2 && (i & 1)) {
                goto shared_label;  /* Creates intersection */
            }
            result ^= (i * k);
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex sibling loops */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    int tmp1, tmp2, tmp3;
    
    /* First loop */
    for (int i = 0; i < N; ++i) {
        tmp1 = i * 3;
        tmp2 = tmp1 + 7;
        asm volatile("" : : "r"(tmp1), "r"(tmp2));
        result += tmp1 - tmp2;
    }
    
    /* Second loop that shares no blocks with first (disjoint) */
    for (int j = 0; j < N; ++j) {
        tmp3 = j * 5;
        asm volatile("" : : "r"(tmp3));
        result ^= tmp3;
    }
    
    /* Nested loops for subset relationships */
    for (int outer = 0; outer < N/2; ++outer) {
        for (int inner = 0; inner < outer; ++inner) {
            tmp1 = outer + inner;
            tmp2 = tmp1 * 2;
            asm volatile("" : : "r"(tmp1), "r"(tmp2));
            result += tmp2;
        }
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 50) + 20;  /* Ensure loops are non-trivial */
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N);
    total ^= partial_overlap(N);
    total ^= mixed_loops(N);
    total ^= sibling_loops(N);
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", total & 255);
    
    return 0;
}
