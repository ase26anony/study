/* 
 * Test program for GCC hardware loop optimization coverage
 * Specifically targets bitmap intersection logic in hw-doloop.cc
 * 
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c hwloop_test.c
 * For generic targets: gcc -O2 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage hwloop_test.c
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
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            a = i + j;
            b = a * 2;
            c = b - a;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            result ^= (a * b) >> (c & 7);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            asm volatile("" : : "r"(x));
            result += x;
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < (N / 2); ++k) {
            /* Create register pressure */
            y = i + k;
            z = y * 3;
            
            asm volatile("" : : "r"(y), "r"(z));
            result ^= (y * z) >> (k & 3);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_with_goto(int N) {
    int result = 0;
    int p, q, r;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        p = i * 2;
        
    loop_b_start:
        /* Loop B - shares block via goto */
        for (int j = 0; j < (N / 2); ++j) {
            q = i + j;
            r = q * 3;
            
            asm volatile("" : : "r"(q), "r"(r));
            result ^= (p * q) >> (r & 7);
            
            /* Conditional goto into Loop A's body */
            if ((i + j) % 7 == 0) {
                goto shared_block;
            }
        }
        
        continue;
        
    shared_block:
        /* This block is shared between both loops via the goto */
        result += p;
        asm volatile("" : : "r"(result));
        
        /* Jump back to Loop B */
        if (i < N - 1) {
            goto loop_b_start;
        }
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with do-while and while */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int a, b, c;
    
    /* Outer for loop */
    for (int i = 0; i < N; ++i) {
        a = i;
        
        /* Inner do-while loop */
        int dw = 0;
        do {
            b = a + dw;
            asm volatile("" : : "r"(b));
            result ^= b;
            dw++;
        } while (dw < 5);
        
        /* Another while loop in same outer loop */
        int w = 0;
        while (w < 3) {
            c = a * w;
            asm volatile("" : : "r"(c));
            result += c;
            w++;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int complex_sibling_loops(int N) {
    int result = 0;
    int tmp1, tmp2, tmp3;
    
    /* Level 1 loop */
    for (int i = 0; i < N; ++i) {
        /* Sibling loop A */
        for (int j = 0; j < (i % 5 + 1); ++j) {
            tmp1 = i * j;
            asm volatile("" : : "r"(tmp1));
            result += tmp1;
        }
        
        /* Some intermediate code */
        tmp2 = i * 7;
        
        /* Sibling loop B */
        for (int k = 0; k < (N - i); ++k) {
            tmp3 = tmp2 + k;
            asm volatile("" : : "r"(tmp3));
            result ^= tmp3;
        }
        
        /* Another loop at same level */
        int m = 0;
        while (m < 4) {
            asm volatile("" : : "r"(m));
            result -= m;
            m++;
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to create variable loop bounds */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    
    /* Call all test functions */
    total += perfect_nesting(N1);
    total += loop_subset_of_other(N2);
    total += overlapping_with_goto(N3);
    total += mixed_loop_types(N4);
    total += complex_sibling_loops(N5);
    
    /* Generate side effect to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 0xFF;
}
