/* test_hw_doloop.c
 * Test program to cover uncovered lines in GCC's hw-doloop pass
 * Lines 429-436 in hw-doloop.cc
 */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer */
__attribute__((noinline, target("thumb")))
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
        
        /* Post-inner loop block still in outer loop */
        if (__builtin_expect(sum > 1000, 0)) {
            sink = sum;
        }
    }
    
    /* Prevent dead code elimination */
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks */
__attribute__((noinline, target("thumb")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* First loop */
    do {
        /* Shared block - both loops will include this if we jump back */
        if (__builtin_expect(i & 3, 0)) {
            sink = i;
        }
        
        sum += i;
        i++;
        
        /* Conditional entry to second loop */
        if (i < n/2) {
            /* Second loop that overlaps but isn't nested */
            j = 0;
            while (j < m) {
                /* This block is only in second loop */
                sum += j;
                j++;
                
                /* Jump back to shared block - creates overlap */
                if (j < m/2) {
                    goto shared_block;
                }
            }
        }
        
        shared_block:
        /* This label creates shared basic block between loops */
        sink = sum;
        
    } while (i < n);
    
    sink = sum;
}

/* Pattern C: Sequential loops with shared preheader */
__attribute__((noinline, target("thumb")))
void sequential_loops(int n, int m) {
    int sum = 0;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int shared_counter = 0;
    sink = shared_counter;
    
    /* First loop */
    for (int i = 0; i < n; i++) {
        /* Conditional that creates multiple blocks */
        if (__builtin_expect(i % 2 == 0, 1)) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        /* Early exit creates more block variety */
        if (i > n/2) break;
    }
    
    /* Reset shared counter between loops */
    shared_counter = 1;
    sink = shared_counter;
    
    /* Second loop - sequential but may share preheader block */
    for (int j = 0; j < m; j++) {
        /* Different structure than first loop */
        int k = j;
        do {
            sum += k;
            k--;
        } while (k > 0 && sum < 1000);
        
        /* Another conditional block */
        if (__builtin_expect(j % 3 == 0, 0)) {
            sink = j;
        }
    }
    
    sink = sum;
}

/* Pattern D: Complex nested loops with early exits */
__attribute__((noinline, target("thumb")))
void complex_nesting(int n, int m, int p) {
    int total = 0;
    
    /* Level 1 loop */
    for (int i = 0; i < n; i++) {
        /* Level 2 loop */
        int j = 0;
        while (j < m) {
            /* Level 3 loop - innermost */
            for (int k = 0; k < p; k++) {
                total += i + j + k;
                
                /* Early exit from middle loop */
                if (total > 5000) {
                    goto next_j;
                }
                
                /* Conditional to create blocks */
                if (__builtin_expect(k % 4 == 0, 1)) {
                    sink = k;
                }
            }
            
            next_j:
            j++;
            
            /* Another conditional */
            if (__builtin_expect(j % 5 == 0, 0)) {
                sink = j;
            }
        }
        
        /* Early exit from outer loop */
        if (total > 10000) {
            break;
        }
    }
    
    sink = total;
}

/* Pattern E: Loops with switch statements inside */
__attribute__((noinline, target("thumb")))
void loops_with_switch(int n) {
    int state = 0;
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Switch creates multiple basic blocks within loop */
        switch (state) {
            case 0:
                result += i;
                state = 1;
                break;
            case 1:
                result += i * 2;
                state = 2;
                break;
            case 2:
                result += i * 3;
                state = 0;
                /* Nested loop inside switch case */
                for (int j = 0; j < 3; j++) {
                    result += j;
                    if (__builtin_expect(j == 1, 0)) {
                        sink = j;
                    }
                }
                break;
        }
        
        /* Another loop that overlaps with switch cases */
        int k = 0;
        while (k < 2) {
            result -= k;
            k++;
        }
    }
    
    sink = result;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    /* Call each pattern with different parameters
     * to create diverse loop structures */
    perfect_nesting(100, 50);
    partial_overlap(200, 100);
    sequential_loops(150, 75);
    complex_nesting(50, 30, 20);
    loops_with_switch(100);
    
    /* Compile-time check to ensure compilation proceeds */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return 0;
}
