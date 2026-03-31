/* test_auto_profile.c - Test program for GCC auto-profile coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization of copy chains */
static volatile int vol_sink;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    int b = a;
    vol_sink = b;  /* Prevent optimization */
    int c = b;
    return c;
}

/* Test 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;  /* Will become phi node at loop header */
    
    for (int i = 0; i < n; i++) {
        /* Create phi for 'flag' at loop header */
        if (i % 3 == 0) {
            flag = 1;
        } else if (i % 3 == 1) {
            flag = 0;
        }
        /* else keep current flag value (phi) */
        
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int flag_copy = copy_chain(flag);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (flag_copy == 0) {  /* Compare against 0 */
        result += 100;
    } else if (flag_copy == 1) {  /* Compare against 1 */
        result += 200;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    if (x > 100) {
        condition = 1;
    } else if (x < 0) {
        condition = 0;
    } else {
        condition = (y % 2);
    }
    
    /* Create copy chain */
    int cond_copy1 = condition;
    int cond_copy2 = cond_copy1;
    vol_sink = cond_copy2;
    int final_cond = cond_copy2;
    
    /* Compare against 1 */
    if (final_cond == 1) {
        return x * 2;
    }
    
    /* Compare against 0 */
    if (final_cond == 0) {
        return y * 3;
    }
    
    return x + y;
}

/* Test 3: Phi node from switch statement */
int test_phi_from_switch(int mode) {
    int status;
    
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
        default:
            status = 0;
            break;
    }
    
    /* Longer copy chain */
    int s1 = status;
    int s2 = s1;
    int s3 = s2;
    vol_sink = s3;
    int s4 = s3;
    int s5 = s4;
    
    /* Hot path - use __builtin_expect to hint */
    if (__builtin_expect(s5 == 1, 1)) {
        return 1000;
    }
    
    return 500;
}

/* Test 4: Boolean phi with direct boolean comparison */
bool test_boolean_phi(int a, int b) {
    bool flag;
    
    if (a > b) {
        flag = true;
    } else {
        flag = false;
    }
    
    /* Boolean creates 0/1 comparison */
    bool flag_copy = flag;
    vol_sink = flag_copy;
    
    /* if (bool_var) compares against 0 */
    if (flag_copy) {
        return true;
    }
    
    return false;
}

/* Test 5: Recursive function creating phi for condition */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int next_val;
    if (n % 2 == 0) {
        next_val = test_recursive_phi(n / 2, depth - 1);
    } else {
        next_val = test_recursive_phi(n * 3 + 1, depth - 1);
    }
    
    /* Phi created from recursive calls */
    int phi_val = (next_val > 100) ? 1 : 0;
    
    /* Copy chain */
    int v1 = phi_val;
    int v2 = v1;
    vol_sink = v2;
    
    /* Compare against 0 */
    if (v2 == 0) {
        return next_val + 1;
    }
    
    return next_val - 1;
}

/* Test 6: Complex phi with nested control flow */
int test_complex_phi_with_hot_path(int iterations) {
    int hot_counter = 0;
    int flag = 0;
    
    /* Hot loop to make basic block annotated */
    for (int i = 0; i < iterations; i++) {
        /* Create phi for 'flag' with multiple predecessors */
        if (i % 10 == 0) {
            flag = 1;
        } else if (i % 7 == 0) {
            flag = 0;
        }
        
        /* Use the flag in hot path */
        int f1 = flag;
        int f2 = f1;
        
        /* Compare against 1 in hot path */
        if (f2 == 1) {
            hot_counter += 2;
        } else {
            hot_counter += 1;
        }
    }
    
    /* Final comparison with copy chain */
    int final_flag = copy_chain(flag);
    
    /* Compare against 0 */
    if (final_flag == 0) {
        hot_counter *= 2;
    }
    
    return hot_counter;
}

/* Main function to execute all tests repeatedly */
int main(void) {
    int total_result = 0;
    const int hot_iterations = 100000;
    
    printf("Starting auto-profile coverage test...\n");
    
    /* Execute tests many times to create hot paths */
    for (int i = 0; i < hot_iterations; i++) {
        /* Test 1 - creates phi from loop */
        total_result += test_phi_after_loop(i % 100 + 1);
        
        /* Test 2 - phi from multiple returns */
        total_result += test_phi_from_multiple_returns(i, i * 2);
        
        /* Test 3 - phi from switch */
        total_result += test_phi_from_switch(i);
        
        /* Test 4 - boolean phi */
        total_result += test_boolean_phi(i, i / 2);
        
        /* Test 5 - recursive phi */
        if (i % 100 == 0) {  /* Less frequent to avoid stack overflow */
            total_result += test_recursive_phi(i, 5);
        }
        
        /* Test 6 - complex phi with explicit hot path */
        total_result += test_complex_phi_with_hot_path(10);
        
        /* Prevent compiler from optimizing away loops */
        if (total_result > 1000000000) {
            total_result = total_result % 1000000;
        }
    }
    
    printf("Final result: %d\n", total_result);
    printf("Test completed. Check coverage with:\n");
    printf("  gcc -O2 -fprofile-estimate -fauto-profile test_auto_profile.c\n");
    printf("  gcc -O2 -fprofile-estimate -fdump-tree-phiopt test_auto_profile.c\n");
    
    return total_result == 0 ? 0 : 1;
}
