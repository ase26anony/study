/* test_hw_doloop.c
 * 
 * This program is designed to trigger specific conditions in GCC's
 * hardware loop optimization pass (hw-doloop.cc), particularly the
 * bitmap intersection logic at lines 429-436.
 *
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop
 * Or for MIPS: gcc -O3 -mips32r2 -funroll-loops -fdump-rtl-hw-doloop -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop
 */

#include <stdint.h>

/* Prevent function removal */
#define KEEP_FUNCTION __attribute__((used))

/* Use volatile to prevent loop removal */
static volatile int counter = 0;

/* Pattern A: Perfectly nested loops - inner loop is subset of outer loop */
KEEP_FUNCTION void perfect_nesting(void) {
    volatile int i, j;
    volatile int sum = 0;
    volatile int N = 100;
    volatile int M = 50;
    
    /* Outer loop */
    for (i = 0; i < N; i++) {
        /* Add some basic blocks inside outer loop but before inner loop */
        if (__builtin_expect(i & 1, 1)) {
            sum += i;
        }
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < M; j++) {
            /* Split inner loop into multiple basic blocks */
            if (__builtin_expect(j & 1, 0)) {
                sum += j * 2;
            } else {
                sum += j;
            }
        }
        
        /* More code after inner loop */
        if (__builtin_expect(i > N/2, 0)) {
            sum -= i;
        }
    }
    
    counter += sum;
}

/* Pattern B: Partially overlapping loops - share some blocks but not perfectly nested */
KEEP_FUNCTION void partial_overlap(void) {
    volatile int i, j;
    volatile int sum = 0;
    volatile int N = 100;
    volatile int M = 50;
    volatile int flag = 1;
    
    /* First loop */
    i = 0;
    while (i < N) {
        /* Shared block - both loops will execute this */
        if (__builtin_expect(flag, 1)) {
            sum += i;
            flag = !flag;
        }
        
        /* Conditional inner loop - not always executed */
        if (i % 3 == 0) {
            /* Second loop that shares the block above */
            for (j = 0; j < M; j++) {
                /* This block is only in the inner loop */
                sum += j;
                
                /* Jump back to shared code */
                if (j % 5 == 0) {
                    goto shared_code;
                }
            }
        }
        
shared_code:
        /* More shared code */
        sum += i * 2;
        i++;
    }
    
    counter += sum;
}

/* Pattern C: Sequential loops with shared preheader/setup block */
KEEP_FUNCTION void sequential_loops(void) {
    volatile int i, j;
    volatile int sum = 0;
    volatile int N = 100;
    volatile int M = 50;
    
    /* Shared setup block - may be included in both loop bitmaps */
    volatile int shared = N * 2;
    
    /* First loop */
    i = 0;
    do {
        sum += i + shared;
        if (__builtin_expect(i == N/2, 0)) {
            /* Early exit creates more complex control flow */
            break;
        }
        i++;
    } while (i < N);
    
    /* Reset shared between loops */
    shared = M * 3;
    
    /* Second loop - sequential but shares conceptual setup */
    for (j = 0; j < M; j++) {
        /* Different computation but similar structure */
        sum -= j + shared;
        
        /* Nested conditional to create more basic blocks */
        if (__builtin_expect(j & 1, 1)) {
            if (__builtin_expect(j & 2, 0)) {
                sum += j * 3;
            }
        }
    }
    
    counter += sum;
}

/* Pattern D: Complex nested loops with early exits and gotos */
KEEP_FUNCTION void complex_nesting(void) {
    volatile int i, j, k;
    volatile int sum = 0;
    volatile int N = 50;
    
    /* Triple nested loop with irregular control flow */
    for (i = 0; i < N; i++) {
        /* Middle loop with conditional execution */
        if (i % 2 == 0) {
            j = 0;
            while (j < N) {
                /* Innermost loop with early exit */
                for (k = 0; k < N; k++) {
                    sum += i + j + k;
                    
                    /* Early exit from innermost loop */
                    if (k > i) {
                        goto next_j;
                    }
                    
                    /* Another basic block splitter */
                    if (__builtin_expect(k == j, 0)) {
                        sum -= k;
                    }
                }
                
next_j:
                /* Code after inner loop in middle loop */
                sum += j * 2;
                j++;
                
                /* Break from middle loop under condition */
                if (j > i + 10) {
                    goto next_i;
                }
            }
        }
        
next_i:
        /* Code after middle loop in outer loop */
        sum += i * 3;
        
        /* Another loop at same level as outer but with overlap */
        if (i % 3 == 0) {
            volatile int m;
            for (m = 0; m < 10; m++) {
                /* This loop shares some blocks with outer loop
                   through the function prologue/epilogue */
                sum += m;
            }
        }
    }
    
    counter += sum;
}

/* Pattern E: Loops with switch statements inside */
KEEP_FUNCTION void loops_with_switch(void) {
    volatile int i, state;
    volatile int sum = 0;
    volatile int N = 100;
    
    for (i = 0; i < N; i++) {
        state = i % 4;
        
        switch (state) {
            case 0:
                /* Loop inside case 0 */
                for (volatile int j = 0; j < 10; j++) {
                    sum += j;
                }
                break;
                
            case 1:
                /* Different loop structure */
                volatile int k = 0;
                while (k < 5) {
                    sum += k + i;
                    k++;
                }
                break;
                
            case 2:
                /* Shared code block */
                sum += i * 2;
                /* Fall through */
                
            default:
                /* Another loop */
                for (volatile int m = 0; m < 3; m++) {
                    sum -= m;
                }
                break;
        }
    }
    
    counter += sum;
}

/* Helper to ensure all functions are called */
static void call_all_functions(void) {
    perfect_nesting();
    partial_overlap();
    sequential_loops();
    complex_nesting();
    loops_with_switch();
}

/* Compile-time check to ensure optimization */
#define EXPECTED_LOOPS 10
_Static_assert(EXPECTED_LOOPS > 0, "Must have positive number of loops");

/* Main function that calls all test patterns */
int main(void) {
    /* Initialize volatile counter */
    counter = 0;
    
    /* Call all functions multiple times to ensure execution */
    for (volatile int run = 0; run < 3; run++) {
        call_all_functions();
    }
    
    /* Return something based on counter to prevent dead code elimination */
    return counter > 0 ? 0 : 1;
}
