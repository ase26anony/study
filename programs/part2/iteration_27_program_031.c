/* test_auto_profile.c - Test program for GCC AutoFDO coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
int phi_defined_variable(int iteration, int prev_val) {
    /* Pattern B: Merge point phi */
    int phi_val;
    if (iteration % 3 == 0) {
        phi_val = 1;  /* Will be compared against 1 later */
    } else {
        phi_val = 0;  /* Will be compared against 0 later */
    }
    
    /* Pattern C: Chained copies to test the while loop */
    int copy1 = phi_val;
    int copy2 = copy1;
    int final_val = copy2;
    
    /* Critical comparison against constant 0 or 1 */
    if (final_val == 1) {  /* RHS is constant 1 */
        global_array[iteration % 256] += 1;
        return 1;
    } else if (final_val == 0) {  /* RHS is constant 0 */
        global_counter++;
        return 0;
    }
    
    return -1;
}

__attribute__((noinline, noipa))
int loop_dependent_phi(int n) {
    /* Pattern A: Loop-dependent phi node */
    int result = 0;
    int prev_x = 0;
    
    for (int i = 0; i < n; i++) {
        /* x is defined by a phi node at loop header */
        int x;
        if (i == 0) {
            x = 0;
        } else {
            x = prev_x + (rand() % 3);  /* Use rand to prevent constant folding */
        }
        
        /* Chain assignments to test the while loop */
        int temp1 = x;
        int temp2 = temp1;
        int final_x = temp2;
        
        /* Compare phi-derived value against constant 0 */
        if (final_x == 0) {
            result += 1;
        }
        
        prev_x = x;
    }
    
    return result;
}

__attribute__((noinline, noipa))
bool boolean_phi_test(int a, int b) {
    /* Pattern using boolean phi */
    bool condition;
    
    if (a > b) {
        condition = true;  /* true becomes 1 in comparisons */
    } else {
        condition = false; /* false becomes 0 in comparisons */
    }
    
    /* Chain through multiple assignments */
    bool b1 = condition;
    bool b2 = b1;
    bool final_bool = b2;
    
    /* Compare against boolean constant (which becomes 0/1) */
    if (final_bool == true) {  /* Will be == 1 */
        global_counter += 2;
        return true;
    }
    
    return false;
}

__attribute__((noinline, noipa))
void hot_function(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix different patterns to ensure coverage */
        sum += phi_defined_variable(i, sum);
        
        if (i % 100 == 0) {
            sum += loop_dependent_phi(10);
        }
        
        if (i % 50 == 0) {
            boolean_phi_test(i, i/2);
        }
        
        /* Additional pattern: phi from switch statement */
        int switch_val;
        switch (i % 4) {
            case 0: switch_val = 0; break;
            case 1: switch_val = 1; break;
            case 2: switch_val = 0; break;
            default: switch_val = 1; break;
        }
        
        /* Chain assignments */
        int s1 = switch_val;
        int s2 = s1;
        
        /* Compare against 1 */
        if (s2 == 1) {
            global_array[(i + 1) % 256] ^= i;
        }
    }
    
    /* Use result to prevent dead code elimination */
    global_array[0] = sum % 256;
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default large number for hot path */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000000;
        }
    }
    
    printf("Running AutoFDO test with %d iterations\n", iterations);
    
    /* Seed random for variability but reproducible behavior */
    srand(42);
    
    /* Clear global state */
    memset(global_array, 0, sizeof(global_array));
    global_counter = 0;
    
    /* Execute hot function many times to create profile */
    hot_function(iterations);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum = (checksum + global_array[i]) % 1000;
    }
    checksum = (checksum + global_counter) % 1000;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
