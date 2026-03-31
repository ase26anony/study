/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 * lines 429-436
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
    return (i * 17 + j * 13) % 3;
}

/* Function with side effects */
void record_value(int idx, int val) {
    results[idx % 1000] = val;
    checksum ^= val;
    counter++;
}

int main(void) {
    int i, j, k;
    int outer_limit = 50;
    int middle_limit = 30;
    int inner_limit = 20;
    
    /* Seed random for unpredictable but bounded conditions */
    srand(time(NULL));
    
    printf("Starting hardware loop test patterns...\n");
    
    /* Pattern 1: Classic nested loops with full containment
     * Inner loop fully contained in outer loop
     * Should trigger bitmap_intersect_p but not bitmap_intersect_compl_p
     */
    for (i = 0; i < outer_limit; ++i) {
        /* Some code before inner loop */
        record_value(i, i * 2);
        
        /* Fully contained inner loop */
        for (j = 0; j < middle_limit; ++j) {
            record_value(i * 100 + j, i + j);
            
            /* Innermost fully contained loop */
            for (k = 0; k < inner_limit; ++k) {
                record_value(i * 10000 + j * 100 + k, i * j * k);
            }
        }
        
        /* Some code after inner loop */
        record_value(i + 1000, i * 3);
    }
    
    /* Pattern 2: Partial overlap with conditional inner loop execution
     * Inner loop executes only in some iterations of outer loop
     * Creates partial basic block overlap
     */
    for (i = 0; i < outer_limit; ++i) {
        int condition = get_condition(i, 0);
        
        /* Branch with inner loop */
        if (condition == 0) {
            record_value(i, i * 5);
            
            /* Inner loop that shares some blocks with outer */
            for (j = 0; j < middle_limit; ++j) {
                record_value(i * 200 + j, i - j);
                
                /* Conditional within inner loop creates more blocks */
                if (j % 2 == 0) {
                    for (k = 0; k < inner_limit; ++k) {
                        record_value(i * 20000 + j * 200 + k, i + j + k);
                    }
                } else {
                    record_value(i * 20000 + j, i * j);
                }
            }
        }
        /* Branch without inner loop */
        else if (condition == 1) {
            record_value(i, i * 7);
            /* No inner loop here - creates blocks in outer not in inner */
            for (int x = 0; x < 10; ++x) {
                record_value(i * 300 + x, i * x);
            }
        }
        /* Another branch with different inner loop structure */
        else {
            record_value(i, i * 11);
            
            /* Different inner loop with partial overlap */
            for (j = 5; j < middle_limit - 5; ++j) {
                record_value(i * 400 + j, i / (j + 1));
            }
        }
    }
    
    /* Pattern 3: Sibling loops inside outer loop
     * Two inner loops that don't overlap with each other
     * but both are contained in outer loop
     */
    for (i = 0; i < outer_limit / 2; ++i) {
        /* First inner loop */
        if (i % 3 == 0) {
            for (j = 0; j < middle_limit; j += 2) {
                record_value(i * 500 + j, i % (j + 1));
            }
        }
        
        /* Code between sibling loops */
        record_value(i, rand() % 100);
        
        /* Second inner loop (sibling of first) */
        if (i % 4 == 0) {
            for (j = 1; j < middle_limit; j += 2) {
                record_value(i * 600 + j, i * (j % 7));
                
                /* Nested within sibling */
                for (k = 0; k < inner_limit / 2; ++k) {
                    record_value(i * 60000 + j * 600 + k, k - i);
                }
            }
        }
    }
    
    /* Pattern 4: Complex nesting with multiple exit points
     * Creates more complex control flow graphs
     */
    for (i = 0; i < outer_limit; ++i) {
        /* Early continue creates additional blocks */
        if (i % 7 == 0) {
            record_value(i, -i);
            continue;
        }
        
        for (j = 0; j < middle_limit; ++j) {
            /* Break from inner loop creates separate exit block */
            if (j > middle_limit / 2 && i > outer_limit / 2) {
                record_value(i * 700 + j, 999);
                break;
            }
            
            /* Loop with early exit */
            for (k = 0; k < inner_limit; ++k) {
                if (k > inner_limit / 3) {
                    record_value(i * 70000 + j * 700 + k, 888);
                    goto inner_done;
                }
                record_value(i * 70000 + j * 700 + k, i + j + k);
            }
            inner_done:
            
            /* Code after inner loop but still in middle loop */
            if (j % 3 == 1) {
                record_value(i * 800 + j, 777);
            }
        }
        
        /* Label to continue outer loop */
        if (i % 5 == 0) {
            record_value(i + 2000, 666);
        }
    }
    
    /* Pattern 5: Do-while and while loops mixed with for loops
     * Different loop structures create varied basic block patterns
     */
    i = 0;
    while (i < outer_limit / 3) {
        record_value(i + 3000, i * i);
        
        j = 0;
        do {
            record_value(i * 900 + j, (i + j) % 19);
            
            /* For loop inside do-while */
            for (k = 0; k < inner_limit; ++k) {
                record_value(i * 90000 + j * 900 + k, k * 2);
            }
            
            j++;
        } while (j < middle_limit / 2);
        
        i++;
    }
    
    /* Final verification */
    printf("Test completed. Counter: %d, Checksum: %d\n", counter, checksum);
    printf("Sample values: %d, %d, %d\n", 
           results[0], results[100], results[500]);
    
    return 0;
}
