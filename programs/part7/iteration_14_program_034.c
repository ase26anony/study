/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * For best results, also use: -funroll-loops -fpeel-loops
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
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* No code here to ensure inner loop is subset */
        
        /* Inner loop - this will be 'other' (subset of outer) */
        for (int j = 0; j < (N - i); ++j) {
            a = i + j;
            b = a * 2;
            c = b - i;
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result ^= (a * b) >> (c & 7);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: loop is subset of other */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            x = i * j;
            y = x + global_seed;
            asm volatile("" : : "r"(x), "r"(y));
            result += x - y;
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < (N / 2); ++k) {
            z = i * k;
            /* Create register pressure */
            int t1 = z * 3;
            int t2 = t1 + k;
            int t3 = t2 - i;
            int t4 = t3 * t1;
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
            result ^= t4;
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops using goto */
NOINLINE int overlapping_with_goto(int N) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        int val = i * 2;
        
        /* Loop B - partially overlaps with Loop A */
        for (int j = 0; j < N; ++j) {
            if (j == i && i > N/2) {
                /* Jump into Loop A's body, creating intersection */
                goto inside_loop_a;
            }
            result += j * val;
        }
        
        continue;
        
    inside_loop_a:
        /* This label is inside Loop A but reachable from Loop B */
        result -= val;
        /* Use asm to prevent optimization */
        asm volatile("" : : "r"(val));
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int counter = 0;
    
    /* Outer for loop */
    for (int i = 0; i < N; ++i) {
        /* Inner do-while loop */
        int j = 0;
        do {
            int a = i + j;
            int b = a * a;
            int c = b % 17;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result += c;
            j++;
        } while (j < 5);
        
        /* Another while loop in sequence */
        int k = 0;
        while (k < 3) {
            result ^= (i * k);
            k++;
            /* Create more basic blocks with conditional */
            if (k == 2) {
                result += global_seed;
            }
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with shared blocks via switch */
NOINLINE int sibling_loops_with_switch(int N) {
    int result = 0;
    
    /* First loop */
    for (int i = 0; i < N; ++i) {
        switch (i % 4) {
            case 0:
                result += i * 2;
                break;
            case 1:
                result -= i;
                break;
            default:
                result ^= i;
        }
    }
    
    /* Second loop that shares the switch dispatch code */
    for (int j = N-1; j >= 0; --j) {
        /* Same switch structure creates shared basic blocks */
        switch (j % 4) {
            case 0:
                result += j * 3;
                break;
            case 1:
                result -= j * 2;
                break;
            default:
                result ^= (j + 1);
        }
    }
    
    return result & 0xFF;
}

/* Function 6: Complex nested structure with break to outer label */
NOINLINE int complex_nested_with_break(int N) {
    int result = 0;
    
    outer_loop:
    for (int i = 0; i < N; ++i) {
        /* Middle loop */
        for (int j = 0; j < N; ++j) {
            /* Innermost loop */
            for (int k = 0; k < N; ++k) {
                result += i * j * k;
                
                /* Conditional break to outer loop */
                if (result > 1000 && k == N/2) {
                    result /= 2;
                    /* This break exits the middle loop, not outer */
                    break;
                }
                
                /* Another condition that continues outer loop */
                if (result < 0) {
                    result = -result;
                    goto outer_loop;
                }
            }
            
            /* Additional computation in middle loop */
            result ^= j;
        }
        
        /* Code in outer loop after inner loops */
        result += i * global_seed;
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N = (seed % 100) + 10;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= loop_subset_of_other(N);
    total ^= overlapping_with_goto(N);
    total ^= mixed_loop_types(N);
    total ^= sibling_loops_with_switch(N);
    total ^= complex_nested_with_break(N);
    
    /* Ensure result is used to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    /* Additional runs with different N to exercise different paths */
    for (int i = 0; i < 3; i++) {
        N = (global_seed + i) % 50 + 5;
        total += perfect_nesting(N);
        total += loop_subset_of_other(N);
    }
    
    printf("Final result: %d\n", total & 0xFF);
    
    return 0;
}
