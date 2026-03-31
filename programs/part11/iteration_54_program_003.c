/* test_hw_loops.c - Test program to cover hw-doloop.cc bitmap intersection logic */

#include <stdint.h>

/* Prevent optimization of empty loops */
static volatile int sink;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Add some basic blocks inside outer but outside inner */
        if (__builtin_expect(i & 1, 0)) {
            sum += 1;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Split inner loop block */
            if (__builtin_expect(j & 1, 0)) {
                sum += 1;
            }
        }
        
        /* More outer loop blocks */
        if (__builtin_expect(i % 3 == 0, 0)) {
            sum -= 1;
        }
    }
    
    sink = sum;
}

/* Pattern B: Partially overlapping loops with shared blocks but not nested */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared setup block */
    int shared = n + m;
    
    /* First loop */
    while (i < n) {
        sum += i;
        
        /* Shared block between loops */
        if (shared > 0) {
            sum += shared;
            shared--;
        }
        
        /* Conditional second loop inside first */
        if (i % 2 == 0) {
            /* Second loop that shares the 'shared' block */
            j = 0;
            do {
                sum += j;
                j++;
                
                /* This block is only in second loop */
                if (j % 3 == 0) {
                    sum += 3;
                }
            } while (j < m && i < n);  /* Depends on outer loop condition too */
        }
        
        i++;
        
        /* Block only in first loop */
        if (i % 4 == 0) {
            sum *= 2;
        }
    }
    
    sink = sum;
}

/* Pattern C: Sequential loops with shared preheader/setup */
__attribute__((noinline))
void sequential_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared setup block - will be in both loop bitmaps if not separated */
    int setup = n * 2;
    
    /* First loop */
    i = 0;
    if (setup > 0) {
        do {
            sum += i;
            i++;
            
            /* Early exit creates different block structure */
            if (i > n/2 && __builtin_expect(setup > 10, 0)) {
                break;
            }
        } while (i < n);
    }
    
    /* Reset setup for second loop */
    setup = m * 2;
    
    /* Second loop - shares structure but different blocks */
    j = 0;
    if (setup > 0) {
        while (j < m) {
            sum -= j;
            j++;
            
            /* Different condition structure */
            switch (j % 3) {
                case 0: sum += 5; break;
                case 1: sum += 3; break;
                default: sum += 1; break;
            }
        }
    }
    
    sink = sum;
}

/* Pattern D: Complex nested loops with early exits */
__attribute__((noinline))
void complex_nesting(int n, int m, int k) {
    int i, j, l;
    int sum = 0;
    
    /* Three-level nesting with variations */
    for (i = 0; i < n; i++) {
        /* Middle loop with early exit */
        j = 0;
        while (j < m) {
            sum += i * j;
            
            /* Innermost loop */
            for (l = 0; l < k; l++) {
                sum += l;
                
                /* Early exit from middle loop */
                if (sum > 1000) {
                    goto middle_exit;
                }
                
                /* Split block in innermost */
                if (l % 2 == 0) {
                    sum += 2;
                }
            }
            
            j++;
            
            /* Another early exit point */
            if (j > m/2 && i > n/2) {
                break;
            }
        }
    middle_exit:
        /* Block after middle loop exit */
        sum += i;
    }
    
    sink = sum;
}

/* Pattern E: Loops with irreducible control flow */
__attribute__((noinline))
void irreducible_flow(int n) {
    int i = 0;
    int sum = 0;
    
    /* Create irreducible region with loops */
    if (n > 0) {
        goto start_loop;
    }
    
    alternate_path:
        sum += 100;
        i++;
    
    start_loop:
    while (i < n) {
        sum += i;
        
        /* Conditional goto creates overlapping loop regions */
        if (i % 3 == 0) {
            goto alternate_path;
        }
        
        i++;
        
        /* Another loop-like region */
        if (i < n/2) {
            int j = 0;
            do {
                sum += j;
                j++;
            } while (j < 5 && i < n);
        }
    }
    
    sink = sum;
}

/* Main function to ensure all patterns are used */
int main() {
    /* Call each pattern with different parameters */
    perfect_nesting(100, 50);
    partial_overlap(100, 50);
    sequential_loops(100, 50);
    complex_nesting(10, 20, 30);
    irreducible_flow(100);
    
    return sink != 0;
}
