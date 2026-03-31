/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * or: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage -march=native
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Create register pressure and prevent optimization */
#define KEEP(i) asm volatile("" : : "r"(i))
#define KEEP_VAR(v) asm volatile("" : : "r"(v))

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int seed = N; /* Prevent constant propagation */
    int limit = (seed % 50) + 10;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < limit; ++i) {
        /* No code here ensures loop blocks are exactly outer header + inner loop */
        
        /* Inner loop - this will be 'other' (subset of loop's blocks) */
        for (int j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * a;
            result ^= d;
            KEEP(a); KEEP(b); KEEP(c); KEEP(d);
        }
        
        /* No code here either - ensures perfect nesting */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 40) + 15;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < limit; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int a = i + j;
            int b = a * 2;
            result += b;
            KEEP(a); KEEP(b);
        }
        
        /* Second inner loop - this will be 'loop' (subset of other's blocks) */
        for (int k = 0; k < i + 2; ++k) {
            int x = k * i;
            int y = x >> 2;
            int z = y + result;
            result = z ^ k;
            KEEP(x); KEEP(y); KEEP(z);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto (Condition 1 - intersection) */
NOINLINE int partial_overlap_goto(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 30) + 20;
    
    /* Loop A - will be 'loop' */
    for (int i = 0; i < limit; ++i) {
        int a = i * 3;
        
    shared_block: /* Label creates shared basic block */
        result += a;
        KEEP(a);
        
        /* Loop B - will be 'other' */
        for (int j = 0; j < 5; ++j) {
            int b = j * 2;
            result ^= b;
            
            if (j == 3 && i < limit/2) {
                /* Jump to shared block in loop A */
                goto shared_block;
            }
            
            KEEP(b);
        }
        
        int c = result * i;
        result = c & 0xFFFF;
        KEEP(c);
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex CFG */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 25) + 25;
    
    /* for loop */
    for (int i = 0; i < limit; ++i) {
        int a = i;
        
        /* do-while loop inside for */
        int j = 0;
        do {
            int b = a + j;
            result += b;
            KEEP(b);
            j++;
        } while (j < 3);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 2) {
            int c = result - k;
            result = c ^ a;
            KEEP(c);
            k++;
        }
    }
    
    /* Another loop that shares some blocks via break */
    for (int x = 0; x < limit/2; ++x) {
        int d = x * x;
        
        for (int y = 0; y < 4; ++y) {
            int e = d + y;
            result ^= e;
            
            if (y == 2 && result > 1000) {
                /* Break to outer loop's continuation */
                break;
            }
            KEEP(e);
        }
        
        result += d;
        KEEP(d);
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with shared header-like structure */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 35) + 10;
    
    int init = limit * 2;
    KEEP_VAR(init);
    
    /* Two sequential loops that might share preheader */
    for (int i = 0; i < limit; ++i) {
        int a = i + init;
        int b = a * 3;
        result += b;
        KEEP(a); KEEP(b);
    }
    
    for (int j = 0; j < limit; ++j) {
        int c = j * result;
        int d = c >> 2;
        result ^= d;
        KEEP(c); KEEP(d);
    }
    
    /* Nested variant */
    for (int outer = 0; outer < 5; ++outer) {
        /* Empty outer loop body except for inner loop */
        for (int inner = 0; inner < outer + 2; ++inner) {
            int e = outer * inner;
            result += e;
            KEEP(e);
        }
    }
    
    return result & 0xFF;
}

/* Main function to drive all test cases */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int base = argc;
    int N1 = (base % 100) + 20;
    int N2 = (base % 80) + 30;
    int N3 = (base % 60) + 40;
    int N4 = (base % 70) + 25;
    int N5 = (base % 90) + 15;
    
    /* Call all test functions */
    total_result ^= perfect_nesting(N1);
    total_result ^= reverse_nesting(N2);
    total_result ^= partial_overlap_goto(N3);
    total_result ^= mixed_loop_types(N4);
    total_result ^= sibling_loops(N5);
    
    /* Ensure result is used to prevent dead code elimination */
    printf("Result: %d\n", total_result & 0xFF);
    
    return 0;
}
