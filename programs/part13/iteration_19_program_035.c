/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates phi-node-defined condition variables with copy chains
 * and constant comparisons (0/1) in hot basic blocks.
 */

#include <stdio.h>
#include <stdlib.h>

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
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* After loop: 'flag' is defined by phi node */
    int flag_copy = copy_chain(flag);  /* Create copy chain */
    
    /* Conditional comparing phi-defined variable against 0 */
    if (flag_copy == 0) {  /* cmp_rhs is integer constant 0 */
        result += 100;
    } else {
        result += 200;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Multiple control flow paths create phi for 'condition' */
    if (x > 0) {
        condition = 1;
    } else {
        condition = 0;
    }
    
    /* Create a longer copy chain */
    int a = condition;
    int b = a;
    volatile_sink = b;
    int c = b;
    int d = c;
    
    /* Compare against 1 */
    if (d == 1) {  /* cmp_rhs is integer constant 1 */
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
            status = 1;
            break;
        default:
            status = 0;
            break;
    }
    
    /* Copy chain through volatile */
    volatile int v1 = status;
    int s1 = v1;
    volatile_sink = s1;
    int s2 = s1;
    
    /* Boolean context comparison (implicit != 0) */
    if (s2) {  /* Equivalent to s2 != 0 */
        return 1000;
    }
    return 2000;
}

/* Test 4: Nested control flow with phi */
int test_nested_phi(int a, int b) {
    int flag = 0;
    
    if (a > 0) {
        if (b > 0) {
            flag = 1;
        } else {
            flag = 0;
        }
    } else {
        if (b < 0) {
            flag = 1;
        } else {
            flag = 0;
        }
    }
    
    /* Multiple copy assignments */
    int x1 = flag;
    int x2 = x1;
    int x3 = x2;
    volatile_sink = x3;
    int x4 = x3;
    
    /* Compare against 0 */
    if (x4 == 0) {
        return a + b;
    }
    return a - b;
}

/* Test 5: Recursive function creating phi */
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
    
    /* 'next_val' comes from phi node due to recursion */
    int chain = copy_chain(next_val);
    
    /* Compare result against 1 */
    if (chain == 1) {
        return 1;
    }
    return 0;
}

/* Test 6: Loop with early exit creating phi */
int test_phi_with_early_exit(int limit) {
    int found = 0;
    int sum = 0;
    
    for (int i = 0; i < limit; i++) {
        sum += i;
        if (i == limit / 2) {
            found = 1;
            break;  /* Creates phi for 'found' */
        }
    }
    
    /* 'found' is defined by phi (loop exit vs break) */
    int f1 = found;
    volatile_sink = f1;
    int f2 = f1;
    
    /* Compare against 1 */
    if (f2 == 1) {
        return sum * 2;
    }
    return sum;
}

/* Main function to create hot execution paths */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Hot loop to make basic blocks annotated as "hot" */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Call each test function multiple times */
        total += test_phi_after_loop(i % 100);
        total += test_phi_from_multiple_returns(i, i * 2);
        total += test_phi_from_switch(i);
        total += test_nested_phi(i, i + 1);
        total += test_recursive_phi(i, 3);
        total += test_phi_with_early_exit(100);
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect((i % 10) == 0, 1)) {
            volatile_sink = i;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
