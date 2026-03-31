/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * or for generic testing: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    volatile int result = 0;
    int a, b, c, d, e, f;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* Create register pressure */
        a = i * 2;
        b = i + 5;
        c = a ^ b;
        
        /* Inner loop - this will be 'other' (subset of outer) */
        for (int j = 0; j < 5; ++j) {
            /* Complex body to prevent optimization */
            d = j * 3;
            e = d - i;
            f = e ^ c;
            result += f;
            
            /* Prevent dead code elimination */
            asm volatile("" : : "r"(result));
        }
        
        /* No code here ensures inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int N) {
    volatile int result = 0;
    int x, y, z;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x ^ 0x55;
            result ^= y;
            asm volatile("" : : "r"(result));
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < 4; ++k) {
            z = k + i;
            result += z * 2;
            asm volatile("" : : "r"(result));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto (Condition 1) */
NOINLINE int partial_overlap_goto(int N) {
    volatile int result = 0;
    int tmp1, tmp2, tmp3;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < N; ++i) {
        tmp1 = i * 3;
        
    loop_a_body:
        tmp2 = tmp1 ^ i;
        result += tmp2;
        
        /* Loop B - will be 'other' */
        for (int j = 0; j < 4; ++j) {
            tmp3 = j * 5;
            result ^= tmp3;
            
            /* Jump into loop A's body to create intersection */
            if (j == 2 && i < N/2) {
                goto loop_a_body;
            }
            
            asm volatile("" : : "r"(result));
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(tmp1));
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with break to shared label */
NOINLINE int mixed_loops_shared_block(int N) {
    volatile int result = 0;
    int a, b, c;
    
    /* do-while loop */
    int i = 0;
    do {
        a = i * 2;
        
        /* while loop inside do-while */
        int j = 0;
        while (j < 3) {
            b = j + a;
            result += b;
            
            /* Shared label that both loops can reach */
            shared_block:
            c = result ^ 0xAA;
            
            if (j == 1 && i % 2 == 0) {
                /* Break to shared block */
                goto shared_block;
            }
            
            j++;
            asm volatile("" : : "r"(c));
        }
        
        i++;
    } while (i < N);
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int complex_sibling_loops(int N) {
    volatile int result = 0;
    int v1, v2, v3, v4, v5;
    
    /* Outer loop */
    for (int i = 0; i < N; ++i) {
        v1 = i * 7;
        
        /* First sibling inner loop */
        for (int j = 0; j < 2; ++j) {
            v2 = v1 + j;
            v3 = v2 ^ 0x33;
            result += v3;
            asm volatile("" : : "r"(v3));
        }
        
        /* Some intermediate code */
        v4 = result * 2;
        
        /* Second sibling inner loop */
        for (int k = 0; k < 3; ++k) {
            v5 = v4 - k;
            result ^= v5;
            asm volatile("" : : "r"(v5));
        }
        
        /* Another loop at same level */
        int m = 0;
        while (m < 2) {
            result += m * 11;
            m++;
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int total = 0;
    
    /* Use volatile to prevent optimization */
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 20) + 25;
    int N5 = (seed % 10) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= loop_subset_of_other(N2);
    total ^= partial_overlap_goto(N3);
    total ^= mixed_loops_shared_block(N4);
    total ^= complex_sibling_loops(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
