/* Test program to trigger hardware loop analysis for nested loops with partial basic block overlap */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define OUTER_ITER 50
#define MID_ITER 30
#define INNER_ITER 20

/* Global arrays to prevent optimization and create side effects */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* External function to prevent optimization */
extern int dummy_external(int x);

/* Function to create complex control flow */
int conditional_branch(int x, int y) {
    return (x * y) % 7;
}

/* Test case 1: Inner loop fully contained in outer loop (should trigger bitmap_intersect_p) */
void test_fully_contained(void) {
    volatile int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_ITER; i++) {
        /* Code before inner loop */
        sum += i * 2;
        
        /* Inner loop fully contained */
        for (int j = 0; j < MID_ITER; j++) {
            sum += j * 3;
            results[(i * j) % SIZE] = sum;
        }
        
        /* Code after inner loop */
        sum += i * 5;
    }
    
    checksum += sum;
    counter++;
}

/* Test case 2: Inner loop partially overlapping with outer loop 
   (should trigger bitmap_intersect_compl_p checks) */
void test_partial_overlap(void) {
    volatile int sum = 0;
    volatile int rnd;
    
    srand(time(NULL));
    
    /* Outer loop with conditional that creates partial overlap */
    for (int i = 0; i < OUTER_ITER; i++) {
        rnd = rand() % 10;
        
        /* Conditional creates different basic blocks */
        if (rnd < 7) {
            /* This branch contains the inner loop */
            sum += i * 11;
            
            /* Inner loop */
            for (int j = 0; j < MID_ITER; j++) {
                sum += j * 13;
                results[(i + j) % SIZE] = sum;
                
                /* Additional conditional inside inner loop */
                if (j % 3 == 0) {
                    sum += dummy_external(j);
                }
            }
            
            sum += i * 17;
        } else {
            /* This branch does NOT contain the inner loop
               Creates basic blocks in outer loop not in inner loop */
            sum += i * 19;
            results[i % SIZE] = sum * 2;
            
            /* Different inner structure - sibling loop */
            for (int k = 0; k < INNER_ITER / 2; k++) {
                sum -= k * 7;
            }
        }
        
        /* Common code after conditional */
        sum += i * 23;
    }
    
    checksum += sum;
    counter++;
}

/* Test case 3: Three-level nesting with varying overlap patterns */
void test_three_level_nesting(void) {
    volatile int sum = 0;
    volatile int toggle = 0;
    
    /* Level 1: Outermost loop */
    for (int a = 0; a < OUTER_ITER / 2; a++) {
        toggle = 1 - toggle;  /* Flip between 0 and 1 */
        
        /* Level 2: Middle loop - partially overlaps with outer */
        for (int b = 0; b < MID_ITER; b++) {
            sum += a * b;
            
            /* Conditional that sometimes includes innermost loop */
            if ((a + b) % 3 != 0) {
                /* Level 3: Innermost loop */
                for (int c = 0; c < INNER_ITER; c++) {
                    sum += c * 7;
                    results[(a + b + c) % SIZE] = sum;
                    
                    /* Additional complexity */
                    if (c % 4 == 0) {
                        sum += dummy_external(c);
                    }
                }
            } else {
                /* Alternative path without innermost loop */
                sum += b * 11;
                results[(a * b) % SIZE] = sum * 3;
            }
            
            /* Code after conditional, inside middle loop */
            sum += b * 13;
        }
        
        /* Code in outer loop not in middle loop when toggle is 1 */
        if (toggle) {
            sum += a * 100;
            for (int d = 0; d < 5; d++) {
                sum -= d * 3;
            }
        }
    }
    
    checksum += sum;
    counter++;
}

/* Test case 4: Sibling loops inside outer loop (cousin relationship) */
void test_sibling_loops(void) {
    volatile int sum = 0;
    
    /* Outer loop containing two sibling inner loops */
    for (int i = 0; i < OUTER_ITER; i++) {
        /* First sibling loop - executes conditionally */
        if (i % 3 == 0) {
            for (int j = 0; j < MID_ITER; j++) {
                sum += i * j * 2;
                results[(i * 2 + j) % SIZE] = sum;
            }
        }
        
        /* Code between sibling loops */
        sum += i * 7;
        
        /* Second sibling loop - executes under different condition */
        if (i % 4 == 0) {
            for (int k = 0; k < INNER_ITER; k++) {
                sum += i * k * 3;
                results[(i * 3 + k) % SIZE] = sum + 1;
                
                /* Nested inside second sibling */
                if (k % 2 == 0) {
                    for (int m = 0; m < 5; m++) {
                        sum += m * 5;
                    }
                }
            }
        }
        
        /* Final code in outer loop */
        sum += dummy_external(i);
    }
    
    checksum += sum;
    counter++;
}

/* Test case 5: Complex diamond-shaped control flow with loops */
void test_diamond_pattern(void) {
    volatile int sum = 0;
    
    for (int x = 0; x < OUTER_ITER; x++) {
        /* First decision point */
        if (x % 2 == 0) {
            /* Path A */
            sum += x * 2;
            
            /* Loop in path A */
            for (int y = 0; y < MID_ITER; y++) {
                sum += y * 3;
                if (y % 3 == 0) {
                    results[(x + y * 2) % SIZE] = sum;
                }
            }
        } else {
            /* Path B */
            sum += x * 5;
            
            /* Different loop in path B */
            for (int z = 0; z < INNER_ITER; z++) {
                sum += z * 7;
                results[(x + z * 3) % SIZE] = sum * 2;
            }
        }
        
        /* Reconvergence point with another conditional */
        if (x % 3 == 0) {
            /* Shared loop after reconvergence */
            for (int w = 0; w < 10; w++) {
                sum += w * 11;
            }
        }
        
        /* Final code in outer loop */
        sum += x * 13;
    }
    
    checksum += sum;
    counter++;
}

/* Dummy external function to prevent optimization */
int dummy_external(int x) {
    static int state = 0;
    state = (state * 1103515245 + 12345) & 0x7fffffff;
    return (state >> 16) % 100 + x;
}

int main(void) {
    /* Initialize results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = i;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_fully_contained();
    printf("Test 1 completed. Checksum: %d\n", checksum);
    
    test_partial_overlap();
    printf("Test 2 completed. Checksum: %d\n", checksum);
    
    test_three_level_nesting();
    printf("Test 3 completed. Checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 4 completed. Checksum: %d\n", checksum);
    
    test_diamond_pattern();
    printf("Test 5 completed. Checksum: %d\n", checksum);
    
    /* Final verification */
    volatile int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += results[i];
    }
    
    printf("Final results checksum: %d\n", final_sum);
    printf("Total test cases executed: %d\n", counter);
    
    return (final_sum > 0 && counter == 5) ? 0 : 1;
}
