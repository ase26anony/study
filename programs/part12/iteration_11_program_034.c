#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global arrays to ensure side effects and prevent optimization */
volatile int results[1000];
volatile int checksum = 0;
volatile int counter = 0;

/* Function to create unpredictable but bounded conditions */
int get_condition(int i, int j) {
    return (i * 17 + j * 13) % 3;
}

/* Function with side effects */
void record_value(int idx, int val) {
    results[idx % 1000] = val;
    checksum ^= val;
}

int main() {
    srand(time(NULL));
    
    /* Pattern 1: Classic nested loops with partial overlap */
    printf("Pattern 1: Classic nested with conditional inner loop\n");
    for (int i = 0; i < 50; ++i) {
        /* Code before inner loop - creates blocks in outer but not inner */
        int pre_val = rand() % 100;
        record_value(i, pre_val);
        
        /* Conditional execution of inner loop */
        if (i % 3 != 0) {
            /* Inner loop - fully contained in this branch */
            for (int j = 0; j < 30; ++j) {
                int inner_val = i * j + pre_val;
                record_value(i * 100 + j, inner_val);
                counter += inner_val;
            }
        } else {
            /* Alternative path - creates blocks in outer but not inner */
            int alt_val = i * i;
            record_value(i + 1000, alt_val);
            counter -= alt_val;
        }
        
        /* Code after inner loop - more outer-only blocks */
        int post_val = pre_val * 2;
        record_value(i + 2000, post_val);
    }
    
    /* Pattern 2: Three-level nesting with varying overlap */
    printf("Pattern 2: Three-level nesting\n");
    for (int a = 0; a < 20; ++a) {
        volatile int outer_marker = a * 10;
        
        /* First inner loop - sometimes executed */
        if (get_condition(a, 0) > 0) {
            for (int b = 0; b < 15; ++b) {
                volatile int middle_marker = outer_marker + b;
                
                /* Innermost loop - conditionally executed */
                if (b % 2 == 0) {
                    for (int c = 0; c < 10; ++c) {
                        int val = a * 100 + b * 10 + c;
                        results[(a + b + c) % 1000] = val;
                        checksum += val;
                    }
                } else {
                    /* Alternative in middle loop */
                    results[(a + b) % 1000] = middle_marker;
                    checksum ^= middle_marker;
                }
            }
        } else {
            /* Alternative path in outer loop */
            results[a % 1000] = outer_marker * 2;
        }
    }
    
    /* Pattern 3: Sibling inner loops with partial overlap */
    printf("Pattern 3: Sibling inner loops\n");
    for (int x = 0; x < 25; ++x) {
        volatile int shared = x * 7;
        
        /* First sibling loop */
        if (x % 4 == 0 || x % 4 == 1) {
            for (int y = 0; y < 12; ++y) {
                int val = shared + y * 3;
                record_value(x * 50 + y, val);
                
                /* Additional condition inside sibling */
                if (y % 3 == 0) {
                    counter += val;
                } else {
                    counter -= val;
                }
            }
        }
        
        /* Code between siblings - in outer but not in either inner */
        int between = shared * 2;
        record_value(x + 3000, between);
        
        /* Second sibling loop (partially overlapping conditions) */
        if (x % 4 == 1 || x % 4 == 2) {
            for (int z = 0; z < 8; ++z) {
                int val = shared - z * 5;
                record_value(x * 50 + z + 100, val);
                
                /* Different internal structure than first sibling */
                switch (z % 3) {
                    case 0: counter += val * 2; break;
                    case 1: counter += val / 2; break;
                    case 2: counter += val; break;
                }
            }
        }
        
        /* Final code in outer loop */
        record_value(x + 4000, between + shared);
    }
    
    /* Pattern 4: Complex diamond-shaped nesting */
    printf("Pattern 4: Diamond-shaped nesting\n");
    for (int p = 0; p < 15; ++p) {
        int base = p * 11;
        
        /* Branch A */
        if (p % 2 == 0) {
            for (int q = 0; q < 10; ++q) {
                /* Shared inner loop in both branches */
                if (q < 5) {
                    for (int r = 0; r < 6; ++r) {
                        int val = base + q * 7 + r;
                        results[(p + q + r) % 1000] = val;
                        checksum |= val;
                    }
                } else {
                    /* Different code in same inner loop */
                    results[(p + q) % 1000] = base - q;
                }
            }
        } 
        /* Branch B - partially overlapping inner structure */
        else {
            for (int q = 0; q < 10; ++q) {
                /* Same inner loop but with different condition */
                if (q % 2 == 0) {
                    for (int r = 0; r < 6; ++r) {
                        int val = base - q * 3 + r;
                        results[(p + q + r + 500) % 1000] = val;
                        checksum &= ~val;
                    }
                } else {
                    /* Different alternative */
                    results[(p + q + 500) % 1000] = base + q * 2;
                }
            }
        }
        
        /* Common outer tail */
        record_value(p + 5000, base * 3);
    }
    
    /* Pattern 5: Loop with early exit creating partial blocks */
    printf("Pattern 5: Early exit patterns\n");
    for (int m = 0; m < 30; ++m) {
        volatile int start_val = rand() % 50;
        
        /* Inner loop with possible early exit */
        for (int n = 0; n < 20; ++n) {
            if (start_val + n > 40) {
                /* Early exit - creates block in inner but not always executed */
                record_value(m * 20 + n, 999);
                break;
            }
            
            /* Normal inner loop body */
            int val = start_val * n;
            record_value(m * 20 + n, val);
            
            /* Nested mini-loop */
            if (val % 7 == 0) {
                for (int k = 0; k < 3; ++k) {
                    results[(m + n + k) % 1000] = val + k;
                }
            }
        }
        
        /* Outer continuation */
        if (m % 5 == 0) {
            record_value(m + 6000, start_val * 10);
        }
    }
    
    /* Final validation to ensure all loops executed */
    printf("Final checksum: %d\n", checksum);
    printf("Counter: %d\n", counter);
    printf("Sample results: %d, %d, %d\n", 
           results[0], results[500], results[999]);
    
    return 0;
}
