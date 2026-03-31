/* test_auto_profile.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;
volatile int global_array[256] = {0};

/* Prevent inlining to preserve SSA structure */
__attribute__((noinline, noipa))
void hot_function(int iterations, int start_val) {
    int phi_val = start_val;
    int prev_val = 0;
    
    /* Pattern A: Loop-dependent phi node */
    for (int i = 0; i < iterations; i++) {
        /* This creates a phi node: phi_val = (i == 0) ? start_val : phi_val + 1 */
        if (i == 0) {
            phi_val = start_val;
        } else {
            phi_val = prev_val + 1;
        }
        
        /* Pattern C: Chain of assignments to obscure origin */
        int temp1 = phi_val;
        int temp2 = temp1;
        int temp3 = temp2;
        
        /* Comparison against constant 0 */
        if (temp3 == 0) {
            global_counter += 1;
            global_array[i & 255] = i;
        }
        
        /* Another comparison against constant 1 */
        if (temp3 == 1) {
            global_counter -= 1;
            global_array[(i + 1) & 255] = i * 2;
        }
        
        prev_val = phi_val;
    }
}

/* Another function with different pattern */
__attribute__((noinline, noipa))
void merge_point_function(int a, int b, int iterations) {
    int merge_val;
    
    for (int i = 0; i < iterations; i++) {
        /* Pattern B: Merge point phi */
        if ((i & 1) == 0) {
            merge_val = a;
        } else {
            merge_val = b;
        }
        
        /* Chain assignments */
        int x = merge_val;
        int y = x;
        
        /* Comparison against 1 */
        if (y == 1) {
            global_array[i & 255] += 1;
        }
        
        /* Comparison against 0 */
        if (y == 0) {
            global_array[i & 255] -= 1;
        }
    }
}

/* Function with boolean phi */
__attribute__((noinline, noipa))
void boolean_phi_function(int iterations) {
    bool flag = false;
    
    for (int i = 0; i < iterations; i++) {
        /* Boolean phi node */
        flag = !flag;
        
        /* Boolean comparison (compiles to == 0 or == 1) */
        if (flag == true) {  /* Will be == 1 */
            global_counter += i;
        }
        
        if (!flag) {  /* Will be == 0 */
            global_counter -= i;
        }
    }
}

/* Complex pattern with nested loops */
__attribute__((noinline, noipa))
void nested_loop_function(int outer_iter, int inner_iter) {
    int outer_phi = 0;
    
    for (int i = 0; i < outer_iter; i++) {
        /* Outer loop phi */
        outer_phi = (i == 0) ? 1 : outer_phi * 2;
        
        int inner_phi = outer_phi;
        
        for (int j = 0; j < inner_iter; j++) {
            /* Inner loop phi with dependency */
            inner_phi = (j == 0) ? outer_phi : inner_phi - 1;
            
            /* Multiple assignments chain */
            int a = inner_phi;
            int b = a;
            int c = b;
            
            /* Hot comparison against 0 */
            if (c == 0) {
                global_array[(i + j) & 255] += 1;
            }
            
            /* Comparison against 1 */
            if (c == 1) {
                global_array[(i + j) & 255] *= 2;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    /* Call all patterns to ensure coverage */
    hot_function(iterations, 0);  /* Start with 0 to trigger == 0 branch */
    hot_function(iterations / 2, 1);  /* Start with 1 to trigger == 1 branch */
    
    merge_point_function(0, 1, iterations);
    boolean_phi_function(iterations);
    nested_loop_function(100, iterations / 100);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
