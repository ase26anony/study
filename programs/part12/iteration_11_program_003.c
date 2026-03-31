/* Test program for hardware loop optimization analysis
 * Specifically targets bitmap_intersect_compl_p logic in hw-doloop.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define NEST_LEVELS 4

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[SIZE][SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 7;
}

/* Test case 1: Simple nested loops with partial overlap */
void test_partial_overlap_nested() {
    int i, j, k;
    
    /* Outer loop - will contain blocks not in inner loops */
    for (i = 0; i < 50; ++i) {
        /* Conditional that creates separate basic blocks */
        if (get_condition(i, 0) > 2) {
            /* First inner loop - partially overlapping with outer */
            for (j = 0; j < 30; ++j) {
                results[i][j] = i * j + counter++;
                /* Additional conditional inside inner loop */
                if (j % 3 == 0) {
                    checksum += results[i][j];
                }
            }
        } else {
            /* Alternative path in outer loop - not in inner loop */
            results[i][0] = i * i - counter++;
            checksum -= results[i][0];
            
            /* Second inner loop at same nesting level */
            for (k = 10; k < 25; ++k) {
                results[i][k] = i + k * 2;
                checksum ^= results[i][k];
            }
        }
        
        /* Additional code in outer loop after inner loops */
        if (i % 5 == 0) {
            counter += 7;
        }
    }
}

/* Test case 2: Three-level nesting with varying overlap patterns */
void test_three_level_nesting() {
    int a, b, c;
    
    /* Level 1: Outermost loop */
    for (a = 0; a < 20; ++a) {
        /* Some outer loop code */
        results[a][0] = a * 2;
        
        /* Level 2: Middle loop - partially contained in outer */
        for (b = 0; b < 15; ++b) {
            /* Conditional that creates partial overlap */
            if (get_condition(a, b) < 4) {
                /* Level 3: Innermost loop - fully contained in middle */
                for (c = 0; c < 10; ++c) {
                    results[a][b] += c * 3;
                    checksum += results[a][b];
                }
            } else {
                /* Alternative path in middle loop */
                results[a][b] = b * 5;
                checksum -= results[a][b];
            }
            
            /* More code in middle loop after inner */
            if (b % 2 == 0) {
                counter++;
            }
        }
        
        /* More outer loop code not in middle loop */
        if (a % 3 == 0) {
            checksum *= 2;
        }
    }
}

/* Test case 3: Sibling loops with partial overlap through shared outer */
void test_sibling_loops() {
    int x, y, z;
    
    /* Outer loop containing two sibling inner loops */
    for (x = 0; x < 25; ++x) {
        volatile int temp = 0;
        
        /* First sibling loop - executes conditionally */
        if (x % 3 != 0) {
            for (y = 0; y < 12; ++y) {
                results[x][y] = x * y + y;
                temp += results[x][y];
            }
        }
        
        /* Code between sibling loops - in outer but not in first inner */
        checksum += temp;
        temp = x * 11;
        
        /* Second sibling loop - different condition */
        if (x % 4 != 0) {
            for (z = 5; z < 18; ++z) {
                results[x][z] = x + z * 7;
                temp -= results[x][z];
            }
        }
        
        /* Final outer loop code */
        checksum ^= temp;
    }
}

/* Test case 4: Complex nested structure with multiple exits */
void test_complex_nesting() {
    int p, q, r;
    
    for (p = 0; p < 15; ++p) {
        /* Early continue creates additional basic blocks */
        if (p % 6 == 0) {
            counter += 3;
            continue;
        }
        
        for (q = p; q < 12; ++q) {
            /* Nested conditional with inner loop */
            if (get_condition(p, q) > 1) {
                for (r = 0; r < 8; ++r) {
                    results[p][q] += r * p * q;
                    if (r % 2 == 0) {
                        checksum++;
                    } else {
                        checksum--;
                    }
                }
            } else {
                /* Alternative without innermost loop */
                results[p][q] = p - q;
                checksum += results[p][q];
            }
            
            /* Early break from middle loop */
            if (q > 8 && get_condition(p, q) < 2) {
                break;
            }
        }
        
        /* Post-middle loop code */
        results[p][0] = p * 100;
    }
}

/* Test case 5: Loop with invariant code motion challenges */
void test_invariant_challenges() {
    int i, j;
    volatile int invariant = rand() % 10 + 1;  /* Volatile prevents hoisting */
    
    for (i = 0; i < 30; ++i) {
        /* Code that looks invariant but has volatile dependency */
        int base = invariant * i;
        
        for (j = 0; j < 20; ++j) {
            /* Inner loop uses 'base' which changes with outer i */
            results[i][j] = base + j;
            checksum += results[i][j];
            
            /* Conditional that creates partial block overlap */
            if (j % invariant == 0) {
                counter += results[i][j];
            }
        }
        
        /* Outer loop code not in inner */
        invariant = (invariant + 1) % 5 + 1;  /* Change volatile */
    }
}

int main() {
    /* Initialize random seed for get_condition function */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            results[i][j] = 0;
        }
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_partial_overlap_nested();
    printf("Test 1 complete, checksum: %d\n", checksum);
    
    test_three_level_nesting();
    printf("Test 2 complete, checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 3 complete, checksum: %d\n", checksum);
    
    test_complex_nesting();
    printf("Test 4 complete, checksum: %d\n", checksum);
    
    test_invariant_challenges();
    printf("Test 5 complete, checksum: %d\n", checksum);
    
    /* Final verification */
    int final_check = 0;
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 20; ++j) {
            final_check += results[i][j];
        }
    }
    
    printf("Final check: %d, Counter: %d\n", final_check, counter);
    printf("All tests completed successfully.\n");
    
    return 0;
}
