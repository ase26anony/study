/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p checks in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define ITERS 50

/* Global arrays to prevent optimization and create side effects */
volatile int results[SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* External function to prevent optimization */
extern void external_call(int x);

/* Function to create complex control flow */
int conditional_inner(int x, int y) {
    return (x * y) % 7;
}

/* Test case 1: Three-level nested loops with partial overlap */
void test_partial_overlap_nested(void) {
    volatile int arr1[SIZE];
    volatile int arr2[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = SIZE - i;
    }
    
    /* Outer loop - will contain partial blocks of inner loops */
    for (int i = 0; i < ITERS; i++) {
        /* Conditional execution creates separate basic blocks */
        if (i % 3 == 0) {
            /* First inner loop - fully contained in this branch */
            for (int j = 0; j < i + 5; j++) {
                /* Complex body to prevent optimization */
                arr1[j] = arr1[j] + arr2[i] * j;
                results[counter++ % SIZE] = arr1[j];
                
                /* Innermost loop - partially overlapping with middle loop */
                if (j % 2 == 0) {
                    for (int k = 0; k < 10; k++) {
                        arr2[k] = arr1[j] + k * i;
                        checksum += arr2[k];
                    }
                } else {
                    /* Alternative path - creates blocks not in innermost loop */
                    arr2[j] = arr1[j] * 2;
                    external_call(arr2[j]);
                }
            }
        } else if (i % 3 == 1) {
            /* Second inner loop - sibling to first, sharing some outer blocks */
            for (int j = 5; j < ITERS/2; j++) {
                arr1[j] = arr1[j] - arr2[i];
                results[counter++ % SIZE] = arr1[j];
                
                /* Different control flow pattern */
                if (j % 3 == 0) {
                    for (int k = j; k < j + 3; k++) {
                        arr2[k % SIZE] = arr1[j] * k;
                        checksum -= arr2[k % SIZE];
                    }
                }
            }
        } else {
            /* No inner loop here - creates outer loop blocks not in any inner loop */
            arr1[i % SIZE] = arr2[i % SIZE] * i;
            external_call(arr1[i % SIZE]);
        }
        
        /* Additional outer loop computation */
        checksum += arr1[i % SIZE];
    }
}

/* Test case 2: Loops with intersecting but not contained blocks */
void test_intersecting_cousins(void) {
    volatile int matrix[SIZE][SIZE];
    volatile int temp[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        temp[i] = i * 2;
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Outer loop with two inner loops that are cousins */
    for (int outer = 0; outer < 30; outer++) {
        /* First conditional region */
        if (outer % 4 < 2) {
            /* First inner loop */
            for (int inner1 = outer; inner1 < outer + 10; inner1++) {
                matrix[outer][inner1 % SIZE] = temp[inner1 % SIZE];
                results[counter++ % SIZE] = matrix[outer][inner1 % SIZE];
                
                /* Conditional inside inner1 */
                if (inner1 % 5 == 0) {
                    temp[inner1 % SIZE] = matrix[outer][inner1 % SIZE] * 3;
                }
            }
        }
        
        /* Code between the two inner loops - part of outer but not in inner1 */
        temp[outer % SIZE] = checksum + outer;
        
        /* Second conditional region */
        if (outer % 4 >= 2) {
            /* Second inner loop - cousin of first, sharing some outer blocks */
            for (int inner2 = 0; inner2 < 15; inner2++) {
                matrix[inner2 % SIZE][outer % SIZE] = temp[inner2 % SIZE];
                checksum += matrix[inner2 % SIZE][outer % SIZE];
                
                /* Different control flow pattern */
                if (inner2 % 3 == 0) {
                    for (int deep = 0; deep < 5; deep++) {
                        results[(counter + deep) % SIZE] = 
                            matrix[inner2 % SIZE][outer % SIZE] + deep;
                    }
                }
            }
        } else {
            /* Alternative path for outer loop */
            external_call(temp[outer % SIZE]);
        }
    }
}

/* Test case 3: Complex diamond-shaped control flow with loops */
void test_diamond_nesting(void) {
    volatile int a[SIZE], b[SIZE], c[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = 0;
    }
    
    /* Main diamond structure */
    for (int i = 0; i < 40; i++) {
        /* Top of diamond */
        int selector = a[i % SIZE] % 4;
        
        switch (selector) {
            case 0:
                /* Path with deeply nested loops */
                for (int j = 0; j < 20; j++) {
                    c[j] = a[i % SIZE] + b[j];
                    if (j % 2 == 0) {
                        for (int k = 0; k < 8; k++) {
                            results[(i + j + k) % SIZE] = c[j] * k;
                            checksum += results[(i + j + k) % SIZE];
                        }
                    }
                }
                break;
                
            case 1:
                /* Path with sibling loops */
                for (int j = 5; j < 25; j++) {
                    a[j % SIZE] = b[j % SIZE] - i;
                }
                /* Another loop in same case but not nested */
                for (int j = 0; j < 10; j++) {
                    b[j] = a[j] * 2;
                }
                break;
                
            case 2:
                /* Path with partially overlapping loop */
                for (int j = i; j < i + 15; j++) {
                    if (j % 3 == 0) {
                        c[j % SIZE] = a[j % SIZE] + b[j % SIZE];
                        external_call(c[j % SIZE]);
                    } else {
                        c[j % SIZE] = a[j % SIZE] - b[j % SIZE];
                    }
                }
                break;
                
            default:
                /* Path without loops - creates outer blocks not in any inner loop */
                a[i % SIZE] = b[i % SIZE] * 3;
                checksum -= a[i % SIZE];
                break;
        }
        
        /* Common tail code for all paths */
        results[i % SIZE] = checksum % 1000;
    }
}

/* Test case 4: Loop with early exits creating partial containment */
void test_early_exit_nesting(void) {
    volatile int data[SIZE * 2];
    
    /* Initialize */
    for (int i = 0; i < SIZE * 2; i++) {
        data[i] = i * 3;
    }
    
    /* Outer loop with early exit conditions */
    for (int outer = 0; outer < 60; outer++) {
        /* Early exit check */
        if (data[outer] > 1000) {
            break;
        }
        
        /* Inner loop that may exit early */
        for (int inner = 0; inner < 25; inner++) {
            if (inner > outer * 2) {
                /* Early exit from inner loop */
                break;
            }
            
            data[inner + outer] = data[inner] + data[outer];
            
            /* Deeply nested conditional loop */
            if (inner % 4 == 0) {
                for (int deep = 0; deep < 6; deep++) {
                    results[(outer + inner + deep) % SIZE] = 
                        data[inner + outer] * deep;
                    checksum += results[(outer + inner + deep) % SIZE];
                    
                    /* Very deep nesting */
                    if (deep % 2 == 0) {
                        volatile int temp = 0;
                        for (int very_deep = 0; very_deep < 3; very_deep++) {
                            temp += data[very_deep] * deep;
                        }
                        data[inner] += temp;
                    }
                }
            } else {
                /* Alternative path in inner loop */
                external_call(data[inner]);
            }
        }
        
        /* More outer loop code after inner loop */
        if (outer % 7 == 0) {
            for (int k = 0; k < 5; k++) {
                data[outer + k] = checksum % 100;
            }
        }
    }
}

/* Dummy external function to prevent optimization */
void external_call(int x) {
    /* Use volatile to prevent optimization */
    volatile static int dummy = 0;
    dummy += x;
}

int main(void) {
    /* Seed random number generator */
    srand(time(NULL));
    
    printf("Starting hardware loop analysis test...\n");
    
    /* Run all test cases to create various loop nesting patterns */
    test_partial_overlap_nested();
    printf("Test 1 complete. Checksum: %d\n", checksum);
    
    test_intersecting_cousins();
    printf("Test 2 complete. Checksum: %d\n", checksum);
    
    test_diamond_nesting();
    printf("Test 3 complete. Checksum: %d\n", checksum);
    
    test_early_exit_nesting();
    printf("Test 4 complete. Checksum: %d\n", checksum);
    
    /* Final validation */
    int final_check = 0;
    for (int i = 0; i < SIZE; i++) {
        final_check += results[i];
    }
    
    printf("Final result: %d (checksum: %d)\n", final_check, checksum);
    
    return 0;
}
