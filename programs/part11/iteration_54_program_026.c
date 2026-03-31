/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((used, noinline))
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
            /* Early exit to affect loop structure */
            if (j == m/2) {
                break;
            }
        }
        
        /* Another split in outer loop */
        if (__builtin_expect(sum > 1000, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks */
__attribute__((used, noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared preheader block */
    int shared = n + m;
    sink = shared;
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Conditional entry to second loop's region */
        if (i % 3 == 0) {
            /* This block is shared between both loops */
            sink = i;
            
            /* Start second loop from within first */
            j = 0;
            while (j < m) {
                sum += j;
                j++;
                /* Break early sometimes */
                if (j == m/2 && (i % 2 == 0)) {
                    goto skip_rest;
                }
            }
skip_rest:
            ;
        }
        
        /* More code in first loop */
        if (__builtin_expect(sum < 0, 0)) {
            sink = -sum;
        }
    } while (i < n);
    
    sink = sum;
}

/* Pattern C: Sequential loops with shared setup block */
__attribute__((used, noinline))
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int setup = n * 2;
    sink = setup;
    
    /* First loop */
    i = 0;
    while (i < n) {
        sum += i;
        i++;
        /* Split block */
        if (__builtin_expect(i == n/2, 0)) {
            sink = i;
        }
    }
    
    /* Shared intermediate block */
    sink = sum;
    
    /* Second loop - shares setup block */
    j = 0;
    do {
        sum -= j;
        j++;
        
        /* Nested mini-loop inside second loop */
        int k = 0;
        while (k < 3) {
            sum += k;
            k++;
        }
    } while (j < m);
    
    sink = sum;
}

/* Pattern D: Complex nested loops with multiple exits */
__attribute__((used, noinline))
void complex_nesting(int n, int m, int p) {
    int i, j, k;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Middle loop */
        for (j = 0; j < m; j++) {
            /* Multiple conditions to split blocks */
            if (__builtin_expect(j % 2 == 0, 1)) {
                sink = j;
            }
            
            /* Innermost loop */
            k = 0;
            while (k < p) {
                sum += i * j * k;
                k++;
                
                /* Early exit from innermost */
                if (k == p/2 && (i + j) % 3 == 0) {
                    goto next_middle;
                }
            }
            
            /* Label for goto target */
            next_middle:
            if (__builtin_expect(sum > 10000, 0)) {
                sink = sum >> 8;
            }
        }
        
        /* Another inner loop at same level */
        for (j = m; j > 0; j--) {
            sum -= j;
            /* Conditional continue */
            if (j % 5 == 0) continue;
            sum += i;
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with irregular control flow using goto */
__attribute__((used, noinline))
void irregular_loops(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop with goto creating overlapping regions */
start_outer:
    if (i >= n) goto end;
    
    sum += i;
    i++;
    
    /* Sometimes jump to inner loop */
    if (i % 4 == 0) {
        j = 0;
        /* Inner loop region */
    start_inner:
        if (j >= 5) goto after_inner;
        sum += j;
        j++;
        
        /* Jump back to outer loop from inner */
        if (j == 3 && i % 3 == 0) {
            goto start_outer;
        }
        goto start_inner;
    }
    
after_inner:
    /* More outer loop code */
    if (__builtin_expect(sum < 0, 0)) {
        sink = -sum;
        goto start_outer;
    }
    
    goto start_outer;
    
end:
    sink = sum;
}

/* Pattern F: Mixed loop types (for, while, do-while) */
__attribute__((used, noinline))
void mixed_loops(int n) {
    int i, count;
    int sum = 0;
    
    /* do-while loop */
    i = 0;
    do {
        sum += i;
        i++;
        
        /* while loop inside */
        count = 0;
        while (count < 2) {
            sum += count;
            count++;
        }
    } while (i < n);
    
    /* for loop */
    for (i = n; i > 0; i--) {
        sum -= i;
        
        /* Another do-while inside */
        count = 0;
        do {
            sum += count * 2;
            count++;
        } while (count < 3);
    }
    
    sink = sum;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    /* Small iteration counts for quick execution */
    perfect_nesting(10, 5);
    partial_overlap(8, 6);
    sequential_loops(7, 4);
    complex_nesting(5, 4, 3);
    irregular_loops(6);
    mixed_loops(5);
    
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) == 4, "int must be 32-bit");
    
    return 0;
}
