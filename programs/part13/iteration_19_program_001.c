/* Test program for GCC auto-profile.cc uncovered lines (1312-1333)
 * Creates phi-node-defined condition variables with copy chains
 * and constant comparisons (0/1) in hot basic blocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    return c;
}

/* Function with phi from loop-carried dependency */
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;  /* This will become a phi node */
    
    /* Loop creates phi for 'flag' at loop header */
    for (int i = 0; i < n; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int flag_copy = copy_chain(flag);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (flag_copy == 1) {  /* Comparison against constant 1 */
        result += 1000;
    } else if (flag_copy == 0) {  /* Comparison against constant 0 */
        result += 500;
    }
    
    return result;
}

/* Function with phi from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Different return paths create phi for return value */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Same value from different path */
    }
    
    /* Multiple copy assignments */
    int a = condition;
    int b = a;
    volatile_sink = b;
    int c = b;
    
    /* Test in hot loop to make block annotated */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        if (c == 1) {  /* Comparison against constant 1 */
            sum += x;
        } else {
            sum += y;
        }
    }
    
    return sum;
}

/* Function with phi from switch statement */
int test_phi_from_switch(int mode) {
    int status = 0;  /* Will become phi node */
    
    switch (mode % 4) {
        case 0:
            status = 1;
            break;
        case 1:
            status = 0;
            break;
        case 2:
            status = 1;  /* Same value from different case */
            break;
        default:
            status = 0;
            break;
    }
    
    /* Extended copy chain */
    int tmp1 = status;
    volatile_sink = tmp1;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    volatile_sink = tmp3;
    int final = tmp3;
    
    /* Conditional with likely hot path */
    if (__builtin_expect(final == 1, 1)) {  /* Hot path comparison */
        return 100;
    } else {
        return 200;
    }
}

/* Function with recursive phi creation */
int test_recursive_phi(int depth, int max_depth) {
    if (depth >= max_depth) {
        return 0;
    }
    
    int child_result = test_recursive_phi(depth + 1, max_depth);
    
    /* Phi created from recursive calls */
    int should_process = (child_result % 2 == 0) ? 1 : 0;
    
    /* Copy chain */
    int a = should_process;
    int b = a;
    volatile_sink = b;
    
    /* Conditional in potentially hot path (if called many times) */
    if (b == 1) {
        return depth * 10 + 1;
    } else {
        return depth * 10;
    }
}

/* Function with global variable creating phi */
static int global_counter = 0;

int test_phi_with_global(int iterations) {
    int local_flag;
    
    /* Loop with global creates phi for local_flag */
    for (int i = 0; i < iterations; ++i) {
        if (global_counter++ % 2 == 0) {
            local_flag = 1;
        } else {
            local_flag = 0;
        }
        
        /* Multiple assignments to create copy chain */
        int x = local_flag;
        int y = x;
        volatile_sink = y;
        
        /* Comparison against 0/1 in hot loop */
        if (y == 1) {
            global_counter += 2;
        } else if (y == 0) {
            global_counter += 1;
        }
    }
    
    return global_counter;
}

/* Complex function with nested control flow */
int test_complex_phi_pattern(int n) {
    int state = 0;
    int total = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Inner loop creates additional phi nodes */
        int inner_flag = 0;
        for (int j = 0; j < 10; ++j) {
            if ((i + j) % 3 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            /* Copy and use */
            int copy1 = inner_flag;
            int copy2 = copy1;
            
            if (copy2 == 1) {
                total += i + j;
            }
        }
        
        /* Outer loop phi */
        if (i % 2 == 0) {
            state = 1;
        } else {
            state = 0;
        }
        
        int state_copy = state;
        volatile_sink = state_copy;
        
        if (state_copy == 0) {
            total -= i;
        }
    }
    
    return total;
}

/* Main driver to create hot execution paths */
int main(void) {
    int total_result = 0;
    
    /* Execute many times to create hot paths */
    for (int iteration = 0; iteration < 100000; ++iteration) {
        /* Mix different patterns */
        total_result += test_phi_after_loop(iteration % 100);
        total_result += test_phi_from_multiple_returns(iteration, iteration / 2);
        total_result += test_phi_from_switch(iteration);
        total_result += test_recursive_phi(0, 5);
        total_result += test_phi_with_global(10);
        total_result += test_complex_phi_pattern(20);
        
        /* Prevent compiler from optimizing away loops */
        if (iteration % 10000 == 0) {
            printf("Progress: %d iterations, result so far: %d\n", 
                   iteration, total_result);
        }
    }
    
    printf("Final result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
