/* Test program for hardware loop analysis - partial basic block overlap scenarios */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100
#define NEST_LEVELS 3

/* Global arrays to prevent optimization */
volatile int results[SIZE][SIZE];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create side effects */
void process_value(int value) {
    checksum ^= value;
    counter++;
}

/* Complex nested loop with partial overlap pattern 1 */
void nested_pattern_1(int n) {
    int i, j, k;
    
    for (i = 0; i < n; i++) {  /* Outer loop L1 */
        /* Code before inner loops - creates blocks outside inner loops */
        results[i][0] = rand() % 100;
        
        if (i % 3 == 0) {  /* Conditional creates branch structure */
            /* First inner loop L2 - partially overlaps with L1 */
            for (j = 0; j < n/2; j++) {
                /* Code inside L2 but not in L3 */
                results[i][j] = i * j + rand() % 10;
                
                if (j % 2 == 0) {  /* Another conditional */
                    /* Innermost loop L3 - fully contained in L2 */
                    for (k = 0; k < n/4; k++) {
                        results[i][j] += k;
                        process_value(k);
                    }
                } else {
                    /* Alternative path in L2 but not in L3 */
                    results[i][j] -= i;
                    process_value(j);
                }
                
                /* More code in L2 after the if/else */
                results[i][j] *= 2;
            }
        } else if (i % 3 == 1) {
            /* Second inner loop L4 - sibling to L2, partially overlaps with L1 */
            for (j = n/2; j < n; j++) {
                results[i][j] = i - j;
                process_value(results[i][j]);
                
                /* Conditional with early exit creates more blocks */
                if (results[i][j] < 0) {
                    results[i][j] = 0;
                    break;  /* Creates exit block */
                }
            }
        } else {
            /* Code path in L1 with no inner loops */
            results[i][i] = i * i;
            process_value(results[i][i]);
        }
        
        /* More code in L1 after the if/else chain */
        results[i][n-1] = checksum;
    }
}

/* Pattern 2: Nested loops with cross-overlapping control flow */
void nested_pattern_2(int n) {
    int a, b, c;
    
    for (a = 0; a < n; a++) {  /* Loop A */
        volatile int temp = a;
        
        for (b = 0; b < n; b++) {  /* Loop B - fully contained in A */
            /* Complex condition that splits B's body */
            if ((a + b) % 4 == 0) {
                /* Loop C - partially contained in B */
                for (c = 0; c < b; c++) {
                    results[a][b] += c * temp;
                    temp++;
                }
                
                /* Code in B but not in C */
                results[a][b] *= 3;
            } else if ((a + b) % 4 == 1) {
                /* Different inner loop D - also partially in B */
                for (c = b; c < n; c++) {
                    results[b][c] = a - c;
                    process_value(results[b][c]);
                }
            } else {
                /* Code in B with no inner loops */
                results[a][b] = a * b;
            }
            
            /* More B code after the if/else */
            if (b % 5 == 0) {
                process_value(b);
            }
        }
        
        /* A code that's not in B */
        if (a % 7 == 0) {
            for (b = n/3; b < 2*n/3; b++) {  /* Another loop E - partially overlaps with A */
                results[a][b] = rand() % 50;
            }
        }
    }
}

/* Pattern 3: Switch statement with loops in different cases */
void nested_pattern_3(int n) {
    int x, y, z;
    
    for (x = 0; x < n; x++) {  /* Loop X */
        int selector = x % 4;
        
        switch (selector) {
            case 0:
                /* Loop Y - fully contained in X */
                for (y = 0; y < x; y++) {
                    results[x][y] = x ^ y;
                    
                    /* Loop Z - partially contained in Y */
                    for (z = 0; z < y/2; z++) {
                        results[x][y] += z;
                        process_value(z);
                    }
                    
                    /* Y code not in Z */
                    if (y % 3 == 0) {
                        results[x][y] -= x;
                    }
                }
                break;
                
            case 1:
                /* Different loop structure */
                for (y = n-1; y >= 0; y--) {
                    results[x][y] = x + y;
                    process_value(results[x][y]);
                }
                break;
                
            case 2:
                /* Two sibling loops in X */
                for (y = 0; y < n/2; y++) {
                    results[x][y] = x * y;
                }
                
                for (y = n/2; y < n; y++) {
                    results[x][y] = x / (y + 1);
                }
                break;
                
            default:
                /* X code with no inner loops */
                results[x][x] = checksum;
                break;
        }
        
        /* More X code after switch */
        process_value(x);
    }
}

/* Helper to ensure loops execute */
void verify_results(int n) {
    int i, j;
    int sum = 0;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            sum += results[i][j];
        }
    }
    
    printf("Checksum: %d, Counter: %d, Array sum: %d\n", 
           checksum, counter, sum);
}

int main() {
    /* Seed RNG for unpredictable but reproducible behavior */
    srand(time(NULL));
    
    printf("Testing hardware loop analysis with partial block overlap\n");
    printf("========================================================\n");
    
    /* Clear results array */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            results[i][j] = 0;
        }
    }
    
    checksum = 0;
    counter = 0;
    
    /* Execute different patterns to create various overlap scenarios */
    printf("\nPattern 1: Conditional inner loops with partial overlap\n");
    nested_pattern_1(SIZE/2);
    verify_results(SIZE/2);
    
    printf("\nPattern 2: Cross-overlapping loops with complex conditions\n");
    nested_pattern_2(SIZE/3);
    verify_results(SIZE/3);
    
    printf("\nPattern 3: Switch-based loop nesting\n");
    nested_pattern_3(SIZE/4);
    verify_results(SIZE/4);
    
    /* Additional test with varying bounds */
    printf("\nMixed patterns with different bounds:\n");
    for (int iter = 0; iter < 5; iter++) {
        int bound = 10 + iter * 5;
        
        /* Reset partial state */
        checksum = iter;
        
        /* Quick nested test */
        for (int i = 0; i < bound; i++) {
            if (i % 2 == 0) {
                for (int j = 0; j < bound/2; j++) {
                    results[i][j] = i + j;
                    if (j % 3 == 0) {
                        for (int k = 0; k < j; k++) {
                            results[i][j] += k;
                        }
                    }
                }
            } else {
                for (int j = bound/2; j < bound; j++) {
                    results[i][j] = i - j;
                }
            }
        }
        
        printf("  Iteration %d: checksum=%d\n", iter, checksum);
    }
    
    printf("\nTest completed successfully.\n");
    return 0;
}
