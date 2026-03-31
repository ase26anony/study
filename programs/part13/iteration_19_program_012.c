/* test_auto_profile.c - Test program for GCC auto-profile coverage */
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

/* Test 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for flag */
    for (int i = 0; i < n; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* First incoming value to phi */
        } else {
            flag = 0;  /* Second incoming value to phi */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int flag_copy = copy_chain(flag);
    
    /* Critical conditional comparing against 0/1 */
    if (flag_copy == 1) {  /* Comparison against constant 1 */
        result += 1000;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Multiple control flow paths create phi for condition */
    if (x > 0) {
        condition = 1;  /* First incoming edge */
    } else {
        condition = 0;  /* Second incoming edge */
    }
    
    /* Multiple copy assignments */
    int a = condition;
    int b = a;
    volatile_sink = b;
    int c = b;
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(c == 0, 0)) {  /* Comparison against constant 0 */
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Test 3: Phi node from switch statement */
int test_phi_from_switch(int mode) {
    int flag;
    
    /* Switch creates multiple incoming edges to phi */
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
        default:
            flag = 0;
            break;
    }
    
    /* Extended copy chain */
    int tmp1 = flag;
    volatile_sink = tmp1;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    int tmp4 = tmp3;
    
    /* Conditional with 0/1 comparison */
    if (tmp4 != 0) {  /* Equivalent to != 0 comparison */
        return 1;
    }
    return 0;
}

/* Test 4: Recursive function creating phi for condition */
int test_phi_from_recursion(int depth, int max_depth) {
    if (depth >= max_depth) {
        return 0;
    }
    
    int child_result = test_phi_from_recursion(depth + 1, max_depth);
    
    /* Phi node created from recursive calls */
    int condition = (child_result > 0) ? 1 : 0;
    
    /* Copy chain */
    int a = condition;
    int b = a;
    volatile_sink = b;
    
    /* Conditional with boolean context (implicit 0/1 comparison) */
    if (b) {  /* Implicit comparison against 0 */
        return depth + 100;
    }
    return depth;
}

/* Test 5: Complex nested control flow */
int test_complex_phi(int x) {
    int flag = 0;
    
    /* Nested loops and conditionals */
    for (int i = 0; i < 10; ++i) {
        if (x > i) {
            for (int j = 0; j < 5; ++j) {
                if ((i + j) % 2 == 0) {
                    flag = 1;
                } else {
                    flag = 0;
                }
                volatile_sink = i * j;
            }
        }
    }
    
    /* Multiple copy operations */
    int val1 = flag;
    int val2 = val1;
    int val3 = val2;
    
    /* Direct 0/1 comparison */
    if (val3 == 1) {
        return x * 10;
    }
    return x;
}

/* Test 6: Boolean variable with phi */
bool test_bool_phi(int a, int b) {
    bool result;
    
    /* Boolean phi node */
    if (a > b) {
        result = true;  /* true = 1 */
    } else {
        result = false; /* false = 0 */
    }
    
    /* Copy chain for boolean */
    bool tmp1 = result;
    bool tmp2 = tmp1;
    volatile_sink = tmp2;
    
    /* Boolean comparison (implicit 0/1) */
    if (tmp2) {
        return true;
    }
    return false;
}

/* Main driver with hot loop */
int main() {
    int total = 0;
    const int iterations = 100000;
    
    /* Hot loop to make basic blocks annotated as hot */
    for (int i = 0; i < iterations; ++i) {
        /* Call all test functions to exercise different patterns */
        total += test_phi_after_loop(i % 100);
        total += test_phi_from_multiple_returns(i, i * 2);
        total += test_phi_from_switch(i);
        total += test_phi_from_recursion(0, 5);
        total += test_complex_phi(i);
        total += test_bool_phi(i, i / 2);
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 1000 == 0) {
            volatile_sink = i;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
