/* Test program for GCC auto-profile coverage of phi-node-defined conditional branches */
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
    int flag = 0;
    
    /* Loop creates phi node for flag */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        result += i;
    }
    
    /* Copy chain before comparison */
    int chain1 = copy_chain(flag);
    int chain2 = chain1;
    
    /* Conditional comparing against 0/1 */
    if (chain2 == 0) {  /* Comparison against 0 */
        result += 100;
    } else if (chain2 == 1) {  /* Comparison against 1 */
        result += 200;
    }
    
    return result;
}

/* Function with phi from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Equal case */
    }
    
    /* Create copy chain */
    int a = condition;
    int b = a;
    volatile_sink = b;
    int c = b;
    
    /* Hot path comparison */
    if (c == 1) {  /* Comparison against 1 */
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Function with switch creating phi node */
int test_phi_from_switch(int mode) {
    int status;
    
    switch (mode % 4) {
        case 0:
            status = 0;
            break;
        case 1:
            status = 1;
            break;
        case 2:
            status = 0;
            break;
        case 3:
            status = 1;
            break;
        default:
            status = 0;
    }
    
    /* Multiple copy assignments */
    int tmp1 = status;
    int tmp2 = tmp1;
    volatile_sink = tmp2;
    int tmp3 = tmp2;
    int tmp4 = tmp3;
    
    /* Conditional with 0/1 comparison */
    if (tmp4 == 0) {
        return mode + 10;
    } else {
        return mode + 20;
    }
}

/* Function with recursive phi creation */
int test_recursive_phi(int depth, int max_depth) {
    if (depth >= max_depth) {
        return 0;
    }
    
    int next_val;
    if (depth % 2 == 0) {
        next_val = test_recursive_phi(depth + 1, max_depth) + 1;
    } else {
        next_val = test_recursive_phi(depth + 1, max_depth) + 2;
    }
    
    /* Phi node created from recursive calls */
    int chain = next_val;
    volatile_sink = chain;
    
    /* Comparison against 0/1 */
    if ((chain % 2) == 0) {  /* Will be optimized to compare against 0 */
        return depth * 2;
    } else {
        return depth * 3;
    }
}

/* Function with boolean phi */
bool test_boolean_phi(int a, int b) {
    bool flag;
    
    if (a > b) {
        flag = true;  /* Becomes 1 */
    } else {
        flag = false; /* Becomes 0 */
    }
    
    /* Copy chain for boolean */
    bool flag2 = flag;
    bool flag3 = flag2;
    volatile_sink = flag3;
    
    /* Direct boolean comparison (compares against 0/1) */
    if (flag3) {  /* Equivalent to flag3 != 0 */
        return true;
    } else {
        return false;
    }
}

/* Function with ternary operator creating phi */
int test_ternary_phi(int x, int y) {
    /* Ternary creates phi node */
    int select = (x > y) ? 1 : 0;
    
    /* Multiple assignments */
    int v1 = select;
    int v2 = v1;
    int v3 = v2;
    volatile_sink = v3;
    
    /* Comparison against 1 */
    if (v3 == 1) {
        return x - y;
    }
    return y - x;
}

/* Main function to create hot execution paths */
int main() {
    int total = 0;
    const int iterations = 100000;
    
    /* Hot loop to make basic blocks annotated as hot */
    for (int i = 0; i < iterations; i++) {
        /* Call all test functions to create various phi patterns */
        total += test_phi_after_loop(i % 100);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_phi_from_switch(i);
        total += test_recursive_phi(0, 5);
        total += test_boolean_phi(i, i / 3);
        total += test_ternary_phi(i, i / 4);
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect((i % 1000) == 0, 0)) {
            /* Cold path */
            volatile_sink = i;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
