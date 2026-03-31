/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((target("thumb")))
void perfect_nesting(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Add conditional to create more basic blocks */
        if (__builtin_expect(i & 1, 0)) {
            sink = i;
        }
        
        /* Inner loop - perfectly nested */
        for (int j = 0; j < m; j++) {
            sum += i * j;
            /* Another conditional to split blocks */
            if (__builtin_expect(j & 1, 1)) {
                sink = j;
            }
        }
        
        /* Early exit to affect block bitmap */
        if (i > n/2) {
            break;
        }
    }
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks but not nested */
__attribute__((target("thumb")))
void partial_overlap(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* First loop */
    do {
        sum += i;
        /* Shared block - will be in both loop bitmaps */
        if (__builtin_expect(i < n/2, 1)) {
            sink = i;
        }
        i++;
    } while (i < n);
    
    /* Second loop that shares the conditional block above */
    int j = 0;
    while (j < m) {
        /* This block is also in first loop's bitmap */
        if (__builtin_expect(j < m/2, 1)) {
            sink = j;
        }
        
        /* Unique block to second loop */
        for (int k = 0; k < 3; k++) {
            sum += j * k;
        }
        j++;
    }
    
    /* Third loop that conditionally executes second loop */
    for (int x = 0; x < n; x++) {
        if (x & 1) {
            /* This creates partial overlap */
            int y = 0;
            while (y < m) {
                sum += x * y;
                y++;
            }
        } else {
            sum += x;
        }
    }
    
    sink = sum;
}

/* Pattern C: Sibling loops with shared preheader/initialization */
__attribute__((target("thumb")))
void sibling_loops(int n, int m) {
    int sum = 0;
    
    /* Shared initialization block - may be included in both loop bitmaps */
    int *arr = (int*)&sink;
    
    /* First sibling loop */
    for (int i = 0; i < n; i++) {
        sum += i;
        /* Conditional to create more blocks */
        if (__builtin_expect(i == n-1, 0)) {
            sink = i;
        }
    }
    
    /* Shared computation between loops */
    sum *= 2;
    
    /* Second sibling loop */
    int j = 0;
    while (j < m) {
        sum -= j;
        /* Different conditional structure */
        switch (j % 3) {
            case 0: sink = j; break;
            case 1: sum += j * 2; break;
            case 2: sum += j * 3; break;
        }
        j++;
    }
    
    /* Third loop with early exit that shares some blocks */
    for (int k = 0; k < n+m; k++) {
        if (k >= n) {
            break;  /* Early exit creates different block structure */
        }
        sum += arr[0];  /* Use arr to prevent removal */
    }
    
    sink = sum;
}

/* Pattern D: Complex nested structure with mixed loop types */
__attribute__((used))
__attribute__((target("thumb")))
void complex_nesting(int n) {
    int sum = 0;
    int i = 0;
    
    /* Outer while loop */
    while (i < n) {
        /* Middle for loop */
        for (int j = 0; j < i; j++) {
            /* Innermost do-while */
            int k = 0;
            do {
                sum += i * j * k;
                k++;
                
                /* Conditional continue to affect block bitmap */
                if (k & 1) continue;
                
                sum += 1;
            } while (k < 3);
            
            /* Another conditional block */
            if (__builtin_expect(j == i-1, 0)) {
                sink = j;
            }
        }
        
        /* Goto to create irregular control flow */
        if (i == n/2) {
            goto skip_part;
        }
        
        sum += i;
        
    skip_part:
        i++;
    }
    
    /* Separate loop that shares some blocks via function call simulation */
    for (int x = 0; x < n; x++) {
        /* Inline what would be a function call */
        int y = 0;
        while (y < x) {
            sum -= x * y;
            y++;
            
            /* This block structure partially overlaps with
               the while loop above due to similar patterns */
            if (y & 1) {
                sink = y;
            }
        }
    }
    
    sink = sum;
}

/* Pattern E: Loops with switch statements inside */
__attribute__((target("thumb")))
void loops_with_switch(int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        switch (i % 4) {
            case 0:
                for (int j = 0; j < 2; j++) {
                    sum += i + j;
                }
                break;
            case 1:
                sum += i * 2;
                break;
            case 2: {
                int k = 0;
                while (k < 3) {
                    sum += i * k;
                    k++;
                }
                break;
            }
            case 3:
                sum += i * 3;
                /* Nested loop inside case */
                do {
                    sum -= 1;
                } while (sum > i);
                break;
        }
    }
    
    sink = sum;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    /* Execute each pattern with different parameters
       to create varied loop structures */
    perfect_nesting(100, 50);
    partial_overlap(80, 60);
    sibling_loops(70, 40);
    complex_nesting(50);
    loops_with_switch(30);
    
    return 0;
}
