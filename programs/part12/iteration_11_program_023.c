/* Test program for hardware loop analysis - partial basic block overlap scenarios */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to create side effects and prevent optimization */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* External function to prevent optimization */
extern void external_call(int x);

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 3;
}

/* Test case 1: Inner loop partially contained in outer loop */
void test_partial_overlap_1(int n) {
    for (int i = 0; i < n; ++i) {
        /* Code block A - always executed in outer loop */
        results[counter++] = i * 2;
        
        if (get_condition(i, 0) > 0) {
            /* Inner loop - only executed conditionally */
            for (int j = 0; j < i + 5; ++j) {
                /* Code block B - only in inner loop */
                results[counter++] = i * 100 + j;
                external_call(j);
            }
            /* Code block C - after inner loop, but only in this branch */
            checksum += i * 3;
        } else {
            /* Code block D - alternative branch without inner loop */
            results[counter++] = i * 7;
            external_call(i);
        }
        
        /* Code block E - always executed after the conditional */
        checksum += i;
    }
}

/* Test case 2: Two sibling inner loops with different conditions */
void test_sibling_loops(int n) {
    for (int i = 0; i < n; ++i) {
        /* Common code in outer loop */
        results[counter++] = i * 11;
        
        if (i % 2 == 0) {
            /* First inner loop */
            for (int j = 0; j < 10; ++j) {
                results[counter++] = i * 1000 + j * 100;
                external_call(j + 1);
            }
            /* Code only in this branch */
            checksum += i * 5;
        }
        
        if (i % 3 == 0) {
            /* Second inner loop (sibling to first) */
            for (int k = 0; k < 8; ++k) {
                results[counter++] = i * 2000 + k * 50;
                external_call(k + 100);
            }
            /* Different code in this branch */
            checksum += i * 7;
        }
        
        /* More outer loop code */
        external_call(i);
    }
}

/* Test case 3: Three-level nesting with varying overlap */
void test_three_level_nesting(int n) {
    for (int i = 0; i < n; ++i) {
        /* Level 1 code */
        results[counter++] = i * 3;
        
        for (int j = 0; j < i + 3; ++j) {
            /* Level 2 code - always in middle loop */
            checksum += j;
            
            if (get_condition(i, j) == 1) {
                /* Innermost loop - conditionally executed */
                for (int k = 0; k < j + 2; ++k) {
                    /* Level 3 code */
                    results[counter++] = i * 10000 + j * 100 + k;
                    external_call(k * 2);
                }
                /* Code after innermost loop in this branch */
                checksum += j * 2;
            } else {
                /* Alternative path without innermost loop */
                results[counter++] = i * 5000 + j * 50;
            }
        }
        
        /* More outer loop code */
        external_call(i * 10);
    }
}

/* Test case 4: Complex diamond-shaped control flow */
void test_diamond_nesting(int n) {
    for (int i = 0; i < n; ++i) {
        /* Pre-condition code */
        int cond1 = get_condition(i, 1);
        int cond2 = get_condition(i, 2);
        
        if (cond1 > 0) {
            /* Branch A */
            results[counter++] = i * 20;
            
            if (cond2 > 0) {
                /* Inner loop in nested condition */
                for (int j = 0; j < 7; ++j) {
                    results[counter++] = i * 300 + j * 25;
                    external_call(j + 50);
                }
            } else {
                /* Different code path */
                checksum += i * 11;
            }
        } else {
            /* Branch B */
            for (int k = 0; k < 5; ++k) {
                /* Different inner loop in else branch */
                results[counter++] = i * 400 + k * 30;
                external_call(k + 75);
            }
            
            if (cond2 == 0) {
                /* Yet another potential inner loop */
                for (int m = 0; m < 3; ++m) {
                    results[counter++] = i * 500 + m * 40;
                }
            }
        }
        
        /* Post-conditional code */
        checksum += i * 2;
    }
}

/* Test case 5: Loop with early exit affecting bitmap */
void test_early_exit(int n) {
    for (int i = 0; i < n; ++i) {
        results[counter++] = i * 6;
        
        if (i > n/2) {
            /* Early continue */
            checksum += i;
            continue;
        }
        
        for (int j = 0; j < i + 4; ++j) {
            /* Inner loop only for first half */
            results[counter++] = i * 200 + j;
            if (j > i) {
                /* Early break in inner loop */
                external_call(j);
                break;
            }
            checksum += j;
        }
        
        /* Code that might not execute if continue was taken */
        external_call(i);
    }
}

/* Dummy external function to prevent optimization */
void external_call(int x) {
    /* Volatile to prevent optimization */
    volatile static int dummy = 0;
    dummy += x;
}

int main() {
    /* Initialize random seed for condition functions */
    srand(time(NULL));
    
    printf("Starting hardware loop analysis test...\n");
    
    /* Execute each test case with different bounds */
    test_partial_overlap_1(20);
    printf("Test 1 complete, counter = %d, checksum = %d\n", counter, checksum);
    
    test_sibling_loops(15);
    printf("Test 2 complete, counter = %d, checksum = %d\n", counter, checksum);
    
    test_three_level_nesting(12);
    printf("Test 3 complete, counter = %d, checksum = %d\n", counter, checksum);
    
    test_diamond_nesting(10);
    printf("Test 4 complete, counter = %d, checksum = %d\n", counter, checksum);
    
    test_early_exit(25);
    printf("Test 5 complete, counter = %d, checksum = %d\n", counter, checksum);
    
    /* Final validation */
    int final_sum = 0;
    for (int i = 0; i < counter && i < 1000; ++i) {
        final_sum += results[i];
    }
    
    printf("Final validation: counter = %d, final_sum = %d\n", counter, final_sum);
    printf("Test completed successfully.\n");
    
    return 0;
}
