/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates phi-node-defined condition variables with copy chains
 * and hot path annotations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile helpers to prevent optimization */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy1(int x) { return x; }
static inline int copy2(int x) { return copy1(x); }
static inline int copy3(int x) { return copy2(x); }

/* Test 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for 'flag' at entry */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* Hot path - executed 1/3 of iterations */
        } else {
            flag = 0;  /* Cold path */
        }
        volatile_sink = i;  /* Prevent optimization */
    }
    
    /* Create copy chain from phi-defined 'flag' */
    int a = flag;
    int b = copy1(a);
    int c = copy2(b);
    int d = copy3(c);
    
    /* Critical conditional comparing against 0/1 */
    if (d == 1) {  /* Compare against 1 */
        result = 100;
    } else {
        result = 200;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x) {
    int condition;
    
    /* Multiple control flow paths create phi for 'condition' */
    if (x > 1000) {
        condition = 1;
    } else if (x > 500) {
        condition = 0;
    } else if (x > 100) {
        condition = 1;
    } else {
        condition = 0;
    }
    
    /* Extended copy chain */
    int t1 = condition;
    int t2 = t1;
    int t3 = t2;
    int t4 = copy1(t3);
    int t5 = copy2(t4);
    
    /* Conditional with 0 comparison */
    if (t5 == 0) {
        return x * 2;
    } else {
        return x / 2;
    }
}

/* Test 3: Phi node from switch statement */
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
            status = 1;
            break;
        case 3:
            status = 0;
            break;
        default:
            status = 1;
    }
    
    /* Complex copy chain with volatile to prevent optimization */
    volatile int v1 = status;
    int s1 = v1;
    int s2 = copy1(s1);
    int s3 = copy2(s2);
    volatile_sink = s3;
    int s4 = s3;
    
    /* Conditional with != 1 comparison */
    if (s4 != 1) {  /* Tests the != operator with constant 1 */
        return mode + 100;
    } else {
        return mode - 100;
    }
}

/* Test 4: Nested loops with phi propagation */
int test_nested_loop_phi(int limit) {
    int counter = 0;
    int decision = 0;
    
    /* Outer loop */
    for (int i = 0; i < limit; ++i) {
        /* Inner loop creates phi for 'decision' */
        for (int j = 0; j < 10; ++j) {
            if ((i + j) % 7 == 0) {
                decision = 1;
            } else {
                decision = 0;
            }
            counter += decision;
        }
        
        /* Copy chain inside hot loop */
        int d1 = decision;
        int d2 = d1;
        int d3 = copy1(d2);
        int d4 = copy2(d3);
        
        /* Hot conditional - executed limit * 10 times */
        if (d4 == 0) {
            counter += 2;
        } else {
            counter += 1;
        }
    }
    
    return counter;
}

/* Test 5: Recursive function creating phi nodes */
int test_recursive_phi(int depth, int current) {
    int flag;
    
    if (current >= depth) {
        flag = 1;
    } else {
        /* Recursive call creates phi for return value */
        int child_result = test_recursive_phi(depth, current + 1);
        flag = (child_result % 2 == 0) ? 0 : 1;
    }
    
    /* Copy chain after recursive phi */
    int f1 = flag;
    int f2 = copy3(f1);
    int f3 = f2;
    
    /* Conditional with explicit 0/1 comparison */
    if (f3 == 1) {
        return current * 10;
    } else {
        return current * 5;
    }
}

/* Test 6: Boolean variable (guaranteed 0/1) with phi */
int test_bool_phi(int x, int y) {
    /* Boolean operations create 0/1 values */
    _Bool b1 = (x > y);
    _Bool b2 = (x % 2 == 0);
    
    /* Phi node for boolean result */
    _Bool result;
    if (x > 100) {
        result = b1 && b2;
    } else {
        result = b1 || b2;
    }
    
    /* Boolean in conditional context */
    int r1 = result;  /* Convert bool to int */
    int r2 = copy1(r1);
    int r3 = copy2(r2);
    
    /* if (bool_var) is equivalent to if (bool_var == 1) */
    if (r3) {  /* This should trigger the 0/1 comparison logic */
        return x + y;
    } else {
        return x - y;
    }
}

/* Test 7: Static variable creating phi through multiple calls */
int test_static_var_phi(int val) {
    static int history[4] = {0};
    static int index = 0;
    
    /* Update circular buffer */
    history[index % 4] = val;
    index++;
    
    /* Phi-like behavior from static variable */
    int avg = (history[0] + history[1] + history[2] + history[3]) / 4;
    
    /* Copy chain */
    int a1 = avg;
    int a2 = a1;
    int a3 = copy3(a2);
    
    /* Conditional with 0 comparison */
    if (a3 == 0) {
        return 1;
    } else {
        return 0;
    }
}

/* Main driver with hot loop */
int main() {
    int total = 0;
    const int iterations = 100000;
    
    /* Seed for reproducible behavior */
    srand(42);
    
    printf("Starting auto-profile pattern tests...\n");
    
    /* Hot loop - executes many times to create hot path annotations */
    for (int i = 0; i < iterations; ++i) {
        /* Mix different test patterns */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i);
        total += test_phi_from_switch(i);
        total += test_nested_loop_phi(i % 10 + 1);
        
        if (i % 100 == 0) {
            total += test_recursive_phi(5, 0);
        }
        
        total += test_bool_phi(i, i / 2);
        total += test_static_var_phi(i % 100);
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect((i % 7) == 0, 1)) {
            volatile_sink = i;  /* Hot path operation */
        }
    }
    
    printf("Total result: %d\n", total);
    printf("Test completed. Check for auto-profile coverage.\n");
    
    return 0;
}
