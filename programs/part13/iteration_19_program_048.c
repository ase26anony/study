/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates phi-node-defined condition variables with copy chains
 * and constant 0/1 comparisons in hot basic blocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

volatile int global_counter = 0;
volatile int dummy_volatile = 0;

/* Helper to create copy chains */
static inline int copy_once(int x) {
    volatile int tmp = x;  /* Prevent optimization */
    return tmp;
}

static inline int copy_twice(int x) {
    int a = copy_once(x);
    int b = copy_once(a);
    return b;
}

/* Pattern 1: Loop-carried phi with copy chain */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for flag at entry */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* First incoming edge to phi */
        } else {
            flag = 0;  /* Second incoming edge to phi */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined flag */
    int a = flag;           /* First copy */
    int b = copy_once(a);   /* Second copy via function */
    int c = copy_twice(b);  /* Third copy via chain */
    
    /* Conditional comparing phi-derived value against 0 */
    if (c == 0) {  /* This should trigger the uncovered code */
        result += 1000;
    } else {
        result += 2000;
    }
    
    return result;
}

/* Pattern 2: Multiple return paths creating phi */
int test_phi_from_multiple_returns(int x) {
    int condition;
    
    if (x < 0) {
        condition = 1;  /* First incoming value to phi */
        /* Early return would create phi at merge point */
    } else if (x > 100) {
        condition = 0;  /* Second incoming value to phi */
    } else {
        condition = 1;  /* Third incoming value to phi */
    }
    
    /* Merge point creates phi for condition */
    volatile int v = condition;  /* Prevent optimization */
    
    /* Copy chain */
    int a = v;
    int b = a;
    int c = copy_once(b);
    
    /* Compare against 1 */
    if (c == 1) {  /* Should trigger uncovered code */
        return x * 2;
    }
    return x / 2;
}

/* Pattern 3: Switch statement creating phi */
int test_phi_from_switch(int mode) {
    int flag;
    
    switch (mode % 4) {
        case 0:
            flag = 1;
            break;
        case 1:
            flag = 0;
            break;
        case 2:
            flag = 1;
            break;
        case 3:
            flag = 0;
            break;
        default:
            flag = 1;
    }
    
    /* Create longer copy chain */
    int t1 = flag;
    int t2 = t1;
    int t3 = copy_once(t2);
    int t4 = copy_twice(t3);
    int t5 = t4;
    
    /* Multiple comparisons in hot loop */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        /* Compare against both 0 and 1 */
        if (t5 == 0) {
            sum += i;
        }
        if (t5 == 1) {
            sum += i * 2;
        }
    }
    
    return sum;
}

/* Pattern 4: Boolean phi with implicit 0/1 comparison */
bool test_boolean_phi(int x, int y) {
    bool condition;
    
    /* Different paths set the boolean */
    if (x > y) {
        condition = true;   /* true becomes 1 */
    } else {
        condition = false;  /* false becomes 0 */
    }
    
    /* Copy chain for boolean */
    bool a = condition;
    bool b = a;
    bool c = copy_once(b);
    
    /* Implicit comparison against 0 (if (c)) */
    if (c) {  /* Should be converted to c == 1 or c != 0 */
        return true;
    }
    return false;
}

/* Pattern 5: Recursive phi creation */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int next_val;
    if (n % 2 == 0) {
        next_val = 1;
    } else {
        next_val = 0;
    }
    
    /* Recursive call creates phi for return value */
    int child_result = test_recursive_phi(next_val, depth - 1);
    
    /* Copy chain on recursive result */
    int a = child_result;
    int b = copy_once(a);
    
    /* Compare against 1 */
    if (b == 1) {
        return depth * 2;
    }
    return depth;
}

/* Pattern 6: Complex nested control flow */
int test_nested_control_flow(int x) {
    int flag = 0;
    
    for (int i = 0; i < 10; ++i) {
        if (i < x) {
            for (int j = 0; j < 5; ++j) {
                if (j % 2 == 0) {
                    flag = 1;
                } else {
                    flag = 0;
                }
            }
        } else {
            flag = 0;
        }
    }
    
    /* Extended copy chain */
    int v1 = flag;
    int v2 = v1;
    int v3 = copy_once(v2);
    int v4 = copy_twice(v3);
    int v5 = v4;
    int v6 = copy_once(v5);
    
    /* Hot comparison in frequently executed block */
    int result = 0;
    for (int k = 0; k < 1000; ++k) {
        if (v6 == 0) {
            result += k;
        } else if (v6 == 1) {
            result += k * 2;
        }
    }
    
    return result;
}

/* Main driver to create hot execution paths */
int main() {
    int total = 0;
    
    /* Execute many times to create hot paths */
    for (int iteration = 0; iteration < 100000; ++iteration) {
        /* Mix different patterns to exercise various phi formations */
        total += test_phi_after_loop(iteration % 50);
        total += test_phi_from_multiple_returns(iteration);
        total += test_phi_from_switch(iteration);
        total += test_boolean_phi(iteration, iteration / 2) ? 1 : 0;
        total += test_recursive_phi(iteration % 10, 3);
        total += test_nested_control_flow(iteration % 20);
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect((iteration % 1000) == 0, 0)) {
            dummy_volatile++;  /* Cold path */
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
