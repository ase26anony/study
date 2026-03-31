/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of loops */
static volatile int g_volatile_counter = 0;

/* Pattern A: Perfectly nested loops - inner loop bitmap is subset of outer */
__attribute__((noinline))
void perfect_nesting(int n, int m) {
    int sum = 0;
    int i, j;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        g_volatile_counter++; /* Force block creation */
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            
            /* Split inner loop block */
            if (__builtin_expect(g_volatile_counter > 0, 1)) {
                sum += 1;
            }
        }
        
        /* Another statement in outer loop */
        if (__builtin_expect(i % 2 == 0, 0)) {
            sum += 2;
        }
    }
    
    /* Use result to prevent removal */
    g_volatile_counter = sum & 1;
}

/* Pattern B: Partially overlapping loops with shared blocks but not perfect nesting */
__attribute__((noinline))
void partial_overlap(int n, int m) {
    int sum = 0;
    int i = 0, j = 0;
    
    /* Shared setup block */
    int shared = n * 2;
    
    /* First loop */
    while (i < n) {
        sum += i;
        
        /* Conditional that might execute second loop */
        if (i % 3 == 0 && j < m) {
            /* Second loop inside conditional - creates partial overlap */
            do {
                sum += j;
                j++;
                
                /* Split block in second loop */
                if (__builtin_expect(shared > 0, 1)) {
                    sum += shared;
                }
            } while (j < m && j < 5);
        }
        
        i++;
        
        /* Early exit from first loop */
        if (sum > 1000) {
            break;
        }
    }
    
    /* Continue with second loop independently */
    while (j < m) {
        sum += j * 2;
        j++;
        
        /* Another split */
        if (__builtin_expect(g_volatile_counter != 0, 0)) {
            sum -= 1;
        }
    }
    
    g_volatile_counter = sum & 1;
}

/* Pattern C: Sequential loops with shared preheader block */
__attribute__((noinline))
void sequential_loops(int n, int m) {
    int sum = 0;
    int i;
    
    /* Shared preheader block - may be included in both loop bitmaps */
    int setup = n + m;
    
    /* First loop */
    for (i = 0; i < n; i++) {
        sum += i * setup;
        
        /* Block splitter */
        if (__builtin_expect(setup > 0, 1)) {
            sum += 1;
        }
    }
    
    /* Code between loops */
    setup = sum % 10;
    
    /* Second loop - sequential but may share setup block in bitmap */
    for (i = 0; i < m; i++) {
        sum += i + setup;
        
        /* Different block structure */
        if (__builtin_expect(i % 2 == 0, 1)) {
            sum += 2;
        } else {
            sum += 3;
        }
    }
    
    g_volatile_counter = sum & 1;
}

/* Pattern D: Complex nested loops with multiple exits and irregular structure */
__attribute__((noinline))
void complex_nesting(int n, int m, int k) {
    int sum = 0;
    int i, j, l;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        /* Middle loop - not perfectly nested due to preceding statement */
        sum += i;
        
        for (j = 0; j < m; j++) {
            /* Innermost loop */
            for (l = 0; l < k; l++) {
                sum += i * j * l;
                
                /* Conditional exit from innermost */
                if (sum > 10000) {
                    goto inner_exit;
                }
            }
        inner_exit:
            /* Statement after inner loop, still in middle loop */
            sum += j;
        }
        
        /* Another inner loop at different nesting level */
        for (l = 0; l < 3; l++) {
            sum += i * l;
            
            /* Split block */
            if (__builtin_expect(l == 1, 0)) {
                sum += 100;
            }
        }
    }
    
    g_volatile_counter = sum & 1;
}

/* Pattern E: Do-while and while loops mixed */
__attribute__((noinline))
void mixed_loop_types(int n) {
    int sum = 0;
    int i = 0;
    
    /* Do-while loop */
    do {
        sum += i;
        i++;
        
        /* Nested while loop */
        int j = 0;
        while (j < 5) {
            sum += j;
            j++;
            
            /* Split block */
            if (__builtin_expect(j % 2 == 0, 1)) {
                sum += 2;
            }
        }
    } while (i < n);
    
    /* Another while loop that might share blocks */
    while (sum < 1000) {
        sum += 10;
        
        /* Early break */
        if (sum > 500) {
            break;
        }
    }
    
    g_volatile_counter = sum & 1;
}

/* Pattern F: Loops with switch statements inside */
__attribute__((noinline))
void loops_with_switch(int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        switch (i % 4) {
            case 0:
                sum += i;
                /* Fall through */
            case 1:
                sum += i * 2;
                break;
            case 2:
                /* Inner loop inside switch case */
                for (int j = 0; j < 3; j++) {
                    sum += j;
                }
                break;
            default:
                sum += i * 3;
                
                /* Another inner loop */
                int k = 0;
                while (k < 2) {
                    sum += k;
                    k++;
                }
                break;
        }
    }
    
    g_volatile_counter = sum & 1;
}

/* Main function to ensure all patterns are executed */
int main(void) {
    int iterations = 10;
    
    /* Execute each pattern multiple times */
    for (int run = 0; run < 3; run++) {
        perfect_nesting(iterations, 5);
        partial_overlap(iterations, 7);
        sequential_loops(iterations, 6);
        complex_nesting(4, 3, 2);
        mixed_loop_types(iterations);
        loops_with_switch(iterations);
        
        /* Vary parameters */
        iterations += 2;
    }
    
    /* Ensure compiler keeps all functions */
    __attribute__((used)) static void (*func_ptrs[])(void) = {
        (void (*)(void))perfect_nesting,
        (void (*)(void))partial_overlap,
        (void (*)(void))sequential_loops,
        (void (*)(void))complex_nesting,
        (void (*)(void))mixed_loop_types,
        (void (*)(void))loops_with_switch,
    };
    
    return g_volatile_counter;
}

/* Compile-time assertion to ensure compilation proceeds */
static_assert(sizeof(int) >= 2, "int size check");
