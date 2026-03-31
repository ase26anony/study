/* Test program for GCC auto-profile coverage of phi-node-defined conditionals */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization of copy chains */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    volatile_sink = c;
    return c;
}

/* Test 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for flag */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int flag_copy = copy_chain(flag);
    
    /* Critical conditional comparing phi-defined variable against 0/1 */
    if (flag_copy == 1) {  /* Comparison against constant 1 */
        result += 100;
    } else if (flag_copy == 0) {  /* Comparison against constant 0 */
        result += 200;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Multiple returns create phi node for return value */
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
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(c == 1, 1)) {  /* Comparison against 1 */
        return x * 2;
    } else {
        return y * 3;
    }
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
            status = 1;  /* Same value from different case */
            break;
        default:
            status = 0;
            break;
    }
    
    /* Create longer copy chain */
    int tmp1 = status;
    volatile_sink = tmp1;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    volatile_sink = tmp3;
    int final = tmp3;
    
    /* Conditional with both 0 and 1 comparisons */
    if (final != 0) {  /* Equivalent to final == 1 for 0/1 values */
        return 1000;
    }
    return 2000;
}

/* Test 4: Recursive function creating phi for depth */
int test_recursive_phi(int n, int depth) {
    if (n <= 0 || depth >= 5) {
        return 0;
    }
    
    int should_continue;
    if (n % 2 == 0) {
        should_continue = 1;
    } else {
        should_continue = 0;
    }
    
    /* Copy chain */
    int cont_copy = should_continue;
    volatile_sink = cont_copy;
    
    /* Conditional test */
    if (cont_copy == 1) {  /* Compare against 1 */
        return n + test_recursive_phi(n - 1, depth + 1);
    } else {
        return test_recursive_phi(n - 2, depth + 1);
    }
}

/* Test 5: Boolean variable (guaranteed 0/1) with phi */
bool test_bool_phi(int x, int y) {
    bool is_greater;
    
    /* Multiple assignments to bool create phi */
    if (x > 100) {
        is_greater = true;  /* 1 */
    } else if (y > 100) {
        is_greater = false; /* 0 */
    } else {
        is_greater = (x > y); /* 0 or 1 */
    }
    
    /* Boolean in conditional context */
    bool result;
    if (is_greater) {  /* Implicit comparison against 0 */
        result = true;
    } else {
        result = false;
    }
    
    return result;
}

/* Test 6: Complex nested control flow with phi */
int test_complex_phi(int x) {
    int value = 0;
    int flag = 0;
    
    /* Nested loops and conditionals */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            for (int j = 0; j < 5; j++) {
                if (x > j) {
                    flag = 1;
                } else {
                    flag = 0;
                }
                value += j;
            }
        } else {
            flag = (x % 2 == 0) ? 1 : 0;
        }
        
        /* Hot conditional inside loop */
        int flag_copy = flag;
        volatile_sink = flag_copy;
        
        if (flag_copy == 1) {
            value += 10;
        }
    }
    
    return value;
}

/* Main function to create hot execution paths */
int main() {
    int total = 0;
    const int iterations = 100000;
    
    /* Hot loop to make basic blocks annotated as hot */
    for (int i = 0; i < iterations; i++) {
        /* Call all test functions to exercise different patterns */
        total += test_phi_after_loop(i % 100);
        total += test_phi_from_multiple_returns(i, i * 2);
        total += test_phi_from_switch(i);
        total += test_recursive_phi(i % 20, 0);
        total += test_bool_phi(i, i + 1);
        total += test_complex_phi(i);
        
        /* Prevent loop unrolling from changing control flow */
        if (i % 1000 == 0) {
            volatile_sink = i;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
