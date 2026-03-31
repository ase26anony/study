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

/* External function to prevent optimization */
extern void external_call(int);

/* Function to create unpredictable but bounded conditions */
static inline int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 7;
}

/* Test case 1: Three-level nesting with partial overlap */
void test_nested_partial_overlap(void) {
    volatile int arr1[100];
    volatile int arr2[100];
    
    /* Outer loop - will contain partial blocks of inner loops */
    for (int i = 0; i < 50; ++i) {
        /* Code block that's in outer loop but NOT in inner loop A */
        arr1[i] = rand() % 100;
        checksum += arr1[i];
        
        /* Conditional that creates partial overlap */
        if (get_condition(i, 0) > 2) {
            /* Inner loop A - fully contained in this branch */
            for (int j = 0; j < 30; ++j) {
                /* Code only in inner loop A */
                arr2[j] = i * j;
                results[counter++] = arr2[j];
                
                /* Another conditional inside inner loop */
                if ((i + j) % 3 == 0) {
                    /* Innermost loop - creates three-level hierarchy */
                    for (int k = 0; k < 10; ++k) {
                        results[counter++] = i * j * k;
                        external_call(k);
                    }
                } else {
                    /* Alternative path in inner loop A */
                    results[counter++] = -1;
                }
            }
        } else {
            /* Alternative branch - creates blocks in outer loop 
               that are NOT in inner loop A */
            arr1[i] *= 2;
            external_call(i);
            
            /* Different inner loop B - sibling to loop A */
            for (int j = 5; j < 25; ++j) {
                results[counter++] = i - j;
                /* This creates partial overlap with outer loop's blocks */
                if (j % 4 == 0) {
                    external_call(j);
                }
            }
        }
        
        /* More code in outer loop after the conditional */
        arr1[i] += i;
        checksum ^= arr1[i];
    }
}

/* Test case 2: Complex sibling loops with shared parent */
void test_sibling_loops(void) {
    volatile int temp[50];
    
    /* Parent loop */
    for (int x = 0; x < 40; ++x) {
        temp[x] = x * x;
        
        /* First sibling loop - executes conditionally */
        if (x % 3 == 0) {
            for (int y = 0; y < 20; ++y) {
                results[counter++] = x + y;
                /* Code that might be hoisted but isn't due to volatile */
                temp[y % 50] = rand() % 100;
            }
        }
        
        /* Code between sibling loops - in parent but not in first sibling */
        external_call(x);
        
        /* Second sibling loop - different condition */
        if (x % 4 == 1) {
            for (int z = 10; z < 30; ++z) {
                results[counter++] = x * z;
                /* Different computation to prevent merging */
                temp[z % 50] = (x << 3) | (z & 0xF);
            }
        } else {
            /* Alternative path with no inner loop */
            temp[x % 50] = ~x;
        }
        
        /* Final code in parent loop */
        checksum += temp[x % 50];
    }
}

/* Test case 3: Overlapping but not nested loops */
void test_overlapping_cousins(void) {
    volatile int buffer[100];
    
    /* First loop */
    for (int a = 0; a < 60; a += 2) {
        buffer[a] = a * 3;
        results[counter++] = buffer[a];
        
        /* Conditional that might execute cousin loop */
        if (a % 5 == 0) {
            /* Cousin loop - shares some blocks but not all */
            for (int b = a; b < a + 10; ++b) {
                buffer[b % 100] = b * 7;
                external_call(b);
                
                /* This creates intersection but not containment */
                if (b % 3 == 0) {
                    results[counter++] = -b;
                }
            }
        }
    }
    
    /* Intervening code */
    external_call(999);
    
    /* Second loop that overlaps with first in some blocks */
    for (int c = 20; c < 70; c += 3) {
        buffer[c] = c * 11;
        checksum ^= buffer[c];
        
        /* Similar conditional structure creates bitmap intersection */
        if (c % 6 == 0) {
            for (int d = 5; d < 15; ++d) {
                results[counter++] = c * d;
                buffer[(c + d) % 100] = rand() % 256;
            }
        }
    }
}

/* Test case 4: Switch statement with loops in different cases */
void test_switch_with_loops(void) {
    volatile int values[4][20];
    
    for (int outer = 0; outer < 25; ++outer) {
        /* Switch creates multiple basic blocks in outer loop */
        switch (outer % 4) {
            case 0:
                /* Loop in case 0 */
                for (int inner = 0; inner < 15; ++inner) {
                    values[0][inner] = outer * inner;
                    results[counter++] = values[0][inner];
                }
                break;
                
            case 1:
                /* Different loop structure in case 1 */
                for (int inner = 5; inner < 20; ++inner) {
                    values[1][inner - 5] = outer + inner;
                    external_call(inner);
                }
                /* Extra code in this case only */
                checksum += outer;
                break;
                
            case 2:
                /* Nested loops in case 2 */
                for (int i = 0; i < 10; ++i) {
                    values[2][i] = i * i;
                    for (int j = 0; j < 5; ++j) {
                        values[3][j] = i * j * outer;
                        results[counter++] = values[3][j];
                    }
                }
                break;
                
            default:
                /* No loop in default case */
                values[3][outer % 20] = -outer;
                break;
        }
        
        /* Common code after switch */
        external_call(outer % 8);
    }
}

int main(void) {
    /* Initialize random seed for unpredictable but deterministic behavior */
    srand(42);
    
    printf("Starting hardware loop analysis test...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_nested_partial_overlap();
    printf("Completed test 1, counter = %d\n", counter);
    
    test_sibling_loops();
    printf("Completed test 2, counter = %d\n", counter);
    
    test_overlapping_cousins();
    printf("Completed test 3, counter = %d\n", counter);
    
    test_switch_with_loops();
    printf("Completed test 4, counter = %d\n", counter);
    
    /* Final computation to ensure all results are used */
    volatile int final = 0;
    for (int i = 0; i < counter && i < 1000; ++i) {
        final += results[i];
    }
    
    printf("Final checksum: %d, Result sum: %d\n", checksum, final);
    printf("Test completed successfully.\n");
    
    return 0;
}

/* Dummy external function definition */
void external_call(int x) {
    /* Volatile to prevent optimization */
    volatile static int dummy = 0;
    dummy += x;
}
