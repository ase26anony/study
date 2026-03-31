/* test-hwloops-nested.c
 * 
 * This test creates various nested loop patterns with partial basic block
 * overlap to trigger bitmap intersection checks in GCC's hardware loop
 * optimization pass (hw-doloop.cc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to prevent optimization and create side effects */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* External function to prevent optimization */
extern void external_call(int x);

/* Function to create unpredictable but bounded conditions */
static inline int get_condition(int i, int j) {
    return (i * 17 + j * 13) & 0x7;
}

/* Test case 1: Three-level nested loops with partial overlap */
void test_case_1(void) {
    const int N = 50;
    const int M = 30;
    const int K = 20;
    
    for (int i = 0; i < N; ++i) {
        /* Outer loop block A - always executed */
        results[counter++] = i;
        
        /* Conditional that creates partial overlap */
        if (get_condition(i, 0) > 2) {
            /* Inner loop j - partially contained in outer loop */
            for (int j = 0; j < M; ++j) {
                /* Code inside inner loop j */
                results[counter++] = i * 1000 + j;
                
                /* Another conditional inside j loop */
                if (get_condition(i, j) > 4) {
                    /* Innermost loop k - fully contained in j loop */
                    for (int k = 0; k < K; ++k) {
                        results[counter++] = i * 1000000 + j * 1000 + k;
                        checksum += (i ^ j ^ k);
                    }
                } else {
                    /* Alternative path in j loop (not in k loop) */
                    results[counter++] = i * 1000 + j + 99999;
                    checksum -= (i * j);
                }
            }
        } else {
            /* Alternative outer loop path (not in j loop) */
            results[counter++] = i + 777777;
            checksum += i * 3;
            
            /* Another inner loop that's a sibling of the j loop */
            for (int p = 0; p < 10; ++p) {
                results[counter++] = i * 100 + p + 888888;
                checksum += p;
            }
        }
        
        /* Outer loop block B - always executed (not in inner loops) */
        results[counter++] = i * 2;
    }
}

/* Test case 2: Complex partial overlap with multiple inner loops */
void test_case_2(void) {
    const int LIMIT = 40;
    
    for (int x = 0; x < LIMIT; ++x) {
        volatile int temp = x * 7;
        
        /* First conditional branch */
        if (x % 3 == 0) {
            /* Inner loop A */
            for (int a = 0; a < 15; ++a) {
                results[counter++] = x * 100 + a;
                checksum += temp + a;
                
                /* Conditional inside A that sometimes skips to outer */
                if (a % 5 == 2) {
                    /* Early continue to outer loop */
                    continue;
                }
                
                /* More code in loop A */
                results[counter++] = a * 11;
            }
        } 
        /* Second conditional branch (else if for partial overlap) */
        else if (x % 3 == 1) {
            /* Different inner loop B */
            for (int b = 0; b < 12; ++b) {
                results[counter++] = x * 200 + b;
                checksum -= temp - b;
                
                /* Nested loop inside B */
                for (int c = 0; c < 8; ++c) {
                    results[counter++] = b * 50 + c;
                    checksum ^= (x + b + c);
                }
            }
            
            /* Additional code after loop B but still in else-if */
            results[counter++] = x * 333;
        }
        else {
            /* No inner loops in this branch */
            results[counter++] = x * 444;
            checksum += x * x;
        }
        
        /* Common outer loop code */
        external_call(x);
    }
}

/* Test case 3: Switch statement creating multiple overlap patterns */
void test_case_3(void) {
    const int ROWS = 25;
    const int COLS = 18;
    
    for (int row = 0; row < ROWS; ++row) {
        switch (row % 4) {
            case 0:
                /* Fully nested loop */
                for (int col = 0; col < COLS; ++col) {
                    results[counter++] = row * COLS + col;
                    checksum += row * col;
                }
                break;
                
            case 1:
                /* Partially overlapping loops */
                for (int col = 0; col < COLS / 2; ++col) {
                    results[counter++] = row * 1000 + col;
                    
                    if (col % 2 == 0) {
                        /* Extra inner loop */
                        for (int k = 0; k < 5; ++k) {
                            results[counter++] = col * 10 + k;
                            checksum -= k;
                        }
                    }
                }
                /* Additional code not in inner loops */
                results[counter++] = row * 999;
                break;
                
            case 2:
                /* Two sibling inner loops */
                for (int a = 0; a < 8; ++a) {
                    results[counter++] = row * 8 + a;
                    checksum += a;
                }
                
                for (int b = 0; b < 6; ++b) {
                    results[counter++] = row * 6 + b + 1000;
                    checksum += b * 2;
                }
                break;
                
            default:
                /* No inner loop */
                results[counter++] = row * 1111;
                checksum ^= row;
                break;
        }
        
        /* Outer loop continuation */
        external_call(row % 256);
    }
}

/* Test case 4: Loop with early exit creating partial containment */
void test_case_4(void) {
    const int SIZE = 35;
    
    for (int i = 0; i < SIZE; ++i) {
        results[counter++] = i;
        
        /* Inner loop with possible early break */
        for (int j = 0; j < 20; ++j) {
            results[counter++] = i * 20 + j;
            
            if (j > i) {
                /* Break makes this not fully contained */
                checksum += j * 100;
                break;
            }
            
            /* Continue inner loop */
            checksum += i + j;
            
            /* Sometimes skip to outer */
            if ((i + j) % 7 == 0) {
                goto skip_rest_of_inner;
            }
        }
        skip_rest_of_inner:
        
        /* More outer loop code */
        results[counter++] = i * 7 + 12345;
    }
}

/* Dummy external function definition */
void external_call(int x) {
    /* Prevent inlining and create side effect */
    volatile static int secret = 0;
    secret ^= x;
    results[counter % 1000] ^= secret;
}

int main(void) {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < 1000; ++i) {
        results[i] = i;
    }
    
    counter = 0;
    checksum = 0;
    
    printf("Starting hardware loop nesting tests...\n");
    
    /* Execute all test cases to create various overlap patterns */
    test_case_1();
    printf("Test 1 complete: counter=%d, checksum=%d\n", counter, checksum);
    
    test_case_2();
    printf("Test 2 complete: counter=%d, checksum=%d\n", counter, checksum);
    
    test_case_3();
    printf("Test 3 complete: counter=%d, checksum=%d\n", counter, checksum);
    
    test_case_4();
    printf("Test 4 complete: counter=%d, checksum=%d\n", counter, checksum);
    
    /* Final computation to ensure all loops have effect */
    int final_sum = 0;
    for (int i = 0; i < counter && i < 1000; ++i) {
        final_sum += results[i];
    }
    
    printf("Final result: sum=%d, counter=%d, checksum=%d\n", 
           final_sum, counter, checksum);
    
    return 0;
}
