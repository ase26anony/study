/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The test creates specific loop structures to trigger bitmap intersection
 * logic in hw-doloop.cc's discover_loop_hierarchy function.
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
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < N; ++i) {
        /* Inner loop - this will be 'other' 
         * All blocks of inner loop are in outer loop
         */
        for (j = 0; j < (i % 5) + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = (i << 2) + (j << 1);
            int c = b - a;
            int d = (a * b) >> (c & 3);
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            result ^= d;
        }
        
        /* Small amount of code in outer loop but not in inner loop */
        result += i & 0xF;
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 2; ++j) {
            int a = i + j;
            int b = a * 3;
            result += b;
            asm volatile("" : : "r"(a), "r"(b));
        }
        
        /* Second inner loop - this will be 'loop' 
         * All blocks of this loop are in 'other'
         */
        for (k = 0; k < (i % 3) + 1; ++k) {
            int x = i * k;
            int y = x + k;
            int z = y ^ result;
            
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result = z;
        }
        
        /* More code in 'other' but not in 'loop' */
        result ^= i;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops using goto
 * This should trigger the first condition (bitmap_intersect_p is true)
 * but not the subset conditions
 */
NOINLINE int overlapping_loops_goto(int N) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < N; ++i) {
        int a = i * 2;
        result += a;
        
        /* Label inside loop body for goto target */
        if ((i % 7) == 0) {
            shared_label:
            result ^= 0x55;
        }
        
        asm volatile("" : : "r"(a));
    }
    
    /* Second loop - will be 'other' */
    j = N / 2;
    while (j > 0) {
        int b = j * 3;
        result -= b;
        
        /* Jump into the first loop's body */
        if ((j % 5) == 0) {
            goto shared_label;
        }
        
        asm volatile("" : : "r"(b));
        j--;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex nesting
 * Creates various loop relationships for comprehensive coverage
 */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for loop */
    for (i = 0; i < N; ++i) {
        int counter = 0;
        
        /* do-while loop - potential 'other' */
        do {
            int a = (i << counter) & 0xFF;
            int b = a ^ result;
            result = b;
            
            asm volatile("" : : "r"(a), "r"(b));
            counter++;
        } while (counter < 3);
        
        /* while loop after do-while */
        int j = 0;
        while (j < 2) {
            result += (i * j);
            asm volatile("" : : "r"(j));
            j++;
        }
    }
    
    /* Another for loop that shares some blocks via switch */
    for (i = N - 1; i >= 0; --i) {
        switch (i % 4) {
            case 0:
                result ^= 0xAA;
                break;
            case 1:
                result |= 0x55;
                /* Fall through to create shared block */
            case 2:
                result += i;
                break;
            default:
                result -= i;
        }
        
        asm volatile("" : : "r"(i));
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with break to shared code
 * Creates loops that are neither subset nor superset
 */
NOINLINE int sibling_loops_with_break(int N) {
    int result = 0;
    int i, j;
    
    /* First sibling loop */
    for (i = 0; i < N; ++i) {
        if (i == N/2) {
            /* Break to shared code block */
            goto shared_code;
        }
        
        int a = i * i;
        result += a;
        asm volatile("" : : "r"(a));
    }
    
    shared_code:
    result ^= 0x1234;
    
    /* Second sibling loop */
    for (j = 0; j < N/2; ++j) {
        if (j == N/4) {
            /* Also break to same shared code */
            goto shared_code;
        }
        
        int b = j * 3;
        result -= b;
        asm volatile("" : : "r"(b));
    }
    
    return result & 0xFF;
}

/* Main function to drive all test cases */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 30;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 60) + 5;
    int N5 = (seed % 25) + 15;
    
    /* Call all test functions */
    total_result ^= perfect_nesting(N1);
    total_result ^= loop_subset_of_other(N2);
    total_result ^= overlapping_loops_goto(N3);
    total_result ^= mixed_loop_types(N4);
    total_result ^= sibling_loops_with_break(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total_result & 0xFF);
    
    return 0;
}
