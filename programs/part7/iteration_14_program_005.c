/* test_hwloop.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c -o test_hwloop.o
 * Or for generic target: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c -o test_hwloop_executable
 * 
 * The goal is to trigger specific bitmap intersection logic in hw-doloop.cc:
 * - Condition 1: Two loops with intersecting block bitmaps
 * - Condition 2: Inner loop (other) is subset of outer loop (loop)
 * - Condition 3: Outer loop (other) contains inner loop (loop) plus additional blocks
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - inner loop is subset of outer loop
 * This should trigger: loop->loops.safe_push(other)
 * where other's blocks are subset of loop's blocks
 */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    int a, b, c, d;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* No code here - ensures inner loop starts immediately */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < i; ++j) {
            /* Create register pressure */
            a = i + j;
            b = a * 2;
            c = b - j;
            d = c ^ a;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= (a * b) >> (c & 3);
        }
        
        /* No code here - ensures inner loop ends at outer loop end */
    }
    
    return result & 0xFF;
}

/* Function 2: Outer loop contains inner loop plus extra blocks
 * This should trigger: other->loops.safe_push(loop)
 * where loop's blocks are subset of other's blocks
 */
NOINLINE int outer_contains_inner_plus(int N) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x ^ j;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
        }
        
        /* Some code between loops - ensures 'other' has blocks not in 'loop' */
        x = i * 2;
        asm volatile("" : : "r"(x));
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < i; ++k) {
            y = k * 3;
            z = y ^ i;
            asm volatile("" : : "r"(y), "r"(z));
            result ^= y + z;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops using goto
 * This ensures bitmap_intersect_p returns true
 */
NOINLINE int overlapping_loops_goto(int N) {
    int result = 0;
    int a, b;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        a = i * 2;
        
        /* Loop B with goto into Loop A's body */
        for (int j = 0; j < 5; ++j) {
            b = j * 3;
            
            if (i > N/2 && j == 2) {
                /* Jump to label inside Loop A */
                goto shared_block;
            }
            
            result += a ^ b;
        }
        
        /* This label creates a shared basic block */
        shared_block:
        a = a ^ result;
        asm volatile("" : : "r"(a));
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types - do-while inside for */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int counter = 0;
    
    /* Outer for loop */
    for (int i = 0; i < N; ++i) {
        counter = i;
        
        /* Inner do-while loop */
        do {
            int a = counter * 2;
            int b = a ^ i;
            asm volatile("" : : "r"(a), "r"(b));
            result += a - b;
            counter--;
        } while (counter > 0 && counter < 10);
        
        /* Additional computation in outer loop */
        result ^= i * 7;
    }
    
    return result & 0xFF;
}

/* Function 5: While loop following for loop with break to shared block */
NOINLINE int while_after_for(int N) {
    int result = 0;
    int i = 0;
    
    /* First loop - for loop */
    for (i = 0; i < N; i++) {
        int a = i * 3;
        asm volatile("" : : "r"(a));
        result += a;
        
        if (i == N/2) {
            /* Break to a shared block */
            goto common_exit;
        }
    }
    
    /* Second loop - while loop */
    i = 0;
    while (i < N) {
        int b = i * 5;
        asm volatile("" : : "r"(b));
        result ^= b;
        i++;
        
        if (i == N/3) {
            /* Break to same shared block */
            goto common_exit;
        }
    }
    
    common_exit:
    /* Shared basic block between both loops */
    result = result * 2;
    asm volatile("" : : "r"(result));
    
    return result & 0xFF;
}

/* Function 6: Complex nested structure with sibling loops */
NOINLINE int complex_sibling_nesting(int N) {
    int result = 0;
    
    /* Outer loop */
    for (int i = 0; i < N; ++i) {
        /* First sibling inner loop */
        for (int j = 0; j < 4; ++j) {
            int a = i * j;
            int b = a ^ 0x55;
            asm volatile("" : : "r"(a), "r"(b));
            result += a - b;
        }
        
        /* Code between sibling loops */
        int temp = i * 7;
        asm volatile("" : : "r"(temp));
        
        /* Second sibling inner loop */
        for (int k = 0; k < 3; ++k) {
            int c = i * k;
            int d = c ^ 0xAA;
            asm volatile("" : : "r"(c), "r"(d));
            result ^= c + d;
        }
        
        /* More outer loop code */
        result = (result << 1) | (result >> 31);
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 100) + 10;
    int N2 = (seed % 50) + 20;
    int N3 = (seed % 30) + 15;
    int N4 = (seed % 40) + 25;
    int N5 = (seed % 60) + 5;
    int N6 = (seed % 70) + 8;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= outer_contains_inner_plus(N2);
    total ^= overlapping_loops_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= while_after_for(N5);
    total ^= complex_sibling_nesting(N6);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 255);
    
    return 0;
}
