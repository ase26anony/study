/* Test program for hardware loop nesting analysis in GCC */
/* Designed to trigger bitmap_intersect_compl_p checks in hw-doloop.cc */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to prevent optimization and create side effects */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* External function to prevent hoisting */
extern int get_random_limit(void);

/* Function to create complex control flow */
static int conditional_inner(int x, int y) {
    return (x * y) & 0xF;
}

/* Test case 1: Outer loop with inner loop in one branch only */
void test_partial_overlap_1(int n) {
    int i, j;
    volatile int temp = 0;
    
    /* Outer loop - will contain blocks not in inner loop */
    for (i = 0; i < n; ++i) {
        /* This block is in outer loop but NOT in inner loop */
        temp += i * 2;
        results[counter++ % 1000] = temp;
        
        /* Conditional: inner loop only executes sometimes */
        if (i % 3 == 0) {
            /* Inner loop - fully contained in this branch */
            for (j = 0; j < (i % 5) + 2; ++j) {
                /* Blocks only in inner loop */
                temp -= j;
                results[counter++ % 1000] = temp * j;
            }
        } else {
            /* Alternative path in outer loop - NOT in inner loop */
            temp += 100;
            results[counter++ % 1000] = temp ^ i;
        }
        
        /* More outer loop blocks after the conditional */
        temp = (temp * 17) & 0xFF;
    }
    
    checksum += temp;
}

/* Test case 2: Two inner loops with different conditions */
void test_partial_overlap_2(int n) {
    int i, j, k;
    volatile int acc = 0;
    
    for (i = 0; i < n; ++i) {
        /* First conditional branch with inner loop */
        if (i & 1) {
            for (j = 0; j < (i % 3) + 1; ++j) {
                acc += i * j;
                results[counter++ % 1000] = acc;
            }
        } 
        /* Second conditional branch with different inner loop */
        else if (i & 2) {
            for (k = 0; k < (i % 4) + 1; ++k) {
                acc -= i + k;
                results[counter++ % 1000] = acc | k;
            }
        }
        /* Third branch without any inner loop */
        else {
            acc = (acc << 1) ^ i;
            results[counter++ % 1000] = acc;
        }
        
        /* Common outer loop code */
        acc = (acc + 7) % 256;
    }
    
    checksum ^= acc;
}

/* Test case 3: Three-level nesting with varying overlap */
void test_three_level_nesting(int n) {
    int i, j, k;
    volatile int val = 0;
    
    /* Level 1: Outer loop */
    for (i = 0; i < n; ++i) {
        /* Some outer-only code */
        val += rand() % 10;
        
        /* Level 2: Middle loop - sometimes executes */
        if (i % 2 == 0) {
            for (j = 0; j < (i % 4) + 2; ++j) {
                /* Middle loop code */
                val ^= j * 3;
                
                /* Level 3: Innermost loop - conditional */
                if (j % 2 == 0) {
                    for (k = 0; k < (j % 3) + 1; ++k) {
                        val += i + j + k;
                        results[counter++ % 1000] = val;
                    }
                } else {
                    /* Alternative path in middle loop */
                    val -= 5;
                }
            }
        } else {
            /* Alternative outer path with different inner structure */
            for (j = 0; j < 2; ++j) {
                val = (val * 3) % 100;
                results[counter++ % 1000] = val + j;
            }
        }
        
        /* More outer loop code */
        val = (val + i) & 0xFF;
    }
    
    checksum += val;
}

/* Test case 4: Sibling loops inside outer loop */
void test_sibling_loops(int n) {
    int i, j, k;
    volatile int sum = 0;
    
    for (i = 0; i < n; ++i) {
        /* First sibling loop */
        if (i % 3 == 0) {
            for (j = 0; j < 3; ++j) {
                sum += i * j;
                results[counter++ % 1000] = sum;
            }
        }
        
        /* Code between sibling loops */
        sum = (sum + 1) % 100;
        
        /* Second sibling loop (different condition) */
        if (i % 4 == 0) {
            for (k = 0; k < 2; ++k) {
                sum -= i + k;
                results[counter++ % 1000] = sum ^ k;
            }
        }
        
        /* More outer loop code */
        sum = sum * 2;
    }
    
    checksum ^= sum;
}

/* Test case 5: Complex overlapping with function calls */
void test_complex_overlap(int n) {
    int i, j;
    volatile int x = 0;
    
    for (i = 0; i < n; ++i) {
        /* Outer loop pre-inner code */
        x += conditional_inner(i, 2);
        
        /* Nested loop with early exit */
        for (j = 0; j < 5; ++j) {
            if (x > 100) {
                /* Early exit creates additional blocks */
                x = x / 2;
                break;
            }
            x += i * j;
            
            /* Conditional inside inner loop */
            if (j % 2 == 0) {
                x ^= 0xAA;
            } else {
                x |= 0x55;
            }
            
            results[counter++ % 1000] = x;
        }
        
        /* Outer loop post-inner code */
        x = (x + 7) & 0xFF;
        
        /* Sometimes add another inner loop */
        if (i % 5 == 0) {
            for (j = 0; j < 2; ++j) {
                x -= j * 10;
                results[counter++ % 1000] = x;
            }
        }
    }
    
    checksum += x;
}

int main(void) {
    int i;
    
    /* Seed random for reproducible but non-constant behavior */
    srand(time(NULL));
    
    /* Initialize results array */
    for (i = 0; i < 1000; ++i) {
        results[i] = i;
    }
    
    printf("Starting hardware loop nesting tests...\n");
    
    /* Execute each test case with different bounds */
    test_partial_overlap_1(50);        /* Triggers partial overlap */
    test_partial_overlap_2(40);        /* Two inner loops with conditions */
    test_three_level_nesting(30);      /* Three-level nesting */
    test_sibling_loops(35);            /* Sibling loops in outer loop */
    test_complex_overlap(25);          /* Complex overlap with breaks */
    
    /* Final computation to ensure all loops executed */
    volatile int final = 0;
    for (i = 0; i < 100; ++i) {
        final += results[i % 1000];
    }
    
    checksum += final;
    
    printf("Tests completed. Checksum: %d\n", checksum);
    printf("Counter value: %d\n", counter);
    
    /* Simple validation */
    if (checksum != 0 && counter > 0) {
        printf("SUCCESS: Loops executed with observable side effects\n");
    } else {
        printf("WARNING: Possible optimization issues\n");
    }
    
    return 0;
}
