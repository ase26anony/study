/* test_hw_doloop.c - Test program to cover hw-doloop.cc lines 429-436 */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((target("thumb")))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Split basic block to create more complex bitmap */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Another block splitter */
            if (__builtin_expect(j & 1, 1)) {
                sink = j;
            }
        }
        
        /* Early exit condition affecting bitmap */
        if (i > n/2) {
            break;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops - share some blocks but not subsets */
__attribute__((target("thumb")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Shared block: conditionally enter second loop */
        if (i % 3 == 0) {
            /* Second loop that partially overlaps */
            j = 0;
            while (j < m) {
                sum += j;
                j++;
                /* Different block not in first loop */
                if (j % 2 == 0) {
                    sink = j;
                }
            }
        }
        
        /* Block only in first loop */
        if (i % 4 == 0) {
            sink = i * 2;
        }
    } while (i < n);
    
    sink = sum;
}

/* Pattern C: Sequential loops with shared preheader/setup block */
__attribute__((target("thumb")))
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int shared = n + m;
    sink = shared;
    
    /* First loop */
    i = 0;
    while (i < n) {
        sum += i;
        i++;
        /* Block splitter */
        if (__builtin_expect((i & 3) == 0, 0)) {
            sink = i;
        }
    }
    
    /* Re-use shared variable */
    shared = sum;
    
    /* Second loop - may share setup block in bitmap */
    for (j = 0; j < m; j++) {
        sum -= j;
        /* Different control flow pattern */
        switch (j % 3) {
            case 0: sink = j; break;
            case 1: sink = j * 2; break;
            default: sink = j * 3; break;
        }
    }
    
    sink = sum + shared;
}

/* Pattern D: Complex nested loops with early exits and gotos */
__attribute__((target("thumb")))
void complex_nesting(int n, int m) {
    int i, j, k;
    int sum = 0;
    
    /* Outer loop with goto creating irregular CFG */
    for (i = 0; i < n; i++) {
        if (i % 5 == 0) {
            /* Jump to middle of inner loop structure */
            goto middle;
        }
        
        /* First inner loop */
        j = 0;
        while (j < m) {
            sum += i * j;
            j++;
        }
        
    middle:
        /* Second inner loop that overlaps with first */
        for (k = 0; k < m/2; k++) {
            sum -= k;
            if (k == i) {
                /* Early exit to outer loop */
                break;
            }
        }
        
        /* Block only reachable from goto */
        sink = i;
    }
    
    sink = sum;
}

/* Pattern E: Do-while loops with different structures */
__attribute__((used))
__attribute__((target("thumb")))
void mixed_loop_types(int n) {
    int i = 0;
    int sum = 0;
    
    /* Do-while loop */
    do {
        sum += i;
        i++;
        
        /* Nested for-loop inside do-while */
        for (int j = 0; j < 5; j++) {
            sum += j;
            /* Always true if to create block */
            if (__builtin_expect(1, 1)) {
                sink = j;
            }
        }
        
        /* While loop inside do-while */
        int k = 0;
        while (k < 3) {
            sum -= k;
            k++;
        }
        
    } while (i < n);
    
    sink = sum;
}

/* Helper to force different optimization decisions */
__attribute__((noinline))
static int helper(int x) {
    return x * 3 + 1;
}

/* Pattern F: Loops with function calls affecting block structure */
__attribute__((target("thumb")))
void loops_with_calls(int n) {
    int i, j;
    int sum = 0;
    
    /* Outer loop with call */
    for (i = 0; i < n; i++) {
        sum += helper(i);
        
        /* Inner loop without call */
        j = 0;
        while (j < 5) {
            sum += j;
            j++;
            /* Split block */
            if (j % 2) {
                sink = j;
            }
        }
    }
    
    sink = sum;
}

/* Main function to execute all patterns */
int main(void) {
    /* Small iterations to keep runtime reasonable */
    const int N = 100;
    const int M = 50;
    
    /* Execute each pattern multiple times with different parameters */
    for (int run = 0; run < 3; run++) {
        perfect_nesting(N + run, M + run);
        partial_overlap(N + run * 2, M + run);
        sequential_loops(N - run, M + run * 2);
        complex_nesting(N / (run + 1) + 10, M / (run + 1) + 10);
        mixed_loop_types(N + run * 5);
        loops_with_calls(N - run * 3);
    }
    
    /* Compile-time check to ensure compilation proceeds */
    static_assert(sizeof(int) == 4, "int must be 4 bytes for this test");
    
    return 0;
}
