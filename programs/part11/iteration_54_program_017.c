/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop completely contained in outer */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Add some basic blocks inside outer but outside inner */
        if (__builtin_expect(i % 2 == 0, 1)) {
            sum += i;
        }
        
        /* Perfectly nested inner loop */
        for (j = 0; j < m; j++) {
            /* Split inner loop into multiple basic blocks */
            if (__builtin_expect(j % 3 == 0, 0)) {
                sum += j * 2;
            } else {
                sum += j;
            }
        }
        
        /* More blocks in outer after inner */
        sum -= i;
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks but not perfect nesting */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    int shared_counter = 0;
    
    /* First loop with early exit */
    while (i < n) {
        /* Shared block - will be in both loop bitmaps */
        shared_counter++;
        sum += i;
        
        if (__builtin_expect(i == n/2, 0)) {
            break;  /* Creates additional exit block */
        }
        
        /* Conditional second loop inside first */
        if (i % 3 == 0) {
            /* Start of second loop that partially overlaps */
            j = 0;
            do {
                /* This block is only in second loop */
                sum += j * 3;
                j++;
                
                /* Shared block again */
                shared_counter--;
            } while (j < m && shared_counter > 0);
        }
        
        i++;
    }
    
    /* Continue with second part of second loop */
    while (j < m) {
        /* Blocks only in second loop continuation */
        sum -= j;
        j++;
    }
    
    sink = sum + shared_counter;
}

/* Pattern C: Sequential loops with shared preheader/setup block */
__attribute__((noinline))
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int setup = n * 2;
    
    /* First loop */
    i = 0;
    while (i < n) {
        if (__builtin_expect(setup > 0, 1)) {
            sum += i + setup;
        }
        i++;
    }
    
    /* Shared intermediate block */
    setup /= 2;
    
    /* Second loop - shares setup block */
    for (j = 0; j < m; j++) {
        /* Different structure than first loop */
        switch (j % 4) {
            case 0: sum += j * 2; break;
            case 1: sum += j * 3; break;
            case 2: sum += j * 4; break;
            default: sum += j; break;
        }
        
        /* Include setup in second loop's computation */
        if (setup > 0) {
            sum -= setup;
        }
    }
    
    sink = sum;
}

/* Pattern D: Complex nested loops with multiple exits and continues */
__attribute__((noinline))
void complex_nesting(int n, int m, int k) {
    int a, b, c;
    int sum = 0;
    
    /* Outer loop */
    for (a = 0; a < n; a++) {
        /* Middle loop with continue */
        b = 0;
        while (b < m) {
            if (__builtin_expect(b % 5 == 0, 0)) {
                b++;
                continue;  /* Creates additional edges */
            }
            
            /* Innermost loop with break */
            for (c = 0; c < k; c++) {
                sum += a * b * c;
                
                /* Early exit from innermost */
                if (c == k/2) {
                    sum += 1000;
                    break;
                }
            }
            
            b++;
        }
        
        /* Another inner loop at same level */
        for (c = 0; c < a; c++) {
            /* This creates overlapping but not contained relationship
               with the middle loop above */
            sum -= c;
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with goto creating irregular control flow */
__attribute__((noinline))
void irregular_loops(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop with goto into another loop */
    while (i < n) {
        if (i == n/3) {
            goto shared_block;
        }
        sum += i;
        i++;
    }
    
    i = 0;
    
another_loop:
    while (i < n/2) {
        sum += i * 2;
        
shared_block:
        /* This block is shared between both loops due to goto */
        sum += 1;
        
        if (j++ > 10) {
            goto loop_end;
        }
        i++;
    }
    
loop_end:
    sink = sum;
}

/* Pattern F: Mixed loop types (do-while, for, while) with volatile */
__attribute__((noinline))
void mixed_loops(int n) {
    volatile int count = 0;
    int i, j;
    int sum = 0;
    
    /* do-while loop */
    i = 0;
    do {
        sum += i;
        
        /* Nested for loop */
        for (j = 0; j < 5; j++) {
            /* Use volatile to prevent optimization */
            count = j;
            sum += count;
        }
        
        i++;
    } while (i < n);
    
    /* while loop with condition that uses result */
    while (sum > 0) {
        sum--;
        if (sum < n) {
            /* Another nested loop */
            for (i = 0; i < 3; i++) {
                sum += i;
            }
        }
    }
    
    sink = sum + count;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    /* Call each pattern with different parameters to create
       different control flow shapes */
    perfect_nesting(100, 50);
    partial_overlap(100, 50);
    sequential_loops(100, 50);
    complex_nesting(10, 20, 30);
    irregular_loops(100);
    mixed_loops(50);
    
    return 0;
}
