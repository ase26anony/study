/* test_auto_profile.c - Test program for GCC AutoFDO coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1024] = {0};

/* Prevent inlining to maintain SSA structure */
__attribute__((noinline, noipa))
void hot_function_pattern_a(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi node feeding comparison with 0/1 */
    int x_prev = start_val;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain of assignments to test the while loop walking back through GIMPLE_ASSIGN */
        int a = x;      /* GIMPLE_ASSIGN 1 */
        int b = a;      /* GIMPLE_ASSIGN 2 */
        int c = b;      /* GIMPLE_ASSIGN 3 */
        
        /* Comparison against constant 0 - this should trigger the uncovered code */
        if (c == 0) {
            /* Hot path - executed many times */
            global_array[i % 1024] += 1;
            global_counter++;
        } else {
            /* Cold path - rarely executed */
            global_array[i % 1024] -= 1;
        }
        
        x_prev = x;
    }
}

__attribute__((noinline, noipa))
void hot_function_pattern_b(int iterations, int seed) {
    /* Pattern B: Merge point phi from conditional assignment */
    volatile int cond_source = seed; /* volatile to prevent constant folding */
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node at the merge point */
        int val = (cond_source & 1) ? 1 : 0;
        
        /* Multiple assignments to obscure origin */
        int tmp1 = val;
        int tmp2 = tmp1;
        
        /* Comparison against constant 1 */
        if (tmp2 == 1) {
            /* Hot path */
            global_counter += 2;
            global_array[(i * 7) % 1024] ^= tmp2;
        } else {
            /* Cold path */
            global_counter -= 1;
        }
        
        /* Change condition source to create varying behavior */
        cond_source = (cond_source * 1103515245 + 12345) & 0x7fffffff;
    }
}

__attribute__((noinline, noipa))
void hot_function_pattern_c(int iterations) {
    /* Pattern C: Complex phi network with boolean type */
    bool flag1 = true;
    bool flag2 = false;
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi nodes for boolean values */
        bool current_flag = (i % 3 == 0) ? flag1 : flag2;
        
        /* Boolean comparisons implicitly use 0/1 */
        if (current_flag == true) {  /* Should compile to comparison with 1 */
            global_counter += 3;
            global_array[i % 1024] |= 0x01;
        }
        
        if (current_flag == false) { /* Should compile to comparison with 0 */
            global_counter -= 1;
            global_array[i % 1024] &= ~0x01;
        }
        
        /* Toggle flags */
        flag1 = !flag1;
        if (i % 5 == 0) {
            flag2 = !flag2;
        }
    }
}

__attribute__((noinline, noipa))
void mixed_patterns(int iterations) {
    /* Mix all patterns in one function to increase coverage chances */
    int pattern_selector = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Pattern selector creates a phi node */
        int selector = (pattern_selector++) % 3;
        
        /* Switch creates multiple phi nodes at merge points */
        switch (selector) {
            case 0: {
                /* Nested phi: selector -> temp -> comparison */
                int temp = selector;
                for (int j = 0; j < 3; j++) {
                    temp = (j == 0) ? temp : temp + 1;
                }
                if (temp == 0) {
                    global_counter++;
                }
                break;
            }
            case 1: {
                /* Chain of assignments with char type */
                char c1 = (char)selector;
                char c2 = c1;
                char c3 = c2;
                if (c3 == 1) {
                    global_counter += 2;
                }
                break;
            }
            case 2: {
                /* Short type with phi */
                short s1 = (short)selector;
                short s2 = s1;
                if (s2 == 0) {
                    global_counter += 3;
                }
                break;
            }
        }
        
        /* Prevent loop unrolling from eliminating phi nodes */
        volatile int barrier = i;
        (void)barrier;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot paths */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Execute all patterns to generate diverse profile data */
    hot_function_pattern_a(iterations / 4, 0);
    hot_function_pattern_b(iterations / 4, 42);
    hot_function_pattern_c(iterations / 4);
    mixed_patterns(iterations / 4);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
