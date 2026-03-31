/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates specific loop structures to exercise the
 * bitmap intersection logic in hw-doloop.cc's discover_loop_hierarchy:
 * 
 * 1. Perfectly nested loops (other is subset of loop)
 * 2. Outer loop containing multiple inner loops (loop is subset of other)
 * 3. Partially overlapping loops via goto
 * 4. Disjoint loops for comparison
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop is proper subset */
        
        /* Inner loop - this will be 'other' */
        for (int j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * 3;
            int e = d >> 2;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (a + b + c + d + e) & 0xFF;
        }
        
        /* No code here either to maintain subset property */
    }
    
    return result;
}

/* Function 2: Outer loop with multiple inner loops - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int outer_with_multiple_inner(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int temp = i * j;
            result += temp;
            asm volatile("" : : "r"(temp));
        }
        
        /* Some intermediate code in outer loop */
        int intermediate = i * 7;
        asm volatile("" : : "r"(intermediate));
        
        /* Second inner loop - this will be 'loop' (subset of 'other') */
        for (int k = 0; k < i + 2; ++k) {
            /* Complex body for register pressure */
            int a = i * k;
            int b = a << 2;
            int c = b % 17;
            int d = c ^ intermediate;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            result ^= d;
        }
        
        /* More code in outer loop after second inner loop */
        result += intermediate;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops via goto
 * This creates loops that intersect but neither is subset
 */
NOINLINE int overlapping_via_goto(int N) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        if (i % 3 == 0) {
            /* Jump into Loop B's body */
            goto enter_loop_b;
        }
        
        result += i * 2;
        continue;
        
    enter_loop_b:
        /* Loop B - shares this block with Loop A via goto */
        for (int j = 0; j < i + 1; ++j) {
            int a = i + j;
            int b = a * 3;
            asm volatile("" : : "r"(a), "r"(b));
            result ^= b;
            
            /* Break back to Loop A */
            if (j == i / 2) {
                break;
            }
        }
        
        /* This block belongs to both loops */
        result += 1;
    }
    
    return result;
}

/* Function 4: Mixed loop types for varied CFG structure */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        /* do-while nested inside while */
        int j = 0;
        do {
            int a = i * j;
            int b = j << i;
            int c = a ^ b;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result += c;
            j++;
        } while (j < 5);
        
        i++;
    }
    
    /* for loop after while */
    for (int k = 0; k < N / 2; ++k) {
        int temp = k * result;
        asm volatile("" : : "r"(temp));
        result ^= temp;
    }
    
    return result;
}

/* Function 5: Disjoint loops for baseline comparison */
NOINLINE int disjoint_loops(int N) {
    int result = 0;
    
    /* First independent loop */
    for (int i = 0; i < N; ++i) {
        int a = i * 3;
        int b = a % 11;
        asm volatile("" : : "r"(a), "r"(b));
        result += b;
    }
    
    /* Unrelated code between loops */
    result = (result * 13) & 0xFFF;
    
    /* Second independent loop */
    for (int j = 0; j < N / 2; ++j) {
        int c = j * result;
        int d = c >> 2;
        asm volatile("" : : "r"(c), "r"(d));
        result ^= d;
    }
    
    return result;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to create varying loop bounds */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 30;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 60) + 5;
    int N5 = (seed % 25) + 15;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= outer_with_multiple_inner(N2);
    total ^= overlapping_via_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= disjoint_loops(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 255);
    
    return 0;
}
