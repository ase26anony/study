/* Test program for hardware loop nested block bitmap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define OUTER_ITER 10
#define MID_ITER 20
#define INNER_ITER 30

/* Global arrays to prevent optimization and create side effects */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create side effects */
void record_value(int idx, int val) {
    results[idx % SIZE] = val;
    checksum ^= val;
}

/* Function with unpredictable control flow */
int maybe_skip(int x) {
    return (x & 3) != 0;  /* Returns true 75% of time */
}

int main(void) {
    int i, j, k;
    
    /* Initialize random seed for unpredictable branches */
    srand(time(NULL));
    
    /* Test Case 1: Simple nested loops with full containment */
    printf("Test 1: Fully contained nested loops\n");
    for (i = 0; i < OUTER_ITER; i++) {
        /* Outer loop code before inner loop */
        record_value(counter++, i * 10);
        
        /* Inner loop fully contained within outer loop */
        for (j = 0; j < MID_ITER; j++) {
            record_value(counter++, i * 100 + j);
            
            /* Innermost loop fully contained */
            for (k = 0; k < INNER_ITER; k++) {
                record_value(counter++, i * 1000 + j * 100 + k);
            }
        }
        
        /* Outer loop code after inner loop */
        record_value(counter++, i * 20 + 5);
    }
    
    /* Test Case 2: Partial overlap - inner loop in conditional branch */
    printf("Test 2: Partial overlap with conditional inner loop\n");
    for (i = 0; i < OUTER_ITER * 2; i++) {
        /* Always executed in outer loop */
        int r = rand() % 100;
        record_value(counter++, r);
        
        /* Conditional execution creates partial overlap */
        if (maybe_skip(i)) {
            /* This inner loop shares blocks with outer loop */
            for (j = 0; j < MID_ITER; j++) {
                record_value(counter++, i * 50 + j * 2);
                
                /* Another conditional inside inner loop */
                if (j % 3 == 0) {
                    for (k = 0; k < INNER_ITER / 2; k++) {
                        record_value(counter++, i * 500 + j * 50 + k * 3);
                    }
                } else {
                    /* Alternative path without innermost loop */
                    record_value(counter++, i * 500 + j * 50 + 999);
                }
            }
        } else {
            /* Alternative path without the middle loop */
            record_value(counter++, i * 1000 + 777);
            
            /* But still has a different inner loop! */
            for (k = 0; k < INNER_ITER / 3; k++) {
                record_value(counter++, i * 2000 + k * 7);
            }
        }
        
        /* More outer loop code */
        record_value(counter++, i * 30 + r % 10);
    }
    
    /* Test Case 3: Sibling loops with partial overlap */
    printf("Test 3: Sibling loops with shared parent\n");
    for (i = 0; i < OUTER_ITER; i++) {
        /* Common preamble */
        int base = i * 100;
        record_value(counter++, base);
        
        /* First inner loop */
        if (i % 2 == 0) {
            for (j = 0; j < MID_ITER; j++) {
                record_value(counter++, base + j * 2);
            }
        } else {
            /* Alternative for first loop */
            record_value(counter++, base + 111);
        }
        
        /* Shared middle code */
        record_value(counter++, base + 222);
        
        /* Second inner loop (sibling of first) */
        if (i % 3 == 0) {
            for (j = MID_ITER / 2; j < MID_ITER; j++) {
                record_value(counter++, base + j * 3);
                
                /* Nested inside sibling */
                for (k = 0; k < 5; k++) {
                    record_value(counter++, base + j * 30 + k);
                }
            }
        }
        
        /* Common postamble */
        record_value(counter++, base + 333);
    }
    
    /* Test Case 4: Complex nesting with multiple exits */
    printf("Test Case 4: Complex nesting with early exits\n");
    for (i = 0; i < OUTER_ITER; i++) {
        record_value(counter++, i * 111);
        
        for (j = 0; j < MID_ITER; j++) {
            /* Early exit from middle loop */
            if (j == MID_ITER / 2 && i > OUTER_ITER / 2) {
                record_value(counter++, 8888);
                break;
            }
            
            record_value(counter++, i * 222 + j);
            
            /* Innermost with conditional continue */
            for (k = 0; k < INNER_ITER; k++) {
                if (k % 4 == 0) {
                    record_value(counter++, i * 333 + j * 22 + k);
                    continue;
                }
                record_value(counter++, i * 444 + j * 33 + k * 2);
            }
            
            /* Code after innermost but still in middle */
            if (j % 5 == 0) {
                record_value(counter++, 9999);
            }
        }
        
        /* Outer loop tail with another small loop */
        for (k = 0; k < 3; k++) {
            record_value(counter++, i * 555 + k);
        }
    }
    
    /* Final validation */
    printf("Final checksum: %d\n", checksum);
    printf("Total records: %d\n", counter);
    
    /* Print first few results to ensure computation happened */
    printf("Sample results[0..4]: ");
    for (i = 0; i < 5 && i < SIZE; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return 0;
}
