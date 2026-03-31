/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop should be subset of outer */
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
            /* Early exit to affect block structure */
            if (j == m/2 && (i & 1)) {
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
__attribute__((target("thumb")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    if (n > 0 && m > 0) {
        sink = n + m;
    }
    
    /* First loop */
    do {
        sum += i;
        i++;
        
        /* Conditional inner loop - creates partial overlap */
        if (i % 3 == 0) {
            /* This creates a loop that shares some blocks but not all */
            for (j = 0; j < (m % 5); j++) {
                sum -= j;
                /* Split block */
                if (__builtin_expect(j & 1, 0)) {
                    sink = j;
                }
            }
        }
    } while (i < n);
    
    /* Second loop that overlaps with first through shared condition */
    j = 0;
    while (j < m) {
        /* Reuse same condition block as first loop */
        if (j % 3 == 0) {
            sum += j * 2;
        }
        j++;
        
        /* Early exit creates different block structure */
        if (sum > 1000) {
            break;
        }
    }
    
    sink = sum;
}

/* Pattern C: Sibling loops with shared preheader */
__attribute__((target("thumb")))
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block */
    int shared = n + m;
    sink = shared;
    
    /* First sibling loop */
    for (i = 0; i < n; i++) {
        sum += i;
        /* Split block to create distinct bitmap */
        if (__builtin_expect(i == n/2, 0)) {
            sink = i;
        }
    }
    
    /* Shared middle block */
    if (sum > 0) {
        shared = sum;
    }
    
    /* Second sibling loop - shares preheader and possibly middle block */
    for (j = 0; j < m; j++) {
        sum -= j;
        /* Different split condition */
        if (__builtin_expect(j == m/3, 0)) {
            sink = j;
        }
    }
    
    sink = sum + shared;
}

/* Pattern D: Complex nested structure with multiple exits */
__attribute__((target("thumb")))
void complex_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Triple nesting */
    for (i = 0; i < n; i++) {
        /* Middle loop with early exit */
        j = 0;
        while (j < m) {
            sum += i * j;
            
            /* Innermost loop */
            for (l = 0; l < k; l++) {
                sum -= l;
                /* Multiple exits from innermost */
                if (l == k/2 && (i & 1)) {
                    goto inner_exit;
                }
                if (sum < -1000) {
                    break;
                }
            }
        inner_exit:
            j++;
            
            /* Conditional continue affects block structure */
            if (j % 4 == 0) {
                continue;
            }
            
            sum += 1;
        }
        
        /* Another loop at same level as while */
        for (l = 0; l < i; l++) {
            sum += l;
            /* This creates overlapping but not subset relationship */
            if (l == i/2) {
                sink = l;
            }
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with irregular control flow using goto */
__attribute__((target("thumb")))
void irregular_loops(int n) {
    int i = 0;
    int sum = 0;
    
    /* Loop with goto creating overlapping blocks */
start_loop:
    if (i >= n) goto end_loop;
    
    sum += i;
    i++;
    
    /* Conditional jump to shared block */
    if (i % 2 == 0) {
        goto shared_block;
    }
    
    /* Another mini-loop inside */
    int j = 0;
    while (j < 3) {
        sum -= j;
        j++;
        if (j == 2) {
            goto shared_block;
        }
    }
    
    goto start_loop;
    
shared_block:
    sink = i;
    if (i < n/2) {
        goto start_loop;
    }
    
end_loop:
    sink = sum;
}

/* Pattern F: Mixed loop types with volatile to prevent removal */
__attribute__((used))
__attribute__((target("thumb")))
void mixed_loops(int n) {
    volatile int count = n;
    int sum = 0;
    
    /* do-while */
    do {
        sum += count;
        count--;
        
        /* Nested for */
        for (int i = 0; i < 5; i++) {
            sum += i;
            /* Always true if to split blocks */
            if (__builtin_expect(1, 1)) {
                sink = i;
            }
        }
    } while (count > 0);
    
    /* while loop sharing some blocks */
    while (sum > 0) {
        sum -= 2;
        /* Reuse same condition pattern */
        if (__builtin_expect(sum & 1, 0)) {
            sink = sum;
        }
    }
}

/* Main function to ensure all functions are called */
int main(void) {
    /* Call each function with different parameters to create
       different control flow patterns */
    perfect_nesting(100, 50);
    partial_overlap(100, 50);
    sibling_loops(100, 50);
    complex_nesting(10, 20, 30);
    irregular_loops(100);
    mixed_loops(50);
    
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}
