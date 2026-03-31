/* test_hw_doloop.c - Test program to exercise GCC's hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>

/* Prevent optimization of loops */
static volatile int sink;

/* Function 1: Perfectly nested loops - should trigger inner->outer push */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Inner loop - completely contained within outer loop */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Split basic block to create more complex bitmap */
            if (__builtin_expect(sum & 1, 0)) {
                sum += 1;
            }
        }
        
        /* Additional block in outer loop only */
        if (i % 2 == 0) {
            sum += 100;
        }
    }
    
    sink = sum;
}

/* Function 2: Partially overlapping loops using goto */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* First loop */
    while (i < n) {
        sum += i;
        i++;
        
        /* Shared block - both loops can enter here */
shared_block:
        if (sum % 3 == 0) {
            sum += 5;
        }
        
        /* Conditional entry to second loop */
        if (i == n/2) {
            /* Start second loop that shares the shared_block */
            j = 0;
            while (j < m) {
                sum += j;
                j++;
                goto shared_block;  /* Jump back to shared block */
            }
        }
    }
    
    sink = sum;
}

/* Function 3: Sequential loops with shared setup block */
__attribute__((noinline))
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    int array[100];
    
    /* Shared setup block - might be included in both loop bitmaps */
    for (i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* First loop */
    i = 0;
    do {
        sum += array[i];
        i++;
        
        /* Split block inside first loop */
        if (__builtin_expect(i % 7 == 0, 0)) {
            sum -= 1;
        }
    } while (i < n);
    
    /* Second loop - sequential but shares setup block */
    j = 0;
    while (j < m) {
        sum += array[j] * 2;
        j++;
        
        /* Different block structure in second loop */
        if (j % 3 == 0) {
            sum += array[j];
        } else {
            sum -= array[j];
        }
    }
    
    sink = sum;
}

/* Function 4: Complex nested loops with early exits */
__attribute__((noinline))
void complex_nesting(int n, int m) {
    int i, j, k;
    int sum = 0;
    
    /* Three-level nesting */
    for (i = 0; i < n; i++) {
        /* Middle loop with early exit */
        j = 0;
        while (j < m) {
            sum += i + j;
            j++;
            
            /* Early exit condition */
            if (sum > 1000) {
                break;
            }
            
            /* Innermost loop */
            for (k = 0; k < 5; k++) {
                sum += k;
                /* Conditional to split blocks */
                if (k % 2 == 0) {
                    sum += 2;
                } else {
                    sum += 1;
                }
            }
        }
        
        /* Additional outer-only block */
        if (i % 4 == 0) {
            for (j = 0; j < 3; j++) {
                sum += j * 10;
            }
        }
    }
    
    sink = sum;
}

/* Function 5: Interleaved loops using switch statement */
__attribute__((noinline))
void interleaved_loops(int n) {
    int i = 0, state = 0;
    int sum = 0;
    
    while (i < n) {
        switch (state) {
            case 0:
                /* Loop-like behavior in case 0 */
                for (int j = 0; j < 3; j++) {
                    sum += j;
                    i++;
                    if (i >= n) goto done;
                }
                state = 1;
                break;
                
            case 1:
                /* Different loop in case 1 */
                int k = 0;
                while (k < 2) {
                    sum += k * 10;
                    k++;
                    i++;
                    if (i >= n) goto done;
                }
                state = 2;
                break;
                
            case 2:
                /* Direct computation */
                sum += 100;
                i++;
                state = 0;
                break;
        }
    }
done:
    sink = sum;
}

/* Main function to call all test patterns */
int main(void) {
    /* Call functions with different parameters to create
       different loop execution patterns */
    perfect_nesting(100, 50);
    partial_overlap(100, 50);
    sibling_loops(100, 50);
    complex_nesting(20, 10);
    interleaved_loops(100);
    
    return 0;
}
