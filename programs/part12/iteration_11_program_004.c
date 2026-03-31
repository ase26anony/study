/* Test program for hardware loop nesting analysis in GCC */
/* Designed to trigger bitmap_intersect_compl_p checks in hw-doloop.cc */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to prevent optimization and create side effects */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create side effects that can't be optimized away */
__attribute__((noinline)) void side_effect(int value) {
    results[counter++ % 1000] = value;
    checksum ^= value;
}

/* Function with conditional inner loop - creates partial basic block overlap */
void test_partial_overlap_loops(int n) {
    volatile int x = 0;
    
    /* Outer loop - will contain some but not all blocks of inner loops */
    for (int i = 0; i < n; i++) {
        /* Conditional creates branch - some blocks only in outer loop */
        if (i % 3 == 0) {
            /* First inner loop - fully contained in this branch */
            for (int j = 0; j < (i % 5) + 1; j++) {
                side_effect(i * 100 + j);
                x += i * j;
            }
            
            /* Additional code in same branch but outside inner loop */
            side_effect(i * 1000);
            x += rand() % 10;
        } 
        else if (i % 3 == 1) {
            /* Second inner loop - different structure, creates sibling relationship */
            for (int k = 0; k < (i % 3) + 2; k++) {
                side_effect(i * 200 + k);
                x -= i * k;
                /* Nested conditional inside inner loop */
                if (k % 2 == 0) {
                    side_effect(k * 300);
                }
            }
        }
        else {
            /* Code path without any inner loops */
            side_effect(i * 3000);
            x *= 2;
        }
        
        /* Common code after conditional - part of outer loop but not inner loops */
        checksum += x;
    }
}

/* Test with three-level nesting and varying overlap patterns */
void test_three_level_nesting(int n) {
    volatile int a = 0, b = 0, c = 0;
    
    /* Level 1: Outermost loop */
    for (int i = 0; i < n; i++) {
        side_effect(i);
        
        /* Level 2: Middle loop - partially overlaps with outer */
        for (int j = 0; j < (i % 4) + 2; j++) {
            a += i + j;
            
            /* Conditional that creates partial containment */
            if (j % 2 == (i % 2)) {
                /* Level 3: Innermost loop - fully contained in this branch */
                for (int k = 0; k < (j % 3) + 1; k++) {
                    b += i * j * k;
                    side_effect(i * j * k + 10000);
                }
            } else {
                /* Alternative path in middle loop without innermost loop */
                c += i - j;
                side_effect(i - j + 20000);
            }
            
            /* Code after if-else in middle loop */
            side_effect(a + b + c);
        }
        
        /* Additional outer loop code not in middle loop */
        checksum += a - b + c;
    }
}

/* Test with sibling inner loops inside outer loop */
void test_sibling_loops(int n) {
    volatile int x = 0;
    
    for (int i = 0; i < n; i++) {
        /* First sibling inner loop */
        if (i % 2 == 0) {
            for (int j = 0; j < (i % 3) + 1; j++) {
                x += i * j;
                side_effect(x);
            }
        }
        
        /* Code between sibling loops - part of outer but not inner loops */
        x += rand() % 100;
        
        /* Second sibling inner loop - different condition */
        if (i % 3 == 0) {
            for (int k = 0; k < (i % 4) + 2; k++) {
                x -= i * k;
                side_effect(x + 5000);
            }
        }
        
        /* More outer loop code */
        checksum ^= x;
    }
}

/* Test with loop that has early exit - creates complex control flow */
void test_early_exit_loop(int n) {
    volatile int x = 0;
    
    for (int i = 0; i < n; i++) {
        side_effect(i);
        
        /* Inner loop with early exit */
        for (int j = 0; j < 10; j++) {
            x += i * j;
            side_effect(x);
            
            /* Early exit condition - creates additional basic blocks */
            if (x > 1000) {
                x = 0;
                break;
            }
            
            /* Continue path */
            x += rand() % 10;
        }
        
        /* Outer loop continues */
        checksum += x;
    }
}

/* Main function that runs all tests */
int main() {
    /* Initialize random seed for unpredictable but deterministic behavior */
    srand(42);
    
    printf("Starting hardware loop nesting tests...\n");
    
    /* Run tests with different parameters to create various loop structures */
    test_partial_overlap_loops(50);
    printf("Test 1 complete, checksum = %d\n", checksum);
    
    test_three_level_nesting(30);
    printf("Test 2 complete, checksum = %d\n", checksum);
    
    test_sibling_loops(40);
    printf("Test 3 complete, checksum = %d\n", checksum);
    
    test_early_exit_loop(25);
    printf("Test 4 complete, checksum = %d\n", checksum);
    
    /* Final validation */
    int final_sum = 0;
    for (int i = 0; i < 1000 && i < counter; i++) {
        final_sum += results[i];
    }
    
    printf("Final validation: array sum = %d, total operations = %d\n", 
           final_sum, counter);
    
    return 0;
}
