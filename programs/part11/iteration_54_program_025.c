/* test_hw_doloop.c - Test program to cover GCC hw-doloop pass bitmap intersection logic */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of loops */
volatile int g_counter = 0;
#define KEEP_ALIVE(x) do { g_counter += (int)(x); } while(0)

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((target("arch=armv7-a")))
void perfect_nesting(int n, int m) {
    int sum = 0;
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Split basic block */
        if (__builtin_expect(i & 1, 0)) {
            KEEP_ALIVE(1);
        }
        
        /* Inner loop - perfectly nested */
        for (int j = 0; j < m; j++) {
            sum += i * j;
            /* Another split to create more blocks */
            if (__builtin_expect(j & 1, 1)) {
                KEEP_ALIVE(2);
            }
        }
        
        /* Early exit possibility */
        if (i > n/2) break;
    }
    KEEP_ALIVE(sum);
}

/* Pattern B: Partially overlapping loops - share some blocks but not subsets */
__attribute__((target("arch=armv7-a")))
void partial_overlap(int n, int m) {
    int sum = 0;
    int i = 0;
    
    /* First loop */
    while (i < n) {
        sum += i;
        /* Shared block */
        if (__builtin_expect(sum & 1, 0)) {
            KEEP_ALIVE(3);
        }
        
        /* Conditional second loop inside */
        if (i % 3 == 0) {
            int j = 0;
            /* Second loop - partially overlapping */
            do {
                sum += j;
                j++;
                /* Different block not in first loop */
                if (__builtin_expect(j > m/2, 0)) {
                    KEEP_ALIVE(4);
                    break;
                }
            } while (j < m);
        }
        i++;
    }
    KEEP_ALIVE(sum);
}

/* Pattern C: Sequential loops sharing a preheader block */
__attribute__((target("arch=armv7-a")))
void sequential_loops(int n, int m) {
    int sum = 0;
    
    /* Shared preheader setup */
    int setup = n + m;
    KEEP_ALIVE(setup);
    
    /* First loop */
    for (int i = 0; i < n; i++) {
        sum += i * i;
        /* Split block */
        if (__builtin_expect(i == n-1, 0)) {
            KEEP_ALIVE(5);
        }
    }
    
    /* Shared block between loops */
    int intermediate = sum / 2;
    KEEP_ALIVE(intermediate);
    
    /* Second loop - different structure */
    int k = 0;
    while (k < m) {
        sum -= k;
        k++;
        if (__builtin_expect(k == m/2, 0)) {
            KEEP_ALIVE(6);
            continue;
        }
    }
    
    KEEP_ALIVE(sum);
}

/* Pattern D: Complex nested loops with early exits */
__attribute__((target("arch=armv7-a")))
void complex_nesting(int n, int m, int p) {
    int total = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Middle loop */
        int j = 0;
        while (j < m) {
            /* Innermost loop */
            for (int k = 0; k < p; k++) {
                total += i * j * k;
                
                /* Multiple exit conditions */
                if (total > 1000) goto cleanup;
                if (k == p/2) break;
                
                /* Split block */
                if (__builtin_expect(k & 1, 1)) {
                    KEEP_ALIVE(7);
                }
            }
            
            j++;
            if (j == m/2) continue;
        }
        
        cleanup:
        if (i == n-1) break;
    }
    
    KEEP_ALIVE(total);
}

/* Pattern E: Loops with irregular control flow using goto */
__attribute__((target("arch=armv7-a")))
void irregular_flow(int n) {
    int result = 0;
    int i = 0;
    
start_loop:
    if (i >= n) goto end;
    
    result += i;
    
    /* Conditional jump to shared block */
    if (i % 2 == 0) {
        goto shared_block;
    }
    
    /* Different path */
    result *= 2;
    goto continue_loop;
    
shared_block:
    result += 100;
    KEEP_ALIVE(8);
    
continue_loop:
    i++;
    goto start_loop;
    
end:
    KEEP_ALIVE(result);
}

/* Pattern F: Mixed loop types (for, while, do-while) */
__attribute__((target("arch=armv7-a")))
void mixed_loops(int n) {
    int acc = 0;
    
    /* for loop */
    for (int i = 0; i < n/3; i++) {
        acc += i;
    }
    
    /* while loop overlapping with previous */
    int j = 0;
    while (j < n/2) {
        acc -= j;
        j++;
        
        /* do-while inside while */
        int k = 0;
        do {
            acc += k;
            k++;
        } while (k < 5 && j < n/4);
    }
    
    KEEP_ALIVE(acc);
}

/* Main function to call all patterns */
int main(void) {
    /* Call each function with different parameters to create
       various loop execution patterns */
    perfect_nesting(100, 50);
    partial_overlap(80, 40);
    sequential_loops(60, 30);
    complex_nesting(20, 15, 10);
    irregular_flow(25);
    mixed_loops(50);
    
    /* Ensure all functions are used */
    return g_counter == 0 ? 0 : 1;
}

/* Compile-time assertion to ensure optimization */
_Static_assert(sizeof(int) == 4, "int must be 32-bit for consistent behavior");
