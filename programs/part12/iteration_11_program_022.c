/* Test program for hw-doloop.cc uncovered lines 429-436 */
/* Compile with: -O2 -fhwloops -fdump-rtl-hwloops -fdump-tree-loop */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define ITERS 50

/* Global arrays to prevent optimization */
volatile int results[SIZE][SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* External function to prevent optimization */
extern void external_call(int x);

/* Function with nested loops creating partial basic block overlap */
void test_partial_overlap_nesting(void) {
    int i, j, k;
    
    /* Outer loop - will contain multiple inner loops with partial overlap */
    for (i = 0; i < ITERS; i++) {
        /* First conditional branch - contains inner loop j */
        if (i % 3 == 0) {
            /* Inner loop j - shares some blocks with outer loop */
            for (j = 0; j < ITERS/2; j++) {
                /* Code inside inner loop */
                results[i][j] = i * j + counter;
                counter++;
                
                /* Nested deeper loop k - fully contained in j */
                if (j % 4 == 0) {
                    for (k = 0; k < 10; k++) {
                        results[j][k] = i + j + k;
                        external_call(k);
                    }
                } else {
                    /* Alternative path in j loop */
                    results[i][j] += rand() % 100;
                }
            }
        } else if (i % 3 == 1) {
            /* Second branch - different inner loop structure */
            for (j = ITERS/2; j > 0; j--) {
                results[j][i] = i - j + counter;
                counter--;
                
                /* Conditional with partial overlap */
                if (j % 5 == 0) {
                    for (k = 5; k < 15; k++) {
                        results[k][j] = i * k;
                    }
                }
            }
            
            /* Additional code in this branch not in any inner loop */
            results[i][i] = rand() % 1000;
        } else {
            /* Third branch - no inner loops, just computations */
            results[i][0] = i * i + counter;
            counter += 2;
            external_call(i);
        }
        
        /* Common code in outer loop after conditional */
        checksum += i;
    }
}

/* Function with sibling loops inside outer loop */
void test_sibling_loops(void) {
    int a, b, c;
    
    for (a = 0; a < 30; a++) {
        /* First sibling loop - conditionally executed */
        if (a % 2 == 0) {
            for (b = 0; b < 20; b++) {
                results[a][b] = a + b + checksum;
                checksum++;
                
                /* Deep nesting with partial exit */
                if (b % 3 == 0) {
                    for (c = 0; c < 8; c++) {
                        results[b][c] = a * c;
                        if (c == 4) break; /* Early exit creates different blocks */
                    }
                }
            }
        }
        
        /* Code between sibling loops */
        external_call(a);
        
        /* Second sibling loop - overlaps partially with first */
        if (a % 3 != 0) {
            for (b = 10; b < 25; b++) {
                results[b][a] = b - a + checksum;
                checksum--;
                
                /* Different control flow than first sibling */
                switch (b % 4) {
                    case 0:
                        results[a][b] = 0;
                        break;
                    case 1:
                        for (c = 2; c < 6; c++) {
                            results[c][b] = a + b + c;
                        }
                        break;
                    default:
                        results[a][b] = rand() % 100;
                }
            }
        }
        
        /* More outer loop code */
        checksum += a * 2;
    }
}

/* Function with complex three-level nesting and partial containment */
void test_complex_nesting(void) {
    int x, y, z;
    
    for (x = 0; x < 25; x++) {
        /* Level 1 conditional */
        if (x < 15) {
            for (y = x; y < 20; y++) {
                /* Level 2 - partial containment */
                results[x][y] = x * y + counter;
                
                /* Level 3 - fully contained but with early exits */
                for (z = 0; z < 12; z++) {
                    results[y][z] = x + y + z;
                    if (z == y % 10) {
                        break; /* Creates different exit block */
                    }
                    counter++;
                }
                
                /* Code in y loop but not in z loop */
                if (y % 7 == 0) {
                    external_call(y);
                    results[x][y] += 1000;
                }
            }
        } else {
            /* Alternative x loop path with different nesting */
            for (y = 0; y < 10; y++) {
                results[y][x] = x - y;
                
                /* Inner loop with multiple exits */
                for (z = 5; z > 0; z--) {
                    results[z][y] = x * y * z;
                    if (z == 2 && y == 3) {
                        goto skip_point; /* Non-standard control flow */
                    }
                }
                skip_point:
                checksum += y;
            }
        }
        
        /* Outer loop code not in any inner loop */
        results[x][x] = checksum;
        checksum += x;
    }
}

/* Function with loops that have overlapping but not contained blocks */
void test_overlapping_cousins(void) {
    int p, q, r;
    
    for (p = 0; p < 40; p++) {
        /* Two inner loops that are "cousins" - both in outer but overlapping */
        
        /* First inner loop */
        if (p % 4 == 0) {
            for (q = 0; q < 15; q++) {
                results[p][q] = p * q;
                counter++;
                
                /* Shared code with second inner loop */
                if (q == 7) {
                    external_call(p + q);
                }
            }
        }
        
        /* Code between cousin loops */
        checksum += p;
        
        /* Second inner loop - overlaps with first in some blocks */
        if (p % 4 == 2) {
            for (q = 5; q < 20; q++) {
                results[q][p] = q - p;
                
                /* Same pattern as in first inner loop */
                if (q == 7) {
                    external_call(p + q); /* Shared basic block */
                    results[q][p] += 500;
                } else {
                    /* Different block than first inner loop */
                    results[q][p] -= 100;
                }
            }
        } else if (p % 4 == 1) {
            /* Different path creating more variety */
            for (r = 0; r < 8; r++) {
                results[r][p] = r * p * 2;
            }
        }
    }
}

/* External function definition */
void external_call(int x) {
    /* Simple side effect */
    volatile static int ext_counter = 0;
    ext_counter += x;
    if (ext_counter > 1000000) ext_counter = 0;
}

int main(void) {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            results[i][j] = 0;
        }
    }
    
    printf("Starting hardware loop nesting tests...\n");
    
    /* Execute tests with different nesting patterns */
    test_partial_overlap_nesting();
    printf("Test 1 complete. Checksum: %d, Counter: %d\n", checksum, counter);
    
    test_sibling_loops();
    printf("Test 2 complete. Checksum: %d, Counter: %d\n", checksum, counter);
    
    test_complex_nesting();
    printf("Test 3 complete. Checksum: %d, Counter: %d\n", checksum, counter);
    
    test_overlapping_cousins();
    printf("Test 4 complete. Checksum: %d, Counter: %d\n", checksum, counter);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            final_sum += results[i][j];
        }
    }
    
    printf("Final array sum: %d\n", final_sum);
    printf("All tests completed successfully.\n");
    
    return 0;
}
