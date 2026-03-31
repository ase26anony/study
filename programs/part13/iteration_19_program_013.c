/* Test program for GCC auto-profile coverage of phi-node-defined conditional branches */
#include <stdio.h>
#include <stdlib.h>

/* Volatile helper to prevent optimization of copy chains */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    return c;
}

/* Test 1: Loop-carried phi with copy chain */
int test_loop_phi(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for flag */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Other incoming value to phi */
        }
    }
    
    /* flag is defined by phi at block entry */
    int flag_copy = copy_chain(flag);  /* Create copy chain */
    
    /* Conditional comparing phi-defined variable against 0 */
    if (flag_copy == 0) {  /* Must compare against 0 or 1 */
        result = 10;
    } else {
        result = 20;
    }
    
    return result;
}

/* Test 2: Multiple return paths creating phi */
int test_multi_return_phi(int x) {
    int condition;
    
    if (x < 0) {
        condition = 1;  /* First incoming edge */
        /* Early return would create phi at merge point */
    } else if (x > 100) {
        condition = 0;  /* Second incoming edge */
    } else {
        condition = 1;  /* Third incoming edge */
    }
    
    /* condition is defined by phi at merge point */
    int cond_copy = condition;
    int cond_copy2 = cond_copy;  /* Two-level copy chain */
    
    /* Compare against 1 */
    if (cond_copy2 == 1) {
        return x * 2;
    } else {
        return x / 2;
    }
}

/* Test 3: Switch statement creating phi */
int test_switch_phi(int mode) {
    int status;
    
    switch (mode % 4) {
        case 0:
            status = 1;  /* First incoming value */
            break;
        case 1:
            status = 0;  /* Second incoming value */
            break;
        case 2:
            status = 1;  /* Third incoming value */
            break;
        default:
            status = 0;  /* Fourth incoming value */
            break;
    }
    
    /* status is defined by phi after switch */
    int s1 = status;
    volatile_sink = s1;
    int s2 = s1;
    
    /* Compare against 0 */
    if (s2 == 0) {
        return mode + 100;
    }
    return mode - 100;
}

/* Test 4: Nested loops with phi */
int test_nested_loop_phi(int n) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_flag = 0;
        
        /* Inner loop creates phi for inner_flag */
        for (int j = 0; j < 10; j++) {
            if ((i + j) % 2 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
        }
        
        /* inner_flag defined by phi */
        int flag_copy = inner_flag;
        
        /* Compare against 1 */
        if (flag_copy == 1) {
            sum += i;
        } else {
            sum -= i;
        }
        
        /* Update outer_flag based on condition */
        if (i % 5 == 0) {
            outer_flag = 1;
        } else {
            outer_flag = 0;
        }
    }
    
    /* outer_flag also defined by phi */
    int outer_copy = outer_flag;
    
    /* Final comparison against 0 */
    if (outer_copy == 0) {
        return sum * 2;
    }
    return sum;
}

/* Test 5: Recursive function creating phi */
int test_recursive_phi(int depth, int current) {
    int result;
    
    if (current >= depth) {
        result = 1;  /* Base case value */
    } else {
        /* Recursive call - creates phi for result */
        int recursive_result = test_recursive_phi(depth, current + 1);
        
        if (recursive_result > 0) {
            result = 0;  /* One incoming value */
        } else {
            result = 1;  /* Other incoming value */
        }
    }
    
    /* result is defined by phi at merge point */
    int r1 = result;
    int r2 = r1;
    int r3 = r2;  /* Three-level copy chain */
    
    /* Compare against 1 */
    if (r3 == 1) {
        return current * 10;
    } else {
        return current * 20;
    }
}

/* Test 6: Boolean variable from phi */
int test_bool_phi(int x, int y) {
    _Bool flag;
    
    if (x > y) {
        flag = 1;  /* true */
    } else if (x < y) {
        flag = 0;  /* false */
    } else {
        flag = 1;  /* true */
    }
    
    /* flag is boolean phi */
    _Bool flag_copy = flag;
    
    /* Boolean context compares against 0/1 implicitly */
    if (flag_copy) {  /* Equivalent to flag_copy != 0 */
        return x - y;
    }
    return y - x;
}

/* Test 7: Complex copy chain with function calls */
static int pass_through(int x) {
    int y = x;
    return y;
}

int test_complex_copy_chain(int val) {
    int decision;
    
    /* Create phi with multiple incoming values */
    if (val % 2 == 0) {
        decision = 1;
    } else {
        decision = 0;
    }
    
    /* Long copy chain through multiple functions/variables */
    int d1 = decision;
    int d2 = pass_through(d1);
    int d3 = d2;
    volatile_sink = d3;
    int d4 = pass_through(d3);
    int d5 = d4;
    
    /* Compare against 0 */
    if (d5 == 0) {
        return val * 3;
    }
    return val * 7;
}

/* Test 8: Hot path with __builtin_expect */
int test_hot_path_phi(int x) {
    int hot_flag;
    
    /* Create phi */
    if (x > 1000) {
        hot_flag = 1;
    } else {
        hot_flag = 0;
    }
    
    int flag_copy = hot_flag;
    
    /* Use __builtin_expect to mark as hot path */
    if (__builtin_expect(flag_copy == 1, 1)) {  /* Likely true */
        return x / 2;
    }
    return x * 2;
}

/* Main driver to create hot execution paths */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Hot loop to make basic blocks "annotated" as hot */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Call all test functions to exercise different patterns */
        total += test_loop_phi(i % 100);
        total += test_multi_return_phi(i);
        total += test_switch_phi(i);
        total += test_nested_loop_phi(i % 50);
        
        if (i % 100 == 0) {  /* Less frequent to avoid stack overflow */
            total += test_recursive_phi(5, 0);
        }
        
        total += test_bool_phi(i, i / 2);
        total += test_complex_copy_chain(i);
        total += test_hot_path_phi(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
