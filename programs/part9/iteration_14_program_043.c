/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates specific loop structures to exercise the
 * bitmap intersection logic in hw-doloop.cc's discover_loop_hierarchy.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in bitmap logic) */
    for (i = 0; i < N; ++i) {
        /* No code here - ensures other is subset of loop */
        
        /* Inner loop (will be 'other' in bitmap logic) */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + j;
            int c = b - i;
            int d = c * a;
            result ^= (d >> 2) & 0xFF;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
        
        /* No code here either - maintains subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int reverse_nesting(int N) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in bitmap logic) */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i + j;
            int b = a * 2;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop (will be 'loop' in bitmap logic) */
        for (k = 0; k < i; ++k) {
            /* Create different register pressure pattern */
            int x = k * k;
            int y = x - i;
            int z = y >> 1;
            result ^= z;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops via goto
 * This should trigger the first condition (bitmap_intersect_p returns true)
 * but not the subset conditions
 */
NOINLINE int overlapping_loops(int N) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop */
    while (i < N) {
        int a = i * 3;
        result += a;
        
        if (i == N/2) {
            /* Jump into second loop's body */
            goto inside_second_loop;
        }
        
        i++;
    }
    
    /* Second loop - shares block via goto target */
    for (j = 0; j < N; ++j) {
    inside_second_loop:
        int b = j * 5;
        int c = b - j;
        result ^= c;
        
        asm volatile("" : : "r"(b), "r"(c));
        
        /* Only continue if we came from the goto */
        if (i < N/2) {
            if (j < N-1) continue;
        }
        break;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex control flow */
NOINLINE int mixed_loops(int N) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < N; i++) {
        int j = 0;
        
        /* do-while loop */
        do {
            int a = i + j;
            int b = a * a;
            int c = b % 256;
            result = (result + c) & 0xFF;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            j++;
        } while (j < 5);
        
        /* while loop after do-while */
        int k = 0;
        while (k < 3) {
            int d = result * k;
            result ^= d;
            asm volatile("" : : "r"(d));
            k++;
        }
    }
    
    return result;
}

/* Function 5: Sibling loops with shared header block */
NOINLINE int sibling_loops(int N) {
    int result = 0;
    int i;
    
    /* Two sequential loops that might share some CFG structure */
    for (i = 0; i < N; i++) {
        int a = i * 7;
        result += a;
        asm volatile("" : : "r"(a));
    }
    
    /* Second loop - compiler might create shared prologue/epilogue */
    for (i = N-1; i >= 0; i--) {
        int b = i * 11;
        result ^= b;
        asm volatile("" : : "r"(b));
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to create runtime-varying loop bounds */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 25) + 25;
    int N5 = (seed % 20) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_nesting(N2);
    total ^= overlapping_loops(N3);
    total ^= mixed_loops(N4);
    total ^= sibling_loops(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
