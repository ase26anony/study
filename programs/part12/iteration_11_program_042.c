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
    return (i * 17 + j * 13) % 7;
}

/* Function with side effects */
void record_value(int idx, int val) {
    results[idx % 1000] = val;
    checksum ^= val;
}

/* Test case 1: Three-level nested loops with partial overlap */
void test_case_1() {
    const int N = 50;
    const int M = 30;
    const int K = 20;
    
    for (int i = 0; i < N; ++i) {
        /* Outer loop has blocks not in inner loops */
        record_value(counter++, i);
        
        /* First conditional branch - contains inner loop */
        if (get_condition(i, 0) < 4) {
            /* Middle loop - partially overlaps with outer */
            for (int j = 0; j < M; ++j) {
                /* Middle loop has its own blocks */
                record_value(counter++, i * 100 + j);
                
                /* Second conditional inside middle loop */
                if (get_condition(i, j) < 3) {
                    /* Innermost loop - fully contained in middle loop */
                    for (int k = 0; k < K; ++k) {
                        record_value(counter++, i * 10000 + j * 100 + k);
                    }
                } else {
                    /* Alternative path in middle loop */
                    record_value(counter++, i * 1000 + j);
                }
            }
        } else {
            /* Alternative path in outer loop - creates partial overlap */
            for (int j = 0; j < M/2; ++j) {
                record_value(counter++, i * 200 + j);
            }
        }
        
        /* More code in outer loop after conditional */
        record_value(counter++, i * 300);
    }
}

/* Test case 2: Sibling loops inside outer loop */
void test_case_2() {
    const int N = 40;
    const int M = 25;
    
    for (int i = 0; i < N; ++i) {
        /* Outer loop preamble */
        record_value(counter++, i + 1000);
        
        /* First inner loop - executed conditionally */
        if (i % 3 == 0) {
            for (int j = 0; j < M; ++j) {
                record_value(counter++, i * 500 + j * 2);
                
                /* Extra conditional inside first inner loop */
                if (j % 4 == 0) {
                    record_value(counter++, j * 100);
                }
            }
        }
        
        /* Code between sibling loops */
        record_value(counter++, i * 700);
        
        /* Second inner loop - also conditional, creates partial overlap */
        if (i % 5 != 0) {
            for (int j = 0; j < M + 5; ++j) {
                record_value(counter++, i * 600 + j * 3);
            }
        }
        
        /* Outer loop postamble */
        record_value(counter++, i * 800);
    }
}

/* Test case 3: Complex nested structure with multiple exits */
void test_case_3() {
    const int N = 60;
    const int M = 35;
    const int L = 15;
    
    for (int i = 0; i < N; ++i) {
        record_value(counter++, i + 2000);
        
        /* Loop with early exit possibility */
        for (int j = 0; j < M; ++j) {
            record_value(counter++, i * 900 + j);
            
            /* Early exit creates additional basic blocks */
            if (get_condition(i, j) == 0) {
                record_value(counter++, -1);
                break;
            }
            
            /* Deeply nested conditional loop */
            if (j % 6 == 0) {
                for (int k = 0; k < L; ++k) {
                    record_value(counter++, i * 1000 + j * 100 + k);
                    
                    /* Another conditional inside innermost */
                    if (k % 3 == 0) {
                        record_value(counter++, k * 50);
                    }
                }
            } else {
                /* Alternative path at middle level */
                record_value(counter++, i * 1100 + j);
            }
        }
        
        /* Additional outer loop code */
        if (i % 10 == 0) {
            record_value(counter++, i * 1200);
        }
    }
}

/* Test case 4: Loop with switch statement inside */
void test_case_4() {
    const int N = 45;
    const int M = 20;
    
    for (int i = 0; i < N; ++i) {
        record_value(counter++, i + 3000);
        
        /* Switch creates multiple basic blocks */
        switch (i % 4) {
            case 0:
                for (int j = 0; j < M; ++j) {
                    record_value(counter++, i * 1300 + j);
                }
                break;
            case 1:
                /* Different loop structure */
                for (int j = M-1; j >= 0; --j) {
                    record_value(counter++, i * 1400 + j);
                }
                break;
            case 2:
                /* No inner loop here */
                record_value(counter++, i * 1500);
                break;
            case 3:
                /* Nested loops with different bounds */
                for (int j = 0; j < M/2; ++j) {
                    for (int k = 0; k < 5; ++k) {
                        record_value(counter++, i * 1600 + j * 10 + k);
                    }
                }
                break;
        }
        
        /* Common outer loop code */
        record_value(counter++, i * 1700);
    }
}

int main() {
    /* Initialize random seed for get_condition function */
    srand(time(NULL));
    
    printf("Starting hardware loop analysis test...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_case_1();
    printf("Test case 1 completed. Checksum: %d\n", checksum);
    
    test_case_2();
    printf("Test case 2 completed. Checksum: %d\n", checksum);
    
    test_case_3();
    printf("Test case 3 completed. Checksum: %d\n", checksum);
    
    test_case_4();
    printf("Test case 4 completed. Checksum: %d\n", checksum);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < 1000; ++i) {
        final_sum += results[i];
    }
    
    printf("Final array sum: %d\n", final_sum);
    printf("Total operations recorded: %d\n", counter);
    
    return 0;
}
