#include <stdio.h>
#include <stdlib.h>

// Helper function to prevent optimization
__attribute__((noinline)) 
int use_value(int val) {
    volatile int sink = val;
    return sink;
}

// Function with complex control flow to generate PHI nodes
__attribute__((noinline))
int process_with_phi(int iterations, volatile int* inputs) {
    int phi_var = 0;  // Will require PHI node at loop header
    
    // Chain of assignments creating SSA_NAME chain
    int chain_start = inputs[0];
    int a = chain_start;
    int b = a;
    int c = b;
    int d = c;
    
    // Main hot loop - will be profiled
    for (int i = 0; i < iterations; ++i) {
        // Create PHI node scenario: phi_var gets different values
        if (inputs[1] != 0) {  // This creates a merge point
            phi_var = 1;
        } else {
            phi_var = 2;
        }
        
        // Use the chained variable in comparison with constant 0
        if (d == 0) {  // GIMPLE_COND with RHS = 0
            // True branch
            inputs[0] += 1;
        } else {
            // False branch
            inputs[0] -= 1;
        }
        
        // Another comparison with constant 1
        if (phi_var != 1) {  // GIMPLE_COND with RHS = 1
            inputs[2] *= 2;
        } else {
            inputs[2] /= 2;
        }
        
        // Re-establish assignment chain
        a = inputs[i % 3];
        b = a;
        c = b;
        d = c;
    }
    
    // Use phi_var after loop - creates PHI node at loop exit
    if (phi_var == 1) {  // Another comparison with constant 1
        return 1;
    }
    return 0;
}

// Second function with different patterns
__attribute__((noinline))
int another_hot_path(int iterations, volatile int* vars) {
    int result = 0;
    
    // Complex assignment chain
    int x1 = vars[0];
    int x2 = x1;
    int x3 = x2;
    int x4 = x3;
    
    for (int i = 0; i < iterations; ++i) {
        // Multiple comparisons with constants 0 and 1
        if (x4 == 1) {  // Comparison with constant 1
            result += i;
        } else if (x4 == 0) {  // Comparison with constant 0
            result -= i;
        }
        
        // Create PHI node by assigning different values
        int local_phi;
        if (vars[1] > 0) {
            local_phi = 1;
        } else {
            local_phi = 0;
        }
        
        // Use local_phi in comparison
        if (local_phi != 0) {  // Comparison with constant 0
            vars[2] += local_phi;
        }
        
        // Update assignment chain
        x1 = vars[i % 3];
        x2 = x1;
        x3 = x2;
        x4 = x3;
    }
    
    return result;
}

// Third function with nested loops for more complex CFG
__attribute__((noinline))
int nested_loop_pattern(int outer_iter, int inner_iter, volatile int* data) {
    int sum = 0;
    int phi_val = 0;
    
    for (int o = 0; o < outer_iter; ++o) {
        // Assignment chain
        int t1 = data[o % 4];
        int t2 = t1;
        int t3 = t2;
        
        for (int i = 0; i < inner_iter; ++i) {
            // PHI node creation in inner loop
            if (data[1] != 0) {
                phi_val = 1;
            } else {
                phi_val = 0;
            }
            
            // Multiple comparisons with constants
            if (t3 == 0) {  // RHS = 0
                sum += phi_val;
            }
            if (phi_val != 1) {  // RHS = 1
                sum -= i;
            }
            
            // Update chain
            t1 = data[(o + i) % 4];
            t2 = t1;
            t3 = t2;
        }
        
        // Comparison after inner loop
        if (phi_val == 1) {  // RHS = 1
            data[3] += sum;
        }
    }
    
    return sum;
}

int main(int argc, char** argv) {
    int iterations = 1000000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    // Volatile variables to prevent constant propagation
    volatile int vars[4];
    vars[0] = 0;
    vars[1] = 1;
    vars[2] = 0;
    vars[3] = 1;
    
    // Call functions to create hot code regions
    int result1 = process_with_phi(iterations, vars);
    int result2 = another_hot_path(iterations / 2, vars);
    int result3 = nested_loop_pattern(100, iterations / 100, vars);
    
    // Calculate checksum to prevent dead code elimination
    int checksum = result1 + result2 + result3;
    for (int i = 0; i < 4; ++i) {
        checksum += vars[i];
    }
    
    // Use the result
    printf("Result: %d\n", checksum);
    
    return 0;
}
