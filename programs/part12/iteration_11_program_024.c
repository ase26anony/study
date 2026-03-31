/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define ITERATIONS 10

/* Global arrays to prevent optimization and create side effects */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* External function to prevent optimization */
extern void external_call(int);

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 3 + j * 7) % 5;
}

/* Test case 1: Outer loop with inner loop in one branch only */
void test_partial_overlap_1(void) {
    int i, j, k;
    
    for (i = 0; i < ITERATIONS; i++) {  /* Outer loop L1 */
        /* Code block A: executed in every iteration of outer loop */
        results[i] = i * 2;
        external_call(i);
        
        /* Conditional that creates partial overlap */
        if (get_condition(i, 0) > 2) {  /* Branch B1 */
            /* Inner loop L2 - fully contained in outer loop's block bitmap */
            for (j = 0; j < ITERATIONS / 2; j++) {  /* Inner loop L2 */
                /* Code block B: only in inner loop */
                results[j] += i * j;
                counter++;
                
                /* Another level of nesting */
                if (j % 3 == 0) {  /* Branch B2 */
                    for (k = 0; k < 3; k++) {  /* Inner-inner loop L3 */
                        results[k] ^= (i + j + k);
                        external_call(k);
                    }
                } else {
                    /* Alternative path in L2 but not in L3 */
                    results[j] -= 1;
                }
            }
        } else {  /* Branch B3: executed in outer loop but NOT in inner loop */
            /* This creates blocks in outer loop that are NOT in inner loop */
            results[i] = -results[i];
            external_call(-i);
        }
        
        /* More code in outer loop after the conditional */
        checksum += results[i % SIZE];
    }
}

/* Test case 2: Two sibling inner loops with partial overlap */
void test_sibling_loops(void) {
    int i, j, k;
    
    for (i = 0; i < ITERATIONS; i++) {  /* Outer loop L4 */
        volatile int temp = i;
        
        /* First inner loop L5 - executes conditionally */
        if (temp % 3 == 0) {
            for (j = 0; j < ITERATIONS / 3; j++) {  /* Inner loop L5 */
                results[j] += temp * j;
                counter++;
                
                /* Small nested loop inside L5 */
                for (k = 0; k < 2; k++) {  /* Loop L6 */
                    results[k] ^= j;
                }
            }
        }
        
        /* Code between the two inner loops - in outer but not in L5 */
        external_call(temp);
        
        /* Second inner loop L7 - also executes conditionally */
        if (temp % 4 == 0) {
            for (j = ITERATIONS / 3; j < ITERATIONS / 2; j++) {  /* Inner loop L7 */
                results[j] -= temp;
                counter--;
                
                /* Different control flow inside L7 */
                if (j % 2 == 0) {
                    results[j] *= 2;
                }
            }
        }
        
        /* Final code in outer loop */
        checksum += temp;
    }
}

/* Test case 3: Complex nested structure with multiple exits */
void test_complex_nesting(void) {
    int i, j, k;
    int limit = ITERATIONS;
    
    for (i = 0; i < limit; i++) {  /* Outer loop L8 */
        /* Early continue creates additional basic blocks */
        if (i % 7 == 0) {
            continue;
        }
        
        /* Multiple levels of conditionals with loops */
        for (j = 0; j < (i % 5) + 2; j++) {  /* Inner loop L9 */
            /* Loop with variable bound */
            for (k = 0; k < (j % 3) + 1; k++) {  /* Inner-inner loop L10 */
                results[(i + j + k) % SIZE] += 1;
                external_call(k);
            }
            
            /* Conditional break in middle of loop */
            if (results[j % SIZE] > 1000) {
                break;
            }
        }
        
        /* Another loop at same nesting level as L9 but disjoint */
        if (i % 3 == 1) {
            for (j = 10; j < 15; j++) {  /* Loop L11 - sibling to L9 */
                results[j] = i * j;
            }
        }
    }
}

/* Test case 4: Loop with switch statement creating multiple paths */
void test_switch_in_loop(void) {
    int i, j;
    
    for (i = 0; i < ITERATIONS * 2; i++) {  /* Outer loop L12 */
        switch (i % 4) {
            case 0:
                /* Path with inner loop */
                for (j = 0; j < 5; j++) {  /* Loop L13 */
                    results[j] += i;
                }
                break;
            case 1:
                /* Different path with different loop */
                for (j = 5; j < 10; j++) {  /* Loop L14 */
                    results[j] -= i;
                }
                break;
            case 2:
                /* Path without any inner loop */
                results[i % SIZE] = 0;
                break;
            default:
                /* Path with yet another loop structure */
                for (j = 0; j < 3; j++) {  /* Loop L15 */
                    results[j] *= 2;
                    external_call(j);
                }
                break;
        }
        
        /* Common code after switch */
        checksum += i;
    }
}

/* Test case 5: Do-while loops mixed with for loops */
void test_mixed_loop_types(void) {
    int i = 0;
    int j;
    
    /* Do-while as outer loop */
    do {  /* Loop L16 */
        /* For loop inside */
        for (j = 0; j < (i % 3) + 2; j++) {  /* Loop L17 */
            results[(i + j) % SIZE] ^= 0x55;
            counter++;
        }
        
        /* Conditional with another loop */
        if (i % 2 == 0) {
            int k = 0;
            while (k < 4) {  /* Loop L18 */
                results[k] += i;
                k++;
                external_call(k);
            }
        }
        
        i++;
    } while (i < ITERATIONS);
}

int main(void) {
    /* Initialize random seed for unpredictable but reproducible conditions */
    srand(42);
    
    /* Initialize results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = i;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_partial_overlap_1();
    printf("Test 1 complete, checksum = %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 2 complete, checksum = %d\n", checksum);
    
    test_complex_nesting();
    printf("Test 3 complete, checksum = %d\n", checksum);
    
    test_switch_in_loop();
    printf("Test 4 complete, checksum = %d\n", checksum);
    
    test_mixed_loop_types();
    printf("Test 5 complete, checksum = %d\n", checksum);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += results[i];
    }
    
    printf("Final results checksum = %d\n", final_sum);
    printf("Total operations: %d\n", counter);
    
    return 0;
}

/* Dummy external function definition */
void external_call(int x) {
    /* Volatile to prevent optimization */
    volatile static int dummy = 0;
    dummy += x;
}
