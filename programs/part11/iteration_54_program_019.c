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
        /* Add some basic blocks inside outer loop but outside inner */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Split inner loop block */
            if (__builtin_expect(j & 1, 1)) {
                sink = j;
            }
        }
        
        /* More code in outer loop after inner */
        if (__builtin_expect(i % 3 == 0, 0)) {
            sink = sum;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks but not nested */
__attribute__((used, noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    int shared = n * m;
    sink = shared;
    
    /* First loop */
    while (i < n) {
        sum += i;
        /* Shared computation block */
        if (__builtin_expect(shared > 0, 1)) {
            sink = shared;
            shared--;
        }
        
        /* Conditional jump to second loop's body */
        if (i == n/2) {
            /* Enter second loop's computation without proper nesting */
            for (j = 0; j < m; j++) {
                sum -= j;
                /* Shared block inside both loops' execution paths */
                if (__builtin_expect(shared > 0, 1)) {
                    sink = j;
                }
            }
        }
        i++;
    }
    
    /* Second loop (partially overlaps with first via shared block) */
    j = 0;
    do {
        sum += j * 2;
        /* This block is only in second loop */
        if (__builtin_expect(j % 5 == 0, 0)) {
            sink = j * 2;
        }
        j++;
    } while (j < m);
    
    sink = sum;
}

/* Pattern C: Sibling loops with shared preheader block */
__attribute__((used, noinline))
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block - may be included in both loops' bitmaps */
    int setup = n + m;
    sink = setup;
    
    /* First sibling loop */
    for (i = 0; i < n; i++) {
        sum += i;
        /* Block only in first loop */
        if (__builtin_expect(i % 2 == 0, 1)) {
            sink = i;
        }
    }
    
    /* Shared intermediate block */
    sink = sum;
    
    /* Second sibling loop */
    for (j = 0; j < m; j++) {
        sum -= j;
        /* Block only in second loop */
        if (__builtin_expect(j % 3 == 0, 0)) {
            sink = j;
        }
    }
    
    /* Force different exit conditions */
    if (sum < 0) {
        for (i = 0; i < 5; i++) {
            sum += i;
        }
    }
}

/* Pattern D: Complex nested loops with early exits affecting bitmap shapes */
__attribute__((used, noinline))
void complex_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Middle loop with early exit */
        j = 0;
        while (j < m) {
            sum += i + j;
            
            /* Early exit creates separate exit block */
            if (__builtin_expect(sum > 1000, 0)) {
                break;
            }
            
            /* Innermost loop */
            for (l = 0; l < k; l++) {
                sum += l;
                /* Split innermost block */
                if (__builtin_expect(l % 4 == 0, 1)) {
                    sink = l;
                }
            }
            
            j++;
        }
        
        /* Another inner loop at same level */
        if (i % 2 == 0) {
            for (l = 0; l < 3; l++) {
                sum -= l;
            }
        }
    }
}

/* Pattern E: Loops with irregular control flow using goto */
__attribute__((used, noinline))
void irregular_loops(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop with goto creating overlapping block structure */
    while (i < n) {
        sum += i;
        
        if (i == n/3) {
            /* Jump into what looks like another loop's body */
            goto shared_block;
        }
        
        i++;
        continue;
        
    shared_block:
        /* This block is "shared" between loop iterations */
        sum *= 2;
        sink = sum;
        
        /* Nested loop entered irregularly */
        for (j = 0; j < 5; j++) {
            sum += j;
        }
        
        i++;
    }
    
    /* Another loop that shares the setup */
    for (j = 0; j < 10; j++) {
        sum -= j;
        if (j == 5) {
            goto shared_block;  /* Creates overlap */
        }
    }
}

/* Pattern F: Multiple loops in switch cases */
__attribute__((used, noinline))
void switch_loops(int mode, int n) {
    int i, sum = 0;
    
    switch (mode) {
        case 0:
            /* Loop in first case */
            for (i = 0; i < n; i++) {
                sum += i;
            }
            /* Shared block after loop */
            sink = sum;
            break;
            
        case 1:
            /* Different loop sharing the sink block */
            i = n;
            while (i-- > 0) {
                sum += i * 2;
            }
            sink = sum;  /* Shared with case 0 */
            break;
            
        case 2:
            /* Nested loops in case */
            for (i = 0; i < n; i++) {
                for (int j = 0; j < 3; j++) {
                    sum += i * j;
                }
                sink = i;  /* Block inside outer loop only */
            }
            break;
    }
}

/* Main function to ensure all patterns are executed */
int main(void) {
    int iterations = 100;
    
    /* Execute each pattern multiple times */
    for (int run = 0; run < 10; run++) {
        perfect_nesting(iterations, 50);
        partial_overlap(iterations, 40);
        sibling_loops(iterations, 30);
        complex_nesting(20, 15, 10);
        irregular_loops(iterations);
        switch_loops(run % 3, iterations);
    }
    
    /* Compile-time check to ensure loops aren't optimized away */
    static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return sink != 0;
}
