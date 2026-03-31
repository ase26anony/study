/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 7;
}

/* Function with side effects */
void record_value(int idx, int val) {
    results[idx % 1000] = val;
    checksum ^= val;
}

/* Test case 1: Outer loop with inner loop in conditional branch
 * Creates partial overlap where inner loop blocks are subset of outer loop blocks
 * but outer loop has additional blocks not in inner loop
 */
void test_partial_overlap_1(int n) {
    for (int i = 0; i < n; i++) {
        /* Code before inner loop - creates blocks in outer but not inner */
        record_value(counter++, i * 3);
        
        /* Conditional that determines whether inner loop executes */
        if (get_condition(i, 0) > 2) {
            /* Inner loop - fully contained in this branch */
            for (int j = 0; j < i + 5; j++) {
                record_value(counter++, i * j + 7);
                /* Additional conditional inside inner loop */
                if (j % 3 == 0) {
                    record_value(counter++, j * 11);
                }
            }
        } else {
            /* Alternative branch - creates blocks in outer but not inner */
            record_value(counter++, i * 100);
            for (int k = 0; k < 3; k++) {
                record_value(counter++, i + k * 50);
            }
        }
        
        /* Code after conditional - also in outer but not inner */
        record_value(counter++, i * 2 + 1);
    }
}

/* Test case 2: Two inner loops in same outer loop with different conditions
 * Creates sibling loops that partially overlap with each other
 */
void test_sibling_loops(int n) {
    for (int i = 0; i < n; i++) {
        /* Common prefix code */
        int base = i * 10;
        record_value(counter++, base);
        
        /* First inner loop conditionally */
        if (i % 4 == 0) {
            for (int j = 0; j < 8; j++) {
                record_value(counter++, base + j);
                /* Nested deeper loop */
                for (int k = 0; k < 2; k++) {
                    record_value(counter++, (base + j) * k);
                }
            }
        }
        
        /* Middle code between inner loops */
        record_value(counter++, base + 999);
        
        /* Second inner loop conditionally */
        if (i % 3 == 1) {
            for (int j = 5; j < 12; j++) {
                record_value(counter++, base - j);
                /* Different control flow pattern */
                if (j % 2 == 0) {
                    record_value(counter++, j * 77);
                } else {
                    record_value(counter++, j * 33);
                }
            }
        }
        
        /* Common suffix code */
        record_value(counter++, base * 2);
    }
}

/* Test case 3: Three-level nesting with varying containment
 * Tests bitmap_intersect_p and bitmap_intersect_compl_p for multiple levels
 */
void test_three_level_nesting(int n) {
    for (int level1 = 0; level1 < n; level1++) {
        /* Outer loop code not in inner loops */
        record_value(counter++, level1 * 1000);
        
        /* Middle loop - sometimes executes, sometimes not */
        if (level1 % 2 == 0) {
            for (int level2 = 0; level2 < level1 + 3; level2++) {
                /* Code in middle loop but not in innermost */
                record_value(counter++, level1 * 100 + level2 * 10);
                
                /* Innermost loop with its own conditions */
                if (level2 % 3 != 0) {
                    for (int level3 = 0; level3 < 4; level3++) {
                        record_value(counter++, level1 + level2 + level3);
                        /* Conditional inside innermost */
                        if ((level1 + level2 + level3) % 5 == 0) {
                            record_value(counter++, 9999);
                        }
                    }
                } else {
                    /* Alternative path in middle loop */
                    record_value(counter++, 5555);
                }
                
                /* More code in middle loop after innermost */
                record_value(counter++, level2 * 111);
            }
        }
        
        /* More outer loop code */
        record_value(counter++, level1 * 7);
    }
}

/* Test case 4: Complex overlapping with early exits and continues
 * Creates more complex control flow graphs
 */
void test_complex_control_flow(int n) {
    for (int i = 0; i < n; i++) {
        record_value(counter++, i);
        
        /* Early continue sometimes */
        if (i % 7 == 0) {
            record_value(counter++, 777);
            continue;
        }
        
        /* Loop with break condition */
        for (int j = 0; j < 10; j++) {
            record_value(counter++, i * 10 + j);
            
            if (j == i % 5) {
                record_value(counter++, 888);
                break;
            }
            
            /* Another conditional nested loop */
            if (j % 2 == 0) {
                for (int k = 0; k < 3; k++) {
                    record_value(counter++, i * 100 + j * 10 + k);
                }
            }
        }
        
        /* Final code with another conditional loop */
        if (i % 3 == 0) {
            for (int j = 0; j < 4; j++) {
                record_value(counter++, i * 3 + j);
            }
        }
    }
}

/* Test case 5: Switch statement with loops in different cases
 * Creates disjoint sets of blocks that partially overlap
 */
void test_switch_with_loops(int n) {
    for (int i = 0; i < n; i++) {
        record_value(counter++, i * 10);
        
        switch (i % 4) {
            case 0:
                /* Loop in case 0 */
                for (int j = 0; j < 5; j++) {
                    record_value(counter++, i * 20 + j);
                }
                break;
                
            case 1:
                /* Different loop in case 1 */
                for (int j = 0; j < 3; j++) {
                    record_value(counter++, i * 30 + j);
                    for (int k = 0; k < 2; k++) {
                        record_value(counter++, i * 40 + j * 10 + k);
                    }
                }
                break;
                
            case 2:
                /* No loop in case 2 */
                record_value(counter++, i * 50);
                break;
                
            case 3:
                /* Another loop pattern in case 3 */
                for (int j = 0; j < 4; j++) {
                    record_value(counter++, i * 60 + j);
                }
                break;
        }
        
        /* Common code after switch */
        record_value(counter++, i * 70);
    }
}

int main() {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < 1000; i++) {
        results[i] = 0;
    }
    
    counter = 0;
    checksum = 0;
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases with different parameters
     * to create various loop nesting patterns
     */
    test_partial_overlap_1(20);
    printf("Test 1 complete, checksum: %d\n", checksum);
    
    test_sibling_loops(15);
    printf("Test 2 complete, checksum: %d\n", checksum);
    
    test_three_level_nesting(10);
    printf("Test 3 complete, checksum: %d\n", checksum);
    
    test_complex_control_flow(25);
    printf("Test 4 complete, checksum: %d\n", checksum);
    
    test_switch_with_loops(20);
    printf("Test 5 complete, checksum: %d\n", checksum);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < 1000; i++) {
        final_sum += results[i];
    }
    
    printf("Final array sum: %d\n", final_sum);
    printf("Total operations recorded: %d\n", counter);
    
    return 0;
}
