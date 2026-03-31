/* Test program for hardware loop optimization analysis */
/* Designed to trigger bitmap_intersect_compl_p checks in hw-doloop.cc */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 5;
}

/* Function with side effects */
void record_result(int value) {
    results[counter++ % 1000] = value;
    checksum ^= value;
}

/* Test case 1: Nested loops with partial overlap in outer loop */
void test_partial_overlap_nested(void) {
    int i, j, k;
    volatile int temp = 0;
    
    /* Outer loop with conditional inner loops */
    for (i = 0; i < 50; ++i) {
        /* This block is always executed in outer loop */
        temp += i * 3;
        record_result(i);
        
        /* Conditional inner loop - creates partial overlap */
        if (get_condition(i, 0) > 2) {
            /* Inner loop j - fully contained in this branch */
            for (j = 0; j < 30; ++j) {
                temp += j * 2;
                record_result(j);
                
                /* Innermost loop k - fully contained in j loop */
                if (get_condition(i, j) < 3) {
                    for (k = 0; k < 20; ++k) {
                        temp += k;
                        record_result(k);
                    }
                } else {
                    /* Alternative path in j loop without k loop */
                    temp += 1000;
                }
            }
        } else {
            /* Alternative path in outer loop without j loop */
            temp += i * 100;
            for (k = 0; k < 10; ++k) {
                temp -= k;
                record_result(k + 1000);
            }
        }
        
        /* More code in outer loop after conditional */
        temp = (temp * 7) % 100;
    }
    
    results[0] = temp;
}

/* Test case 2: Sibling loops with shared outer loop blocks */
void test_sibling_loops(void) {
    int a, b, c;
    volatile int acc = 0;
    
    for (a = 0; a < 40; ++a) {
        acc += a;
        record_result(a + 2000);
        
        /* First sibling loop - executes based on condition */
        if (a % 3 == 0) {
            for (b = 0; b < 25; ++b) {
                acc += b * a;
                record_result(b + 3000);
                
                /* Conditional to create variation in basic blocks */
                if (b % 4 == 0) {
                    acc += 777;
                }
            }
        }
        
        /* Code between sibling loops */
        acc = (acc * 3) % 1000;
        
        /* Second sibling loop - different condition */
        if (a % 4 == 1) {
            for (c = 0; c < 20; ++c) {
                acc -= c * a;
                record_result(c + 4000);
                
                /* Nested conditional inside sibling */
                if (c % 5 == 0) {
                    for (b = 0; b < 15; ++b) {
                        acc += b * c;
                        record_result(b + 5000);
                    }
                }
            }
        }
        
        /* Final code in outer loop */
        acc ^= 0xABCD;
    }
    
    results[1] = acc;
}

/* Test case 3: Complex nesting with multiple exit points */
void test_complex_nesting(void) {
    int x, y, z;
    volatile int val = 0;
    
    for (x = 0; x < 35; ++x) {
        val += x * x;
        
        /* Multiple levels of conditionals before inner loop */
        if (x % 2 == 0) {
            if (x % 3 == 0) {
                for (y = 0; y < 28; ++y) {
                    val += y * x;
                    record_result(y + 6000);
                    
                    /* Early continue creates additional basic blocks */
                    if (y % 6 == 0) continue;
                    
                    /* Deeply nested loop with condition */
                    if (y % 7 == 0) {
                        for (z = 0; z < 22; ++z) {
                            val += z * y;
                            record_result(z + 7000);
                            
                            /* Break statement creates exit block */
                            if (z == 10) break;
                        }
                    }
                }
            } else {
                /* Different inner loop structure */
                for (z = 0; z < 18; ++z) {
                    val -= z * x;
                    record_result(z + 8000);
                }
            }
        } else {
            /* Path without y loop but with z loop */
            for (z = 0; z < 12; ++z) {
                val += z * 3;
                record_result(z + 9000);
            }
        }
        
        /* Loop-invariant code that can't be hoisted due to volatile */
        val ^= results[x % 100];
    }
    
    results[2] = val;
}

/* Test case 4: Interleaved loop structures */
void test_interleaved_loops(void) {
    int p, q, r;
    volatile int sum = 0;
    
    /* Outer loop p */
    for (p = 0; p < 30; ++p) {
        sum += p;
        
        /* First inner loop q */
        for (q = 0; q < 24; ++q) {
            sum += q * p;
            
            /* Conditional that sometimes executes, sometimes doesn't */
            if ((p + q) % 5 != 0) {
                /* Innermost loop r - not always executed */
                for (r = 0; r < 16; ++r) {
                    sum += r;
                    record_result(r + 10000);
                    
                    /* Conditional inside r loop */
                    if (r % 3 == 0) {
                        sum += 99;
                    } else {
                        sum -= 33;
                    }
                }
            } else {
                /* Alternative path in q loop */
                sum += 555;
            }
            
            /* More code in q loop after conditional */
            sum = (sum * 2) % 10000;
        }
        
        /* Additional loop at same nesting level as q but different scope */
        if (p % 7 == 0) {
            for (r = 0; r < 14; ++r) {
                sum -= r * p;
                record_result(r + 11000);
            }
        }
    }
    
    results[3] = sum;
}

int main(void) {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Initialize results array */
    for (int i = 0; i < 1000; ++i) {
        results[i] = i;
    }
    
    printf("Starting hardware loop analysis tests...\n");
    
    /* Execute all test cases to create various loop nesting patterns */
    test_partial_overlap_nested();
    printf("Test 1 complete. Checksum: %d\n", checksum);
    
    test_sibling_loops();
    printf("Test 2 complete. Checksum: %d\n", checksum);
    
    test_complex_nesting();
    printf("Test 3 complete. Checksum: %d\n", checksum);
    
    test_interleaved_loops();
    printf("Test 4 complete. Checksum: %d\n", checksum);
    
    /* Final computation to ensure all results are used */
    volatile int final = 0;
    for (int i = 0; i < 100; ++i) {
        final += results[i];
    }
    
    printf("Final result: %d (Checksum: %d)\n", final, checksum);
    printf("Test program completed successfully.\n");
    
    return 0;
}
