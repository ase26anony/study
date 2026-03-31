/* test_hw_doloop.c - Test program for GCC hardware loop optimization pass */
/* Compile with: gcc -O2 -march=armv7-a -fdump-rtl-all -fprofile-arcs -ftest-coverage test_hw_doloop.c -o test_hw_doloop */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of loops */
static volatile int g_counter = 0;
#define NOOPT __attribute__((optimize("O0")))

/* Function 1: Perfectly nested loops (Pattern A)
   Outer loop fully contains inner loop */
NOOPT void perfect_nesting(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Outer loop */
    for (i = 0; i < n; i++) {
        g_counter++; /* Add basic block inside outer but outside inner */
        
        /* Inner loop - perfectly nested */
        for (j = 0; j < m; j++) {
            sum += i * j;
            if (__builtin_expect(j & 1, 0)) {
                /* Split basic block inside inner loop */
                sum += 1;
            }
        }
        
        /* Another basic block in outer loop after inner */
        if (__builtin_expect(i & 1, 1)) {
            sum += 2;
        }
    }
    
    /* Use result to prevent dead code elimination */
    g_counter = sum & 1;
}

/* Function 2: Partially overlapping loops (Pattern B)
   Two loops that share some blocks but aren't perfectly nested */
NOOPT void partial_overlap(int n, int m) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Shared preheader block */
    int shared = n + m;
    
    /* First loop with early exit */
    while (i < n) {
        sum += i;
        
        /* Conditional block that might execute second loop */
        if (i > n/2 && j < m) {
            /* Start second loop inside first loop's body */
            do {
                sum += j;
                j++;
                
                /* Shared computation */
                if (__builtin_expect(shared > 10, 1)) {
                    sum += shared;
                }
                
                /* Early exit from do-while */
                if (j > m/2) break;
            } while (j < m);
        }
        
        i++;
        
        /* Another shared block */
        if (__builtin_expect(sum & 1, 0)) {
            shared--;
        }
    }
    
    /* Second loop might continue or restart */
    while (j < m) {
        sum += j * 2;
        j++;
        
        /* Block not in first loop */
        if (__builtin_expect(j == m-1, 0)) {
            sum += 100;
        }
    }
    
    g_counter = sum & 1;
}

/* Function 3: Sibling loops with shared preheader (Pattern C) */
NOOPT void sibling_loops(int n, int m) {
    int i, j;
    int sum = 0;
    
    /* Shared preheader block - will be in both loop bitmaps */
    int setup = n * 2;
    
    /* First sibling loop */
    for (i = 0; i < n; i++) {
        sum += i + setup;
        
        /* Unique block to first loop */
        if (__builtin_expect(i == n-1, 0)) {
            sum += 999;
        }
    }
    
    /* Shared block between loops */
    setup = m * 3;
    
    /* Second sibling loop */
    j = 0;
    do {
        sum += j + setup;
        j++;
        
        /* Complex if to create multiple basic blocks */
        if (__builtin_expect(j < m/2, 1)) {
            if (__builtin_expect(j % 3 == 0, 0)) {
                sum += 5;
            }
        }
    } while (j < m);
    
    g_counter = sum & 1;
}

/* Function 4: Complex nested structure with mixed loop types */
NOOPT void complex_nesting(int n, int m, int k) {
    int a = 0, b = 0, c = 0;
    int sum = 0;
    
    /* Outer while loop */
    while (a < n) {
        /* Middle for loop */
        for (b = 0; b < m; b++) {
            /* Innermost do-while with early exit */
            c = 0;
            do {
                sum += a * b + c;
                c++;
                
                /* Conditional break creates separate block */
                if (c >= k/2 && __builtin_expect(a > b, 0)) {
                    sum += 50;
                    break;
                }
                
                /* Another split block */
                if (__builtin_expect(c % 7 == 0, 1)) {
                    sum += 7;
                }
            } while (c < k);
            
            /* Block in middle loop but not in innermost */
            if (__builtin_expect(b == m-1, 0)) {
                sum += 1000;
            }
        }
        
        a++;
        
        /* Block in outer loop only */
        if (__builtin_expect(a == n-1, 0)) {
            sum += 5000;
        }
    }
    
    /* Additional disjoint loop at same level */
    for (int x = 0; x < 5; x++) {
        sum += x;
        /* This loop shares no blocks with the nested loops above */
    }
    
    g_counter = sum & 1;
}

/* Function 5: Irregular control flow with goto creating overlapping regions */
NOOPT void irregular_loops(int n) {
    int i = 0, j = 0;
    int sum = 0;
    
    /* Loop with goto creating overlapping block structure */
    for (i = 0; i < n; i++) {
        sum += i;
        
        if (__builtin_expect(i % 3 == 0, 0)) {
            /* Jump to a block that's part of another loop-like region */
            j = 0;
            shared_block:
            sum += j;
            j++;
            
            if (j < 5) {
                goto shared_block; /* Creates loop via goto */
            }
            
            /* This block is shared between the for-loop and goto-loop */
            sum += 10;
        }
        
        /* Another overlapping region */
        if (i % 2 == 0) {
            int k = 0;
            while (k < 3) {
                sum += k;
                k++;
                /* This block might be considered part of both loops */
                if (__builtin_expect(k == 2, 1)) {
                    sum += 20;
                }
            }
        }
    }
    
    g_counter = sum & 1;
}

/* Main function to ensure all functions are called */
int main(void) {
    /* Call each function with different parameters to create
       different control flow patterns */
    perfect_nesting(100, 50);
    partial_overlap(100, 50);
    sibling_loops(100, 50);
    complex_nesting(20, 15, 10);
    irregular_loops(50);
    
    /* Prevent dead code elimination */
    if (g_counter == 0) {
        return 1;
    }
    
    return 0;
}

/* Compile-time assertion to ensure compilation proceeds */
static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");
