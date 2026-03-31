/* test_hwloop.c
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c
 * Or for generic testing: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c -o test_hwloop
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    int a, b, c;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (int i = 0; i < N; ++i) {
        /* No blocks here to ensure other is subset */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (int j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            a = i + j;
            b = a * 2;
            c = b - a;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            result ^= (a * b) >> (c & 7);
        }
        
        /* No blocks here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x + 1;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (int k = 0; k < (N - i); ++k) {
            /* Create more register pressure */
            x = i + k;
            y = x * 3;
            z = y - x;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= (x * y) >> (z & 3);
        }
        
        /* More blocks in 'other' after 'loop' */
        if (i & 1) {
            result += 1;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    int p, q, r;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        p = i * 2;
        
        /* Loop B - shares blocks via goto */
        for (int j = 0; j < 5; ++j) {
            q = p + j;
            
            if (q > 10) {
                /* Jump into Loop A's body */
                goto shared_block;
            }
            
            r = q * 3;
            asm volatile("" : : "r"(q), "r"(r));
            result += r;
        }
        
        continue;
        
    shared_block:
        /* This block is shared between both loops */
        r = p * 4;
        asm volatile("" : : "r"(r));
        result ^= r;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    int a, b, c;
    
    /* Outer for loop */
    for (int i = 0; i < N; ++i) {
        a = i;
        
        /* Inner do-while loop */
        int j = 0;
        do {
            b = a + j;
            c = b * 2;
            
            asm volatile("" : : "r"(b), "r"(c));
            result += c - b;
            
            j++;
        } while (j < 3);
        
        /* While loop after do-while */
        int k = 0;
        while (k < 2) {
            b = a - k;
            asm volatile("" : : "r"(b));
            result ^= b;
            k++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex sibling loops */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    int x, y, z;
    
    /* First sibling loop */
    for (int i = 0; i < N/2; ++i) {
        x = i * i;
        y = x + N;
        
        asm volatile("" : : "r"(x), "r"(y));
        result += y - x;
    }
    
    /* Second sibling loop (partially overlaps via shared variables) */
    for (int i = N/2; i < N; ++i) {
        x = i * 3;
        z = x - i;
        
        asm volatile("" : : "r"(x), "r"(z));
        result ^= z;
        
        /* Nested mini-loop */
        for (int j = 0; j < 2; ++j) {
            y = x + j;
            asm volatile("" : : "r"(y));
            result += y;
        }
    }
    
    return result & 0xFF;
}

/* Function 6: Disjoint loops (should not trigger intersection) */
NOINLINE int disjoint_loops(int N) {
    int result = 0;
    
    /* Loop 1 */
    for (int i = 0; i < 10; ++i) {
        int t = i * global_seed;
        asm volatile("" : : "r"(t));
        result += t;
    }
    
    /* Unrelated code block */
    result = (result * 31) & 0xFF;
    
    /* Loop 2 - completely separate */
    for (int i = 0; i < 5; ++i) {
        int t = i + global_seed;
        asm volatile("" : : "r"(t));
        result ^= t;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and command line to prevent constant propagation */
    volatile int base = argc;
    int N1 = (base % 50) + 20;
    int N2 = (base % 30) + 15;
    int N3 = (base % 40) + 10;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_nesting(N2);
    total ^= overlapping_loops(N3);
    total ^= mixed_loops(N1 + 5);
    total ^= sibling_loops(N2 + 3);
    total ^= disjoint_loops(N3 + 2);
    
    /* Generate side effect to prevent elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
