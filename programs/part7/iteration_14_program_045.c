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
    int result = 0;
    volatile int sink; /* Prevent dead code elimination */
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (int i = 0; i < N; ++i) {
        /* Inner loop (will be 'other' in the analysis) */
        for (int j = 0; j < 5; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            result ^= d;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
        /* No code here ensures loop's blocks are superset of other's blocks */
    }
    
    sink = result;
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    volatile int sink;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a + 1;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        for (int k = 0; k < 4; ++k) {
            /* This loop's blocks are subset of outer loop's blocks */
            int x = i + k;
            int y = x * 3;
            int z = y - x;
            result ^= z;
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    sink = result;
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    volatile int sink;
    
    /* First loop (will be 'loop' in the analysis) */
    for (int i = 0; i < N; ++i) {
        int a = i * 2;
        
    shared_block:
        /* This label creates a shared basic block */
        int b = a + 1;
        result += b;
        
        /* Second loop (will be 'other' in the analysis) */
        int j = 0;
        while (j < 3) {
            int c = b + j;
            result ^= c;
            
            /* Jump to shared block in first loop */
            if (j == 1 && i < N/2) {
                j++;
                goto shared_block; /* Creates intersection */
            }
            
            j++;
            asm volatile("" : : "r"(c));
        }
        
        asm volatile("" : : "r"(a), "r"(b));
    }
    
    sink = result;
    return result & 0xFF;
}

/* Function 4: Mixed loop types with break to shared block */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    volatile int sink;
    
    /* do-while inside for */
    for (int i = 0; i < N; ++i) {
        int a = i;
        
        /* do-while loop */
        int j = 0;
        do {
            int b = a + j;
            
            /* Break to a label in the outer loop */
            if (j == 2 && i > N/2) {
                result += b * 2;
                break; /* Will jump to code after the do-while */
            }
            
            result ^= b;
            j++;
            asm volatile("" : : "r"(b));
        } while (j < 4);
        
        /* Code here is in outer loop but not in do-while */
        int c = a * 3;
        result += c;
        asm volatile("" : : "r"(c));
    }
    
    sink = result;
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    volatile int sink;
    
    /* Outer loop */
    for (int i = 0; i < N; ++i) {
        /* First sibling loop */
        int j = 0;
        while (j < 3) {
            int a = i * j;
            result += a;
            j++;
            asm volatile("" : : "r"(a));
        }
        
        /* Second sibling loop (shares no blocks with first) */
        for (int k = 0; k < 4; ++k) {
            int b = i + k;
            int c = b * 2;
            result ^= c;
            asm volatile("" : : "r"(b), "r"(c));
        }
        
        /* Third loop that partially overlaps with outer via continue */
        for (int m = 0; m < 2; ++m) {
            if (m == 0) {
                /* This continue jumps to outer loop increment */
                continue;
            }
            int d = i * m;
            result += d;
            asm volatile("" : : "r"(d));
        }
    }
    
    sink = result;
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 60) + 5;
    int N5 = (seed % 70) + 8;
    
    /* Call all functions to ensure they're compiled and executed */
    int r1 = perfect_nesting(N1);
    int r2 = reverse_nesting(N2);
    int r3 = overlapping_loops(N3);
    int r4 = mixed_loops(N4);
    int r5 = sibling_loops(N5);
    
    /* Generate side effect to prevent dead code elimination */
    int total = r1 + r2 + r3 + r4 + r5;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
