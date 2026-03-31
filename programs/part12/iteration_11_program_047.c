/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define OUTER_ITER 50
#define MID_ITER 30
#define INNER_ITER 20

/* Global arrays to prevent optimization */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create side effects */
int __attribute__((noinline)) do_work(int x, int y) {
    return (x * 7919 + y * 65537) & 0xFF;
}

/* Test case 1: Inner loop fully contained in outer loop's conditional branch */
void test_fully_contained_nesting(void) {
    int i, j, k;
    int temp = 0;
    
    /* Outer loop */
    for (i = 0; i < OUTER_ITER; i++) {
        /* Conditional that creates separate basic blocks */
        if (i % 3 == 0) {
            /* Middle loop - fully contained in this branch */
            for (j = 0; j < MID_ITER; j++) {
                /* Inner loop - fully contained */
                for (k = 0; k < INNER_ITER; k++) {
                    temp += do_work(i, j) * k;
                }
                /* Additional code in middle loop but outside inner loop */
                temp += do_work(j, i);
            }
        } else {
            /* Alternative branch - no inner loops here */
            temp += do_work(i, 999);
        }
        
        /* Code in outer loop but outside middle loop */
        results[i % SIZE] = temp & 0xFF;
    }
    
    checksum += temp;
    counter++;
}

/* Test case 2: Partially overlapping loops (cousin loops) */
void test_partial_overlap_nesting(void) {
    int a, b, c;
    int temp = 0;
    volatile int r = rand() % 10;
    
    /* Outer loop */
    for (a = 0; a < OUTER_ITER; a++) {
        /* First conditional branch with inner loop */
        if (a % 4 == 0) {
            /* First inner loop */
            for (b = 0; b < MID_ITER; b++) {
                temp += do_work(a, b) * r;
                /* Conditional inside this inner loop */
                if (b % 5 == 0) {
                    temp += do_work(b, a);
                }
            }
        }
        /* Code between the two inner loops in outer loop */
        temp += a * 17;
        
        /* Second conditional branch with different inner loop */
        if (a % 3 == 1) {
            /* Second inner loop - shares some blocks with first via outer loop */
            for (c = 0; c < INNER_ITER; c++) {
                temp += do_work(a, c) * (r + 1);
                /* Different structure than first inner loop */
                if (c % 7 == 0) {
                    temp -= do_work(c, a);
                } else {
                    temp += do_work(c, 255);
                }
            }
        }
        
        results[(a + 10) % SIZE] = temp & 0xFF;
    }
    
    checksum += temp;
    counter++;
}

/* Test case 3: Complex nesting with multiple exit points */
void test_complex_nesting(void) {
    int x, y, z;
    int temp = 0;
    volatile int flag = rand() % 2;
    
    /* Level 1 loop */
    for (x = 0; x < OUTER_ITER; x++) {
        /* Level 2 loop - sometimes executed */
        if (x % 5 != 0 || flag) {
            for (y = x; y < MID_ITER && y < OUTER_ITER; y++) {
                /* Level 3 loop - conditionally executed */
                if (y % 3 == 0) {
                    for (z = 0; z < INNER_ITER; z++) {
                        temp += do_work(x, y) * z;
                        /* Early exit from innermost loop */
                        if (temp > 10000) {
                            temp = temp % 1000;
                        }
                    }
                } else {
                    /* Alternative path in level 2 without level 3 */
                    temp += do_work(y, x) * 11;
                }
                
                /* Additional code in level 2 outside level 3 */
                if (y % 2 == 0) {
                    temp += do_work(x, 777);
                }
            }
        } else {
            /* Alternative path in level 1 without level 2 */
            temp += do_work(x, 888);
        }
        
        /* Code in level 1 outside level 2 */
        results[(x + 20) % SIZE] = temp & 0xFF;
        
        /* Conditional continue */
        if (temp % 7 == 0) {
            continue;
        }
        
        temp += x;
    }
    
    checksum += temp;
    counter++;
}

/* Test case 4: Sibling loops with shared outer loop */
void test_sibling_loops(void) {
    int p, q, r;
    int temp = 0;
    
    /* Outer loop containing two independent inner loops */
    for (p = 0; p < OUTER_ITER; p++) {
        /* First sibling loop */
        if (p % 2 == 0) {
            for (q = 0; q < MID_ITER; q++) {
                temp += do_work(p, q) * 3;
                /* Nested inside first sibling */
                if (q % 4 == 0) {
                    for (r = 0; r < 5; r++) {  /* Very small inner */
                        temp += do_work(q, r);
                    }
                }
            }
        }
        
        /* Code between siblings */
        temp += p * 19;
        
        /* Second sibling loop - different structure */
        if (p % 3 == 0) {
            for (q = MID_ITER - 1; q >= 0; q--) {
                temp += do_work(p, q) * 7;
                /* Different nesting pattern */
                if (q % 6 == 0) {
                    temp += do_work(q, p) * 2;
                } else if (q % 6 == 1) {
                    temp += do_work(q, p + 1);
                }
            }
        }
        
        results[(p + 30) % SIZE] = temp & 0xFF;
    }
    
    checksum += temp;
    counter++;
}

int main(void) {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = 0;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute test cases with different nesting patterns */
    test_fully_contained_nesting();
    printf("Test 1 completed. Checksum: %d\n", checksum);
    
    test_partial_overlap_nesting();
    printf("Test 2 completed. Checksum: %d\n", checksum);
    
    test_complex_nesting();
    printf("Test 3 completed. Checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 4 completed. Checksum: %d\n", checksum);
    
    /* Final verification */
    int final_check = 0;
    for (int i = 0; i < SIZE; i++) {
        final_check += results[i];
    }
    
    printf("Final results checksum: %d\n", final_check);
    printf("Total loops executed: %d\n", counter);
    
    return 0;
}
