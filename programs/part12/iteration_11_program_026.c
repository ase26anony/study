/* Test program to trigger hardware loop nesting analysis in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to prevent optimization */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create side effects */
int __attribute__((noinline)) do_work(int x, int y) {
    return (x * 1103515245 + 12345) ^ (y * 1664525 + 1013904223);
}

/* Function with conditional control flow */
void __attribute__((noinline)) conditional_inner(int i, int j, int mode) {
    volatile int temp = 0;
    
    if (mode & 1) {
        /* This block will be part of outer loop but not inner loop */
        temp = do_work(i, j);
        results[counter++ % 1000] = temp;
    } else {
        /* Different computation - creates separate basic blocks */
        temp = do_work(j, i);
        checksum += temp;
    }
}

/* Test case 1: Three-level nesting with partial overlap */
void test_nested_partial_overlap(void) {
    int i, j, k;
    volatile int acc = 0;
    
    /* Outer loop - will contain blocks not in inner loops */
    for (i = 0; i < 50; ++i) {
        /* Conditional that creates partial overlap */
        if (i % 3 == 0) {
            /* Middle loop - partially overlapping with outer */
            for (j = 0; j < 30; ++j) {
                /* Another conditional for more complex overlap */
                if (j % 2 == 0) {
                    /* Innermost loop - fully contained in middle */
                    for (k = 0; k < 20; ++k) {
                        acc += do_work(i, j + k);
                        results[(i * j + k) % 1000] = acc;
                    }
                } else {
                    /* This block is in middle loop but not innermost */
                    acc -= do_work(j, i);
                    checksum ^= acc;
                }
                
                /* Additional code in middle loop only */
                if (j == 15) {
                    conditional_inner(i, j, 1);
                }
            }
        } else if (i % 3 == 1) {
            /* Different inner loop structure - sibling to the j loop */
            for (int m = 0; m < 25; ++m) {
                acc += do_work(i, m * 2);
                results[(i * m) % 1000] = acc;
                
                /* Conditional inside creates more blocks */
                if (m % 5 == 0) {
                    conditional_inner(i, m, 2);
                }
            }
        } else {
            /* Code in outer loop only - not in any inner loop */
            acc = do_work(i, i);
            checksum += acc * 2;
        }
        
        /* More outer-loop-only code */
        if (i % 10 == 0) {
            results[i % 1000] = checksum;
        }
    }
    
    results[999] = acc;
}

/* Test case 2: Complex sibling loops with shared parent */
void test_sibling_loops(void) {
    int x, y, z;
    volatile int sum = 0;
    
    /* Outer loop containing two sibling inner loops */
    for (x = 0; x < 40; ++x) {
        /* First inner loop - conditionally executed */
        if (x % 4 != 0) {
            for (y = 0; y < 35; ++y) {
                sum += do_work(x, y);
                
                /* Conditional creates partial overlap */
                if (y % 3 == 0) {
                    /* Very inner loop */
                    for (z = 0; z < 15; ++z) {
                        results[(x + y + z) % 1000] = do_work(z, x);
                        sum ^= z;
                    }
                } else {
                    /* Different path in y-loop */
                    sum -= y * x;
                }
            }
        }
        
        /* Code between sibling loops */
        checksum += sum;
        
        /* Second inner loop - different condition */
        if (x % 4 != 1) {
            for (int a = 0; a < 25; ++a) {
                /* This loop shares some blocks with first inner loop 
                   but not all due to different conditionals */
                if (a % 2 == 0) {
                    sum += do_work(a, x);
                    results[(x * a) % 1000] = sum;
                } else {
                    /* Different computation path */
                    for (int b = 0; b < 10; ++b) {
                        sum -= do_work(b, a);
                    }
                }
            }
        }
        
        /* More outer loop code */
        if (x == 20) {
            conditional_inner(x, sum, 3);
        }
    }
    
    results[998] = sum;
}

/* Test case 3: Nested loops with early exits */
void test_loops_with_breaks(void) {
    int p, q, r;
    volatile int prod = 1;
    
    for (p = 0; p < 45; ++p) {
        /* Loop with break - creates additional basic blocks */
        for (q = 0; q < 40; ++q) {
            prod *= 2;
            
            if (prod > 1000000) {
                /* Early exit creates block in q-loop but not in r-loop */
                prod = 1;
                results[p % 1000] = q;
                break;
            }
            
            /* Innermost loop with continue */
            for (r = 0; r < 20; ++r) {
                if (r % 7 == 0) {
                    continue;  /* Creates additional flow control */
                }
                prod += do_work(p + q, r);
                checksum += prod;
            }
            
            /* Code after innermost loop but still in middle loop */
            if (q % 6 == 0) {
                conditional_inner(p, q, 4);
            }
        }
        
        /* Conditional with function call outside inner loops */
        if (p % 8 == 0) {
            prod += do_work(p, prod);
        }
    }
    
    results[997] = prod;
}

/* Test case 4: Switch statement inside loops */
void test_loops_with_switch(void) {
    int outer, inner;
    volatile int val = 0;
    
    for (outer = 0; outer < 35; ++outer) {
        /* Switch creates multiple basic blocks */
        switch (outer % 4) {
            case 0:
                /* Loop inside case 0 */
                for (inner = 0; inner < 30; ++inner) {
                    val += do_work(outer, inner);
                    results[(outer * 30 + inner) % 1000] = val;
                }
                break;
                
            case 1:
                /* Different loop structure */
                for (int a = 0; a < 20; ++a) {
                    for (int b = 0; b < 15; ++b) {
                        val -= do_work(a, b);
                        checksum ^= val;
                    }
                }
                break;
                
            case 2:
                /* Code without inner loop */
                val = do_work(outer, outer);
                conditional_inner(outer, val, 5);
                break;
                
            default:
                /* Yet another loop variant */
                for (int c = 0; c < 25; ++c) {
                    if (c % 3 == 0) {
                        val += c * outer;
                    } else {
                        val -= do_work(c, outer);
                    }
                }
                break;
        }
        
        /* Common outer loop code */
        if (outer % 5 == 0) {
            results[outer % 1000] = checksum + val;
        }
    }
    
    results[996] = val;
}

int main(void) {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < 1000; ++i) {
        results[i] = i;
    }
    
    printf("Starting hardware loop nesting tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_nested_partial_overlap();
    printf("Test 1 completed. Checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 2 completed. Checksum: %d\n", checksum);
    
    test_loops_with_breaks();
    printf("Test 3 completed. Checksum: %d\n", checksum);
    
    test_loops_with_switch();
    printf("Test 4 completed. Checksum: %d\n", checksum);
    
    /* Final computation using results to prevent dead code elimination */
    volatile int final = 0;
    for (int i = 0; i < 1000; ++i) {
        final ^= results[i];
    }
    
    printf("Final result: %d\n", final);
    printf("Test program completed successfully.\n");
    
    return 0;
}
