/* Test program for hardware loop analysis with partial basic block overlap */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to prevent optimization and create side effects */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 3;
}

/* Function with side effects that can't be optimized away */
void record_result(int value) {
    results[counter++ % 1000] = value;
    checksum ^= value;
}

/* Test case 1: Three-level nested loops with conditional inner loop execution */
void test_case_1(int N) {
    volatile int sum = 0;
    
    /* Outer loop - will contain some but not all blocks of inner loops */
    for (int i = 0; i < N; i++) {
        /* Code executed in outer loop but not in inner loops */
        sum += i * 2;
        record_result(i);
        
        /* Conditional execution of first inner loop */
        if (get_condition(i, 0) != 0) {
            /* First inner loop - partially overlaps with outer */
            for (int j = 0; j < N/2; j++) {
                /* Code only in inner loop */
                sum += j * 3;
                record_result(j + 1000);
                
                /* Conditional execution of deepest loop */
                if (get_condition(i, j) > 0) {
                    /* Deepest loop - fully contained in first inner loop */
                    for (int k = 0; k < N/4; k++) {
                        sum += k * 5;
                        record_result(k + 2000);
                    }
                } else {
                    /* Alternative path in first inner loop */
                    sum += 7;
                }
            }
        } else {
            /* Alternative path in outer loop - creates blocks not in inner loops */
            sum += i * 11;
            record_result(i + 3000);
            
            /* Second inner loop (sibling to first) */
            for (int j = N/2; j < N; j++) {
                sum += j * 13;
                record_result(j + 4000);
                
                /* This creates partial overlap with outer but different from first inner */
                if (j % 2 == 0) {
                    sum += 17;
                }
            }
        }
        
        /* More outer loop code not in any inner loop */
        sum += i * 19;
    }
    
    record_result(sum);
}

/* Test case 2: Complex overlapping loop structure */
void test_case_2(int M) {
    volatile int prod = 1;
    
    /* Outer loop A */
    for (int a = 0; a < M; a++) {
        prod *= (a + 1);
        record_result(a + 5000);
        
        /* Inner loop B - partially contained in A */
        for (int b = 0; b < M; b++) {
            if (b % 3 == a % 3) {
                prod += b;
                record_result(b + 6000);
                
                /* Loop C - sometimes executed, sometimes not */
                for (int c = 0; c < b; c++) {
                    prod -= c;
                    record_result(c + 7000);
                }
            } else {
                /* Alternative path in loop B */
                prod *= 2;
            }
        }
        
        /* Additional outer loop code */
        if (a % 4 == 0) {
            /* Another inner loop D - sibling to B */
            for (int d = M/2; d < M; d++) {
                prod += d * d;
                record_result(d + 8000);
            }
        }
    }
    
    record_result(prod);
}

/* Test case 3: Switch statement creating multiple loop paths */
void test_case_3(int P) {
    volatile int acc = 0;
    
    for (int x = 0; x < P; x++) {
        switch (x % 4) {
            case 0:
                /* Path with inner loop E */
                for (int e = 0; e < P/3; e++) {
                    acc += e * x;
                    record_result(e + 9000);
                }
                break;
                
            case 1:
                /* Path with different inner loop F */
                for (int f = P/3; f < 2*P/3; f++) {
                    acc -= f * x;
                    record_result(f + 10000);
                    
                    /* Nested loop inside F */
                    for (int g = 0; g < f/2; g++) {
                        acc += g;
                        record_result(g + 11000);
                    }
                }
                break;
                
            case 2:
                /* Path with no inner loop */
                acc += x * x;
                break;
                
            default:
                /* Path with multiple inner loops */
                for (int h = 0; h < P/4; h++) {
                    acc += h;
                }
                for (int i = P/4; i < P/2; i++) {
                    acc -= i;
                }
                break;
        }
        
        /* Common outer loop code */
        acc += 1;
    }
    
    record_result(acc);
}

/* Test case 4: Do-while loops with complex conditions */
void test_case_4(int Q) {
    volatile int val = 0;
    int r = 0;
    
    do {
        val += r * 3;
        record_result(r + 12000);
        
        int s = 0;
        while (s < Q) {
            val += s * 5;
            record_result(s + 13000);
            
            /* Conditional inner do-while */
            int t = 0;
            if (s % 2 == 0) {
                do {
                    val -= t;
                    record_result(t + 14000);
                    t++;
                } while (t < s/2);
            } else {
                val += 7;
            }
            
            s++;
        }
        
        r++;
    } while (r < Q);
    
    record_result(val);
}

int main() {
    /* Seed random for unpredictable but reproducible behavior */
    srand(42);
    
    /* Initialize results array */
    for (int i = 0; i < 1000; i++) {
        results[i] = 0;
    }
    
    /* Execute test cases with different parameters to create various loop structures */
    printf("Starting hardware loop analysis tests...\n");
    
    /* Test 1: Medium-sized loops */
    test_case_1(50);
    
    /* Test 2: Smaller loops */
    test_case_2(30);
    
    /* Test 3: Different loop structure */
    test_case_3(40);
    
    /* Test 4: Do-while loops */
    test_case_4(25);
    
    /* Final computation to ensure all loops have side effects */
    volatile int final = 0;
    for (int i = 0; i < 1000; i++) {
        final += results[i];
    }
    
    printf("Test completed. Checksum: %d, Final: %d\n", checksum, final);
    
    /* Return non-zero if something went wrong (simplified check) */
    return (final == 0) ? 1 : 0;
}
