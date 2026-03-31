/* test_hw_doloop.c
 * Test program to cover uncovered lines in GCC's hw-doloop.cc
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop
 */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of loops */
static volatile int g_volatile_counter = 0;

/* Function 1: Perfectly nested loops (Pattern A)
 * Outer loop contains inner loop completely
 * Should trigger: bitmap_intersect_p = true, bitmap_intersect_compl_p(other, loop) = false
 * Result: inner loop pushed into outer's loops vector
 */
__attribute__((target("thumb")))
void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        g_volatile_counter++;
        
        /* Split basic block */
        if (__builtin_expect(1, 1)) {
            /* Inner loop - perfectly nested */
            for (j = 0; j < m; j++) {
                sum += i * j;
                /* Early exit to affect block bitmap */
                if (j == m/2 && i == n/2) {
                    break;
                }
            }
        }
        
        /* Another basic block in outer loop */
        if (i % 2 == 0) {
            sum += i;
        }
    }
    
    /* Use result to prevent dead code elimination */
    g_volatile_counter = sum & 1;
}

/* Function 2: Partially overlapping loops (Pattern B)
 * Two loops share some blocks but neither contains the other completely
 * Should trigger: bitmap_intersect_p = true, both bitmap_intersect_compl_p checks = true
 * Result: Neither safe_push occurs, continue after first if
 */
__attribute__((target("arch=armv7-a")))
void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared preheader block */
    int shared = n + m;
    
    /* First loop with complex control flow */
    while (i < n) {
        /* Block shared with second loop */
        if (shared > 0) {
            sum += shared;
            shared--;
        }
        
        /* Unique to first loop */
        if (i % 3 == 0) {
            sum += i * 2;
        }
        
        i++;
        
        /* Conditional entry to second loop */
        if (i == n/2) {
            /* Start second loop from within first */
            j = 0;
            goto start_second_loop;
        }
    }
    
    goto after_loops;
    
start_second_loop:
    /* Second loop that overlaps with first */
    do {
        /* Block shared with first loop */
        if (shared > 0) {
            sum += shared;
            shared--;
        }
        
        /* Unique to second loop */
        if (j % 4 == 0) {
            sum -= j;
        }
        
        j++;
        
        /* Early exit creates different block structure */
        if (j > m/3) {
            break;
        }
        
        /* Another unique block */
        sum += j * 3;
    } while (j < m);
    
after_loops:
    /* Post-loop shared block */
    sum += i + j;
    g_volatile_counter = sum & 1;
}

/* Function 3: Sibling loops with shared preheader (Pattern C)
 * Two sequential loops sharing setup code
 * May trigger one of the safe_push cases depending on bitmap representation
 */
#pragma GCC target("arch=armv7-a")
void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block - may be included in both loop bitmaps */
    int setup = n * 2;
    if (__builtin_expect(setup > 0, 1)) {
        sum = setup;
    }
    
    /* First loop */
    for (i = 0; i < n; i++) {
        sum += i;
        /* Split block to create more complex bitmap */
        if (i == n - 1) {
            sum += 1000;
        }
    }
    
    /* Shared block between loops */
    sum += 500;
    
    /* Second loop - sequential but shares preheader */
    j = 0;
    while (j < m) {
        sum -= j;
        j++;
        
        /* Different block structure than first loop */
        if (j % 5 == 0) {
            sum += j * 5;
        }
    }
    
    g_volatile_counter = sum & 1;
}

/* Function 4: Complex nested structure with multiple levels
 * Creates hierarchy of loops for tree construction
 */
__attribute__((target("thumb")))
void multi_level_nesting(int n, int m, int k) {
    int a, b, c;
    int sum = 0;
    
    /* Level 1: Outer loop */
    for (a = 0; a < n; a++) {
        /* Level 2: Middle loop */
        for (b = 0; b < m; b++) {
            /* Unique block for middle loop */
            sum += a * b;
            
            /* Level 3: Inner loop */
            c = 0;
            while (c < k) {
                sum += c;
                c++;
                
                /* Innermost unique block */
                if (c == k/2) {
                    sum += 999;
                }
            }
            
            /* Another middle loop block */
            if (b % 2 == 0) {
                sum -= a;
            }
        }
        
        /* Outer loop block not in inner loops */
        sum += a * 100;
    }
    
    /* Separate disjoint loop at same level as outer */
    for (a = 0; a < 10; a++) {
        sum += a * 2;
    }
    
    g_volatile_counter = sum & 1;
}

/* Function 5: Loops with switch statements inside
 * Creates complex basic block patterns
 */
void loops_with_switch(int n) {
    int i = 0;
    int sum = 0;
    
    while (i < n) {
        switch (i % 4) {
            case 0:
                sum += i;
                /* Fall through */
            case 1:
                sum += i * 2;
                break;
            case 2:
                /* Nested loop inside switch case */
                for (int j = 0; j < 3; j++) {
                    sum += j;
                }
                break;
            case 3:
                sum += i * 3;
                /* Another control flow split */
                if (i > n/2) {
                    sum += 100;
                }
                break;
        }
        i++;
    }
    
    g_volatile_counter = sum & 1;
}

/* Main function to ensure all code is executed */
int main(void) {
    int test_size = 100;
    
    /* Execute all functions to generate profile data */
    perfect_nesting(test_size, test_size/2);
    partial_overlap(test_size, test_size/3);
    sibling_loops(test_size, test_size/4);
    multi_level_nesting(50, 30, 20);
    loops_with_switch(test_size);
    
    /* Compile-time check to ensure optimization */
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    return g_volatile_counter;
}
