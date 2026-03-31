/* test_hwloop.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures with specific block relationships
 * to trigger the bitmap intersection logic in discover_loop_hierarchy:
 * 1. Loops with intersecting block bitmaps
 * 2. Perfectly nested loops (inner loop blocks subset of outer loop blocks)
 * 3. Sibling loops where one is a subset of another
 * 4. Partially overlapping loops
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Force hardware loop optimization */
#ifdef __ARM_ARCH
#define TARGET_LOOP __attribute__((target("arch=armv8-a+lse")))
#else
#define TARGET_LOOP
#endif

/* Prevent dead code elimination */
static volatile int global_seed = 42;

/* Function 1: Perfectly nested loops - inner loop is subset of outer loop
 * This should trigger: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)
 * where 'other' (inner) is subset of 'loop' (outer)
 */
NOINLINE TARGET_LOOP
int perfect_nesting(int N) {
    int result = 0;
    volatile int prevent_opt = 0;
    
    /* Outer loop - will be 'loop' in the hierarchy */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop is perfect subset */
        
        /* Inner loop - will be 'other' in the hierarchy */
        for (int j = 0; j < (N - i); ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
            prevent_opt = result;
        }
        
        /* No code here either - inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: Sibling loops where second loop is subset of first's blocks
 * Outer loop contains two inner loops in sequence.
 * First inner loop creates blocks in outer that are not in second inner loop.
 * This should trigger: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)
 * where 'loop' (second inner) is subset of 'other' (outer)
 */
NOINLINE TARGET_LOOP
int sibling_subset(int N) {
    int result = 0;
    volatile int temp = 0;
    
    /* Outer loop - will be 'other' in the hierarchy */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in outer not in second inner */
        for (int j = 0; j < 3; ++j) {
            int x = i * j;
            int y = x + j;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
            temp = result;
        }
        
        /* Some intermediate code in outer loop */
        int intermediate = i * 2;
        asm volatile("" : : "r"(intermediate));
        
        /* Second inner loop - will be 'loop' in the hierarchy */
        for (int k = 0; k < 2; ++k) {
            int a = i + k;
            int b = a * 3;
            asm volatile("" : : "r"(a), "r"(b));
            result ^= (a * b);
            temp = result;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto
 * Creates loops that share some blocks but not all
 * Should trigger the first if condition (intersection) but not the subset conditions
 */
NOINLINE TARGET_LOOP
int partial_overlap(int N) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        int a = i * 2;
        
        /* Loop B - partially overlaps with Loop A via goto */
        for (int j = 0; j < (N / 2); ++j) {
            int b = j * 3;
            
            if (i + j > N / 2) {
                /* Jump into Loop A's body */
                goto shared_block;
            }
            
            result += b;
            asm volatile("" : : "r"(b));
        }
        
        /* This label is inside Loop A but reachable from Loop B */
        shared_block:
        result ^= a;
        asm volatile("" : : "r"(a));
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE TARGET_LOOP
int mixed_loops(int N) {
    int result = 0;
    int counter = N;
    
    /* Outer for loop */
    for (int i = 0; i < N && counter > 0; ++i) {
        /* Inner do-while loop */
        int j = 0;
        do {
            /* Create complex register pressure */
            int r1 = i + j;
            int r2 = r1 * r1;
            int r3 = r2 >> 2;
            int r4 = r3 - j;
            int r5 = r4 * i;
            
            asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5));
            
            result += r5;
            j++;
        } while (j < 5);
        
        /* While loop after do-while */
        int k = 0;
        while (k < 3) {
            result ^= (i * k);
            asm volatile("" : : "r"(k));
            k++;
        }
        
        counter--;
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with multiple exits */
NOINLINE TARGET_LOOP
int complex_exits(int N) {
    int result = 0;
    
    /* Outer loop with multiple inner loops */
    for (int i = 0; i < N; ++i) {
        /* First inner - simple */
        for (int j = 0; j < 4; ++j) {
            result += i * j;
            asm volatile("" : : "r"(j));
        }
        
        /* Conditional middle loop */
        if (i % 3 == 0) {
            int k = 0;
            while (k < i % 5 + 2) {
                result ^= k;
                asm volatile("" : : "r"(k));
                k++;
                
                if (result > 1000) {
                    /* Early exit creates interesting CFG */
                    goto early_exit;
                }
            }
        }
        
        /* Another inner loop */
        for (int m = 0; m < 2; ++m) {
            int calc = (i + m) * (i - m);
            result += calc;
            asm volatile("" : : "r"(calc));
        }
        
        early_exit:;
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent optimization */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= sibling_subset(N2);
    total ^= partial_overlap(N3);
    total ^= mixed_loops(N4);
    total ^= complex_exits(N5);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
