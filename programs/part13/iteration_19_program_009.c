/* Test program for GCC auto-profile.cc lines 1312-1333
 * Creates phi-node-defined condition variables with copy chains
 * and constant 0/1 comparisons in hot basic blocks
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_once(int x) { return x; }
static inline int copy_twice(int x) { return copy_once(x); }
static inline int copy_thrice(int x) { return copy_twice(x); }

/* Test 1: Phi node created after loop with copy chain */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* Copy chain between phi and comparison */
    int a = flag;           /* First copy */
    int b = copy_once(a);   /* Second copy via function */
    int c = copy_twice(b);  /* Third copy */
    int d = copy_thrice(c); /* Fourth copy */
    
    /* Conditional comparing phi-defined variable against 0 */
    if (d == 0) {  /* cmp_rhs is integer constant 0 */
        result += 1000;
    } else {
        result += 2000;
    }
    
    volatile_sink = result; /* Prevent optimization */
    return result;
}

/* Test 2: Phi from multiple return paths with copy chain */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Multiple returns create phi for 'condition' */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Same value from different path */
    }
    
    /* Create longer copy chain */
    int t1 = condition;
    int t2 = t1;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    
    /* Compare against 1 */
    if (t5 == 1) {  /* cmp_rhs is integer constant 1 */
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Test 3: Phi from switch statement with copy propagation */
int test_phi_from_switch(int mode) {
    int status;
    
    /* Switch creates phi for 'status' */
    switch (mode % 4) {
        case 0:
            status = 1;
            break;
        case 1:
            status = 0;
            break;
        case 2:
            status = 1;  /* Same as case 0 */
            break;
        default:
            status = 0;  /* Same as case 1 */
            break;
    }
    
    /* Propagate through assignments */
    int tmp = status;
    for (int i = 0; i < 3; i++) {
        tmp = copy_once(tmp);  /* Multiple copies in loop */
    }
    
    /* Boolean context comparison (implicit == 1) */
    if (tmp) {  /* Equivalent to tmp == 1 */
        return mode + 100;
    } else {
        return mode - 100;
    }
}

/* Test 4: Recursive function creating phi for depth */
int test_recursive_phi(int n, int depth) {
    if (n <= 0) {
        return depth;
    }
    
    int next_depth;
    if (depth % 2 == 0) {
        next_depth = 1;  /* One incoming value */
    } else {
        next_depth = 0;  /* Another incoming value */
    }
    
    /* Copy chain */
    int d1 = next_depth;
    int d2 = d1;
    int d3 = d2;
    
    /* Compare against 0 */
    if (d3 == 0) {
        return test_recursive_phi(n - 1, depth + 1) * 2;
    } else {
        return test_recursive_phi(n - 1, depth + 1) / 2;
    }
}

/* Test 5: Complex phi with nested control flow */
int test_complex_phi_with_hot_path(int seed) {
    int counter = 0;
    int flag = 0;
    
    /* Hot loop to make basic block annotated */
    for (int i = 0; i < 10000; i++) {
        /* Nested if-else creates phi for 'flag' */
        if (i % 7 == 0) {
            if (seed > 0) {
                flag = 1;
            } else {
                flag = 0;
            }
        } else if (i % 3 == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        counter += i;
        
        /* Immediate use in hot path with copy chain */
        int f1 = flag;
        int f2 = f1;
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect(f2 == 1, 1)) {  /* Hot comparison against 1 */
            counter += 10;
        } else {
            counter += 1;
        }
    }
    
    return counter;
}

/* Test 6: Global variable creating phi through updates */
static int global_flag = 0;

int test_global_phi(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Update global in different branches */
        if (i % 5 == 0) {
            global_flag = 1;
        } else {
            global_flag = 0;
        }
        
        /* Copy global to local with chain */
        int local = global_flag;
        int chain1 = local;
        int chain2 = chain1;
        int chain3 = chain2;
        
        /* Compare against 0 */
        if (chain3 == 0) {
            sum += i;
        } else {
            sum += i * 2;
        }
    }
    
    return sum;
}

/* Main driver with hot execution */
int main() {
    int total = 0;
    
    /* Execute many times to create hot paths */
    for (int iteration = 0; iteration < 100000; iteration++) {
        /* Mix different test patterns */
        total += test_phi_after_loop(iteration % 100 + 1);
        total += test_phi_from_multiple_returns(iteration, iteration / 2);
        total += test_phi_from_switch(iteration);
        
        if (iteration % 1000 == 0) {
            total += test_recursive_phi(5, 0);
        }
        
        total += test_complex_phi_with_hot_path(iteration);
        total += test_global_phi(10);
        
        /* Prevent compiler from optimizing away loops */
        if (total > 1000000000) {
            total = total % 1000000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
