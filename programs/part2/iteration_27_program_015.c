/* test_auto_profile.c - Test program for GCC AutoFDO coverage of phi-based branch analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[1024] = {0};

/* Prevent inlining to maintain SSA structure */
__attribute__((noinline, noipa))
void hot_function_phi_loop(int iterations, int start_val) {
    /* Pattern A: Loop-dependent phi node feeding comparison with 0/1 */
    int x_prev = start_val;
    
    for (int i = 0; i < iterations; ++i) {
        /* This creates a phi node: x = (i == 0) ? start_val : x_prev + 1 */
        int x = (i == 0) ? start_val : x_prev + 1;
        
        /* Chain assignments to test the while loop walking back through GIMPLE_ASSIGN */
        /* Pattern C: Chained copies */
        int a = x;
        int b = a;
        int c = b;
        
        /* Comparison against constant 0 */
        if (c == 0) {
            global_array[i % 1024] += 1;
            global_counter++;
        }
        
        /* Another comparison against constant 1 */
        int d = (x % 2 == 0) ? 1 : 0;
        if (d == 1) {
            global_array[(i + 1) % 1024] += 2;
        }
        
        x_prev = x;
    }
}

__attribute__((noinline, noipa))
void hot_function_merge_phi(int flag1, int flag2) {
    /* Pattern B: Merge point phi from conditional assignment */
    int val1 = (flag1 > 0) ? 1 : 0;
    int val2 = (flag2 > 0) ? 1 : 0;
    
    /* This creates a phi node at the merge point */
    int merged_val = (val1 && val2) ? 1 : 0;
    
    /* Chain assignments */
    int tmp1 = merged_val;
    int tmp2 = tmp1;
    
    /* Comparison against constant 1 */
    if (tmp2 == 1) {
        global_counter += 10;
        global_array[0] = 1;
    }
    
    /* Another phi from switch-like logic */
    int switch_val;
    switch (flag1 % 3) {
        case 0: switch_val = 0; break;
        case 1: switch_val = 1; break;
        default: switch_val = 2; break;
    }
    
    /* Comparison with chained copies */
    int s1 = switch_val;
    int s2 = s1;
    if (s2 == 0) {
        global_array[1] = 2;
    }
}

__attribute__((noinline, noipa))
void hot_function_complex_phi(int base, int mod) {
    /* Complex phi network with multiple predecessors */
    int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Multiple phi nodes in nested loops */
        int inner_val = (i == 0) ? base : result;
        
        for (int j = 0; j < 10; ++j) {
            /* Another phi node */
            int loop_val = (j == 0) ? inner_val : (loop_val + 1) % mod;
            
            /* Chain and compare */
            int chained = loop_val;
            for (int k = 0; k < 3; ++k) {
                chained = chained;  /* Identity assignment to create more SSA copies */
            }
            
            if (chained == 0) {
                global_counter++;
            }
            
            if (chained == 1) {
                global_array[(i * 10 + j) % 1024] = chained;
            }
        }
        
        result = inner_val + 1;
    }
}

/* Function with boolean phi nodes */
__attribute__((noinline, noipa))
void hot_function_bool_phi(int a, int b, int c) {
    /* Boolean phi nodes */
    _Bool flag1 = (a > b);
    _Bool flag2 = (b > c);
    
    /* Phi of booleans */
    _Bool combined = flag1 && flag2;
    
    /* Chain assignments with different types */
    char c1 = combined;
    short s1 = c1;
    int i1 = s1;
    
    /* Comparisons that should reduce to == 0 or == 1 */
    if (combined == 1) {  /* Explicit comparison with 1 */
        global_counter += 5;
    }
    
    if (i1 == 0) {  /* Chained comparison with 0 */
        global_array[2] = 3;
    }
}

/* Main driver that makes specific paths hot */
int main(int argc, char *argv[]) {
    int iterations = 1000000;  /* Default - make paths hot */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    /* Seed for some variability but predictable hot paths */
    srand(42);
    
    /* Make phi_loop path very hot */
    for (int i = 0; i < iterations; ++i) {
        hot_function_phi_loop(100, i % 10);
    }
    
    /* Make merge_phi path moderately hot */
    for (int i = 0; i < iterations / 10; ++i) {
        hot_function_merge_phi(rand() % 10, rand() % 10);
    }
    
    /* Make complex_phi path hot */
    for (int i = 0; i < iterations / 5; ++i) {
        hot_function_complex_phi(rand() % 5, 3);
    }
    
    /* Make bool_phi path hot */
    for (int i = 0; i < iterations / 20; ++i) {
        hot_function_bool_phi(rand() % 100, rand() % 100, rand() % 100);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_counter;
    for (int i = 0; i < 1024; ++i) {
        checksum += global_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
