/* test_hwloop.c
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c -o test_hwloop.o
 * Or for generic targets: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c -o test_hwloop_executable
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
    int a, b, c, d, e, f; /* Create register pressure */
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        a = i * 2;
        b = i + 1;
        
        /* Inner loop - this will be 'other' (subset of outer) */
        for (int j = 0; j < (i % 5) + 1; ++j) {
            c = a * j;
            d = b - j;
            e = c ^ d;
            f = e << 2;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(c), "r"(d), "r"(e), "r"(f));
            
            result += f;
        }
        
        /* No code here ensures inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    int x, y, z, w, v;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        x = i * 3;
        y = i ^ 0x55;
        
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 2; ++j) {
            z = x + j;
            w = y - j;
            asm volatile("" : : "r"(z), "r"(w));
            result ^= z * w;
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < (i % 3) + 1; ++k) {
            v = x * k;
            asm volatile("" : : "r"(v));
            result += v;
        }
        
        /* More code in outer loop ensures 'loop' is proper subset */
        result = (result << 1) | (result >> 31);
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_with_goto(int N) {
    int result = 0;
    int p, q, r, s;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        p = i * 2;
        q = i + 5;
        
    loop_body:
        r = p * q;
        s = r ^ i;
        
        /* Loop B - shares the loop_body block via goto */
        for (int j = 0; j < (N - i); ++j) {
            if (j == 2) {
                /* Jump into Loop A's body */
                goto loop_body;
            }
            
            asm volatile("" : : "r"(r), "r"(s));
            result += s * j;
        }
        
        result = result ^ p;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int t1, t2, t3, t4, t5;
    
    /* Outer for loop */
    for (int i = 0; i < N; ++i) {
        t1 = i * 7;
        t2 = i % 11;
        
        /* Inner do-while loop */
        int dw = 0;
        do {
            t3 = t1 + dw;
            t4 = t2 ^ dw;
            asm volatile("" : : "r"(t3), "r"(t4));
            result += t3 - t4;
            dw++;
        } while (dw < 3);
        
        /* Another while loop in sequence */
        int w = 0;
        while (w < 2) {
            t5 = t1 * w;
            asm volatile("" : : "r"(t5));
            result ^= t5;
            w++;
        }
    }
    
    /* Follow with a separate while loop that might intersect */
    int outer = 0;
    while (outer < N/2) {
        /* This creates partial overlap with previous loops */
        result = result + perfect_nesting(outer % 3);
        outer++;
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with shared header block */
NOINLINE int sibling_loops_shared(int N) {
    int result = 0;
    int u, v, w;
    
    /* Shared setup block */
    int base = N * 2;
    
    /* First sibling loop */
    for (int i = 0; i < N; i += 2) {
        u = base + i;
        v = u ^ 0xAA;
        asm volatile("" : : "r"(u), "r"(v));
        result += u * v;
    }
    
    /* Shared intermediate computation */
    base = result & 0xF;
    
    /* Second sibling loop - shares some blocks via common subexpression */
    for (int j = 1; j < N; j += 3) {
        w = base * j;
        asm volatile("" : : "r"(w));
        result ^= w << 2;
    }
    
    return result & 0xFF;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 30) + 5;
    int N3 = (seed % 40) + 8;
    int N4 = (seed % 20) + 3;
    int N5 = (seed % 25) + 7;
    
    /* Call all functions to ensure they're compiled and executed */
    total += perfect_nesting(N1);
    total += loop_subset_of_other(N2);
    total += overlapping_with_goto(N3);
    total += mixed_loop_types(N4);
    total += sibling_loops_shared(N5);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    /* Additional calls with different parameters to create varied CFGs */
    if (argc > 1) {
        total += perfect_nesting(atoi(argv[1]) % 20);
        total += loop_subset_of_other(atoi(argv[1]) % 15);
    }
    
    return total & 1; /* Return 0 or 1 based on computation */
}
