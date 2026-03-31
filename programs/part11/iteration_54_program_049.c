/* test_hw_doloop.c - Test program to cover hw-doloop.cc lines 429-436 */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdio.h>

/* Force ARM Thumb mode for hardware loop optimization */
#ifdef __ARM_ARCH
__attribute__((target("thumb")))
#endif
volatile int g_counter = 0;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer */
__attribute__((used, noinline))
static int perfect_nesting(int n, int m) {
    int sum = 0;
    volatile int i, j;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Add some basic blocks inside outer loop before inner loop */
        if (__builtin_expect(g_counter > 0, 0)) {
            sum += 1;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            /* Split inner loop block */
            if (__builtin_expect(j % 2 == 0, 1)) {
                sum += 1;
            }
        }
        
        /* Add some basic blocks after inner loop */
        if (__builtin_expect(i % 3 == 0, 0)) {
            sum -= 1;
        }
    }
    return sum;
}

/* Pattern B: Partially overlapping loops with shared blocks but not nested */
__attribute__((used, noinline))
static int partial_overlap(int n, int m) {
    int sum = 0;
    volatile int i = 0, j = 0;
    int flag = 1;
    
    /* First loop */
    do {
        sum += i;
        /* Shared block - executed in both loops */
        if (__builtin_expect(flag, 1)) {
            g_counter++;
        }
        
        if (i++ >= n) break;
        
        /* Conditional second loop inside first */
        if (flag) {
            j = 0;
            /* Second loop that partially overlaps */
            while (j < m) {
                sum += j;
                /* Another shared block */
                if (__builtin_expect(g_counter > 0, 1)) {
                    sum += 2;
                }
                j++;
                /* Early exit creates different block structure */
                if (j > m/2 && __builtin_expect(sum > 100, 0)) {
                    break;
                }
            }
            flag = 0;
        }
    } while (1);
    
    return sum;
}

/* Pattern C: Sequential loops with shared preheader/setup block */
__attribute__((used, noinline))
static int sequential_loops(int n, int m) {
    int sum = 0;
    volatile int i;
    
    /* Shared setup block - may be included in both loop bitmaps */
    int shared = g_counter;
    
    /* First loop */
    i = 0;
    while (i < n) {
        sum += i + shared;
        i++;
        /* Split block in first loop */
        if (__builtin_expect(i % 4 == 0, 0)) {
            shared++;
        }
    }
    
    /* Shared intermediate block */
    shared *= 2;
    
    /* Second loop - sequential but shares some blocks */
    for (i = 0; i < m; i++) {
        sum -= i - shared;
        /* Different block structure than first loop */
        if (__builtin_expect(i % 5 == 0, 1)) {
            g_counter--;
        }
    }
    
    return sum;
}

/* Pattern D: Complex nested loops with early exits and gotos */
__attribute__((used, noinline))
static int complex_nesting(int n, int m) {
    int sum = 0;
    volatile int i, j, k;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Middle loop */
        j = 0;
        while (j < m) {
            sum += i * j;
            
            /* Innermost loop with early exit */
            for (k = 0; k < 5; k++) {
                sum += k;
                if (__builtin_expect(sum > 1000, 0)) {
                    goto early_exit;
                }
                /* Create additional basic block */
                if (__builtin_expect(k % 2 == 0, 1)) {
                    sum -= 1;
                }
            }
            
            j++;
            /* Conditional continue creates different block */
            if (__builtin_expect(j % 3 == 0, 0)) {
                continue;
            }
        }
        
        early_exit:
        if (__builtin_expect(i > n/2, 0)) {
            break;
        }
    }
    
    return sum;
}

/* Pattern E: Interleaved loops using switch statement */
__attribute__((used, noinline))
static int interleaved_loops(int n, int m) {
    int sum = 0;
    volatile int i = 0, state = 0;
    
    while (i < n) {
        switch (state) {
            case 0:
                /* Loop-like structure in case 0 */
                for (int x = 0; x < 3; x++) {
                    sum += x;
                    if (__builtin_expect(x == 1, 1)) {
                        state = 1;
                    }
                }
                break;
                
            case 1:
                /* Different loop in case 1 */
                int y = 0;
                do {
                    sum -= y;
                    y++;
                    /* Shared block with case 0's loop */
                    if (__builtin_expect(g_counter > 0, 1)) {
                        sum += 10;
                    }
                } while (y < 2);
                state = 0;
                break;
        }
        i++;
    }
    
    return sum;
}

/* Main function to execute all patterns */
int main(void) {
    int result = 0;
    
    /* Execute each pattern multiple times with different parameters */
    result += perfect_nesting(10, 5);
    result += partial_overlap(8, 6);
    result += sequential_loops(7, 9);
    result += complex_nesting(5, 4);
    result += interleaved_loops(6, 3);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes for consistent behavior");
    
    return 0;
}
