/* Test program for GCC auto-profile.cc uncovered lines */
/* Compile with: gcc -O2 -fauto-profile -fprofile-arcs -ftree-vectorize -o test_autofdo test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile variables to prevent optimization of copy chains */
static volatile int volatile_sink;

/* Helper to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    return c;
}

/* Function 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* Copy chain between phi and conditional */
    int flag_copy = copy_chain(flag);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (flag_copy == 1) {  /* Comparison against constant 1 */
        result += 1000;
    } else if (flag_copy == 0) {  /* Comparison against constant 0 */
        result += 500;
    }
    
    return result;
}

/* Function 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Multiple returns create phi for return value */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Same value from different path */
    }
    
    /* Create longer copy chain */
    int a = condition;
    int b = a;
    volatile_sink = b;
    int c = b;
    int d = c;
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(d == 1, 1)) {  /* Comparison against 1 */
        return x * 2;
    } else {
        return y * 2;
    }
}

/* Function 3: Phi node from switch statement */
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
            status = 1;  /* Same value from different case */
            break;
        case 3:
            status = 0;  /* Same value from different case */
            break;
    }
    
    /* Multiple copy assignments */
    int tmp1 = status;
    volatile_sink = tmp1;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    
    /* Conditional with 0/1 comparison */
    if (tmp3 != 0) {  /* Comparison (indirectly against 0) */
        return mode * 3;
    } else {
        return mode * 5;
    }
}

/* Function 4: Recursive function creating phi for condition */
int test_phi_from_recursion(int depth, int max_depth) {
    if (depth >= max_depth) {
        return 0;
    }
    
    int child_result = test_phi_from_recursion(depth + 1, max_depth);
    
    /* Phi node created for 'should_process' from recursive calls */
    int should_process = (child_result % 2 == 0) ? 1 : 0;
    
    /* Copy chain */
    int processed = copy_chain(should_process);
    
    /* Hot conditional in loop context */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        if (processed == 1) {  /* Comparison against 1 */
            sum += i * depth;
        } else {
            sum += i;
        }
    }
    
    return sum + child_result;
}

/* Function 5: Boolean variable with phi */
bool test_boolean_phi(int a, int b) {
    bool flag;
    
    if (a > b) {
        flag = true;  /* true = 1 */
    } else {
        flag = false; /* false = 0 */
    }
    
    /* Boolean copy chain */
    bool flag2 = flag;
    volatile_sink = flag2;
    bool flag3 = flag2;
    
    /* Direct boolean test (implicit comparison against 0) */
    if (flag3) {  /* Equivalent to flag3 != 0 */
        return a > b * 2;
    }
    return a < b * 2;
}

/* Function 6: Complex phi with multiple predecessors */
int test_complex_phi_multiple_preds(int iterations) {
    int value = 0;
    int toggle = 0;
    
    /* Complex control flow creating phi nodes */
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            if (i % 3 == 0) {
                toggle = 1;
            } else {
                toggle = 0;
            }
        } else {
            if (i % 5 == 0) {
                toggle = 1;
            } else {
                toggle = 0;
            }
        }
        value += i * toggle;
    }
    
    /* Extended copy chain */
    int t1 = toggle;
    int t2 = t1;
    volatile_sink = t2;
    int t3 = t2;
    int t4 = t3;
    int t5 = t4;
    
    /* Multiple conditionals with 0/1 comparisons */
    int result = 0;
    if (t5 == 1) {
        result = value * 2;
    } else if (t5 == 0) {
        result = value / 2;
    }
    
    return result;
}

/* Main function to create hot execution paths */
int main() {
    int total = 0;
    const int iterations = 100000;
    
    /* Hot loop to make basic blocks "annotated" as hot */
    for (int i = 0; i < iterations; i++) {
        /* Call each test function multiple times */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_phi_from_switch(i);
        
        if (i % 100 == 0) {
            total += test_phi_from_recursion(0, 5);
        }
        
        total += test_boolean_phi(i, i / 3) ? 1 : 0;
        total += test_complex_phi_multiple_preds(i % 50 + 1);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
