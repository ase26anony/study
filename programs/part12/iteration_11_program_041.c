/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to prevent optimization and create side effects */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 3;
}

/* Function with side effects */
void record_value(int idx, int val) {
    results[idx % 1000] = val;
    checksum ^= val;
}

/* Test case 1: Outer loop with two inner loops that partially overlap */
void test_partial_overlap_nested() {
    int i, j, k;
    
    /* Outer loop - will contain blocks not in inner loops */
    for (i = 0; i < 50; ++i) {
        /* Branch 1: Contains first inner loop */
        if (get_condition(i, 0) == 0) {
            /* Inner loop 1 - fully contained in this branch */
            for (j = 0; j < 30; ++j) {
                record_value(counter++, i * 100 + j);
                
                /* Additional conditional inside inner loop 
                 * creates more basic blocks */
                if (j % 7 == 0) {
                    results[500] = i * j;
                }
            }
        } 
        /* Branch 2: Contains second inner loop with different structure */
        else if (get_condition(i, 0) == 1) {
            /* Inner loop 2 - different iteration count */
            for (k = 0; k < 25; ++k) {
                record_value(counter++, i * 200 + k);
                
                /* Nested conditional creates partial overlap */
                if (k % 5 == 0) {
                    for (int m = 0; m < 3; ++m) {
                        results[600 + m] = i * k * m;
                    }
                } else {
                    results[700] = i + k;
                }
            }
        }
        /* Branch 3: No inner loop - creates blocks outside inner loops */
        else {
            record_value(counter++, i * 300);
            results[800] = i * i;
        }
    }
}

/* Test case 2: Three-level nesting with varying containment */
void test_three_level_nesting() {
    int a, b, c;
    
    /* Level 1: Outer loop */
    for (a = 0; a < 40; ++a) {
        /* Conditional that sometimes skips the inner loops */
        if (a % 4 != 0) {
            /* Level 2: Middle loop - partially overlaps with outer */
            for (b = 0; b < 20; ++b) {
                /* Code executed in middle but not in inner */
                record_value(counter++, a * 1000 + b * 100);
                
                /* Level 3: Innermost loop - fully contained in middle */
                if (b % 3 == 0) {
                    for (c = 0; c < 10; ++c) {
                        record_value(counter++, a * 1000 + b * 100 + c);
                        
                        /* Conditional creates blocks in innermost only */
                        if (c % 2 == 0) {
                            results[900] = a * b * c;
                        }
                    }
                } else {
                    /* Alternative path in middle loop */
                    results[950] = a + b;
                }
            }
        } else {
            /* Outer loop blocks not in any inner loop */
            record_value(counter++, a * 9999);
        }
    }
}

/* Test case 3: Sibling loops with partial overlap through shared outer */
void test_sibling_loops() {
    int x, y, z;
    
    /* Outer loop containing two sibling inner loops */
    for (x = 0; x < 35; ++x) {
        /* Pre-inner loop code in outer */
        int temp = x * 11;
        
        /* First sibling loop */
        if (temp % 2 == 0) {
            for (y = 0; y < 15; ++y) {
                record_value(counter++, x * 100 + y);
                
                /* Code in first sibling only */
                if (y % 4 == 0) {
                    results[300] = x * y;
                }
            }
        }
        
        /* Code between sibling loops (in outer, not in either inner) */
        results[400] = temp;
        
        /* Second sibling loop with different structure */
        if (temp % 3 == 0) {
            for (z = 0; z < 12; ++z) {
                record_value(counter++, x * 200 + z);
                
                /* Nested condition in second sibling */
                if (z % 3 == 0) {
                    for (int w = 0; w < 2; ++w) {
                        results[500 + w] = x * z * w;
                    }
                }
            }
        } else {
            /* Alternative path in outer loop */
            results[600] = x * x;
        }
    }
}

/* Test case 4: Complex diamond-shaped control flow with loops */
void test_diamond_nesting() {
    int p, q, r;
    
    for (p = 0; p < 30; ++p) {
        /* Diamond 1 */
        if (p % 5 == 0) {
            for (q = 0; q < 10; ++q) {
                record_value(counter++, p * 50 + q);
                
                /* Diamond inside inner loop */
                if (q % 2 == 0) {
                    for (r = 0; r < 5; ++r) {
                        results[100 + r] = p * q * r;
                    }
                } else {
                    results[200] = p + q;
                }
            }
        } else if (p % 5 == 1) {
            /* Different inner loop structure */
            for (q = 5; q < 15; ++q) {
                record_value(counter++, p * 60 + q);
            }
        } else if (p % 5 == 2) {
            /* Loop with early exit */
            for (q = 0; q < 8; ++q) {
                record_value(counter++, p * 70 + q);
                if (q == 4) break;
            }
        } else {
            /* No inner loop at all */
            record_value(counter++, p * 80);
        }
    }
}

/* Test case 5: Interleaved loops with shared conditions */
void test_interleaved_loops() {
    int m, n;
    
    /* First outer loop */
    for (m = 0; m < 25; ++m) {
        if (m % 3 == 0) {
            /* Inner loop that will be analyzed with second outer */
            for (n = 0; n < 8; ++n) {
                record_value(counter++, m * 1000 + n);
                results[750] = m * n;
            }
        }
    }
    
    /* Second outer loop sharing some basic blocks */
    for (m = 10; m < 30; ++m) {
        if (m % 4 == 0) {
            /* Same inner loop structure but different bounds */
            for (n = 5; n < 12; ++n) {
                record_value(counter++, m * 2000 + n);
                results[850] = m + n;
            }
        } else {
            /* Different code path */
            results[950] = m * 2;
        }
    }
}

int main() {
    /* Initialize random seed for unpredictable but reproducible conditions */
    srand(42);
    
    printf("Starting hardware loop analysis test...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_partial_overlap_nested();
    printf("Completed test 1, checksum: %d\n", checksum);
    
    test_three_level_nesting();
    printf("Completed test 2, checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Completed test 3, checksum: %d\n", checksum);
    
    test_diamond_nesting();
    printf("Completed test 4, checksum: %d\n", checksum);
    
    test_interleaved_loops();
    printf("Completed test 5, checksum: %d\n", checksum);
    
    /* Final verification */
    int final_check = 0;
    for (int i = 0; i < 100; ++i) {
        final_check += results[i];
    }
    
    printf("Final check: %d (counter: %d)\n", final_check, counter);
    printf("Test completed successfully.\n");
    
    return 0;
}
