/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates phi-node-defined condition variables with copy chains
 * and constant 0/1 comparisons in hot basic blocks
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helpers to prevent optimization */
static volatile int vol_counter = 0;

/* Function to create copy chains */
static inline int copy_once(int x) { return x; }
static inline int copy_twice(int x) { return copy_once(x); }
static inline int copy_thrice(int x) { return copy_twice(x); }

/* Test 1: Loop-carried phi with copy chain */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;  // This will become a phi node
    
    for (int i = 0; i < iterations; ++i) {
        // Create multiple incoming edges to form a phi
        if (i % 3 == 0) {
            flag = 1;  // First incoming value
        } else if (i % 3 == 1) {
            flag = 0;  // Second incoming value
        } else {
            flag = (i & 1);  // Third incoming value (0 or 1)
        }
        result += i;
    }
    
    // After loop: flag is defined by a phi node
    // Create copy chain
    int a = flag;
    int b = copy_once(a);
    int c = copy_twice(b);
    int d = copy_thrice(c);
    
    // Critical conditional comparing against 0/1
    if (d == 0) {  // cmp_rhs is integer constant 0
        result += 1000;
    } else if (d == 1) {  // cmp_rhs is integer constant 1
        result += 2000;
    }
    
    return result;
}

/* Test 2: Multiple returns creating phi for return value */
int test_phi_from_multiple_returns(int x) {
    int intermediate;
    
    if (x < 0) {
        intermediate = 1;
    } else if (x > 100) {
        intermediate = 0;
    } else {
        intermediate = (x & 1);  // 0 or 1
    }
    
    // Copy chain
    int a = intermediate;
    volatile int b = a;  // volatile prevents optimization
    int c = b;
    
    // Conditional on phi-defined variable
    if (c == 1) {
        return x * 2;
    } else {
        return x / 2;
    }
}

/* Test 3: Switch statement creating phi */
int test_phi_from_switch(int mode) {
    int status = 0;
    
    switch (mode % 4) {
        case 0:
            status = 1;
            break;
        case 1:
            status = 0;
            break;
        case 2:
            status = 1;
            break;
        case 3:
            status = 0;
            break;
    }
    
    // Extended copy chain
    int t1 = status;
    int t2 = t1;
    int t3 = copy_once(t2);
    int t4 = copy_twice(t3);
    
    // Hot conditional - use __builtin_expect to hint hot path
    if (__builtin_expect(t4 == 0, 1)) {
        return mode + 100;
    }
    return mode - 100;
}

/* Test 4: Recursive function with phi */
int test_phi_from_recursion(int depth, int max_depth) {
    if (depth >= max_depth) {
        return 0;
    }
    
    int child_result = test_phi_from_recursion(depth + 1, max_depth);
    
    // Phi forms here from recursive calls
    int flag = (child_result > 0) ? 1 : 0;
    
    // Copy chain
    int a = flag;
    int b = a;
    int c = copy_once(b);
    
    // Conditional
    if (c == 1) {
        return depth * 10;
    } else {
        return depth * 5;
    }
}

/* Test 5: Complex control flow with nested loops */
int test_complex_phi(int n) {
    int control = 0;
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 10; ++j) {
            // Multiple assignments to control
            if ((i + j) % 2 == 0) {
                control = 1;
            } else {
                control = 0;
            }
            sum += i * j;
        }
        
        // Phi node for control after inner loop
        int x = control;
        int y = x;
        int z = copy_once(y);
        
        // Hot conditional in frequently executed block
        if (z == 0) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    
    return sum;
}

/* Test 6: Boolean phi with direct boolean comparison */
bool test_bool_phi(int x) {
    bool flag;
    
    if (x % 2 == 0) {
        flag = true;    // Becomes integer 1
    } else {
        flag = false;   // Becomes integer 0
    }
    
    // Copy chain for bool
    bool a = flag;
    bool b = a;
    bool c = copy_once(b);
    
    // if (bool_var) compares against 0
    if (c) {  // Equivalent to if (c != 0)
        return true;
    }
    return false;
}

/* Test 7: Static variable creating phi across calls */
int test_static_phi(int val) {
    static int last_val = 0;
    int diff = val - last_val;
    last_val = val;
    
    // Phi from static variable updates
    int flag = (diff > 0) ? 1 : 0;
    
    // Multi-step copy chain
    int step1 = flag;
    int step2 = step1;
    int step3 = step2;
    int step4 = copy_once(step3);
    int step5 = copy_twice(step4);
    
    // Conditional with both 0 and 1 comparisons
    if (step5 == 0) {
        return val * 3;
    } else if (step5 == 1) {
        return val * 7;
    }
    return val;
}

/* Main driver - creates hot execution paths */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    // Hot loop to make basic blocks "annotated"
    for (int i = 0; i < ITERATIONS; ++i) {
        // Mix different test functions to create varied control flow
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i);
        total += test_phi_from_switch(i);
        
        if (i % 1000 == 0) {
            total += test_phi_from_recursion(0, 5);
        }
        
        total += test_complex_phi(i % 10 + 1);
        total += test_bool_phi(i) ? 1 : 0;
        total += test_static_phi(i);
        
        // Prevent optimization
        vol_counter = i;
    }
    
    printf("Result: %d\n", total);
    printf("Volatile counter: %d\n", vol_counter);
    
    return total > 0 ? 0 : 1;
}
