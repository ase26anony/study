/* Test program for GCC auto-profile.cc uncovered lines */
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

/* Another copy chain function */
static int propagate(int val) {
    int tmp1 = val;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    volatile_sink = tmp3;
    return tmp3;
}

/* Test 1: Phi node from loop-carried dependency */
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
    
    /* After loop: 'flag' is defined by phi node */
    int flag_copy = copy_chain(flag);  /* Create copy chain */
    
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
    
    /* Multiple returns create phi for return value */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Same value from different path */
    }
    
    /* Propagate through assignments */
    int a = condition;
    int b = propagate(a);  /* More copy chains */
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
        default:
            status = 0;
            break;
    }
    
    /* Create longer copy chain */
    int s1 = status;
    int s2 = s1;
    volatile_sink = s2;
    int s3 = propagate(s2);
    int s4 = s3;
    
    /* Conditional with phi-defined variable */
    if (s4 == 0) {  /* Comparison against 0 */
        return mode + 10;
    } else {
        return mode + 20;
    }
}

/* Test 4: Recursive function creating phi nodes */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n > 0 ? 1 : 0;  /* Base case returns 0 or 1 */
    }
    
    int left = test_recursive_phi(n + 1, depth - 1);
    int right = test_recursive_phi(n - 1, depth - 1);
    
    /* 'result' gets phi from two recursive calls */
    int result = (left > right) ? left : right;
    
    /* Copy chain */
    int r1 = result;
    int r2 = propagate(r1);
    
    /* Conditional test */
    if (r2 == 1) {  /* Comparison against 1 */
        return n * 2;
    }
    return n;
}

/* Test 5: Boolean variable with phi */
bool test_bool_phi(int a, int b) {
    bool flag;
    
    if (a > b) {
        flag = true;  /* true = 1 */
    } else {
        flag = false; /* false = 0 */
    }
    
    /* Boolean creates implicit 0/1 comparison in SSA */
    bool flag2 = flag;
    volatile_sink = flag2;
    
    /* if (bool_var) compares against 0 */
    if (flag2) {  /* Equivalent to flag2 != 0 */
        return true;
    }
    return false;
}

/* Test 6: Complex nested control flow */
int test_nested_phi(int x) {
    int val = 0;
    
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            if (x > 5) {
                val = 1;
            } else {
                val = 0;
            }
        } else {
            val = (x < 10) ? 1 : 0;
        }
        
        /* Inner loop to create hot path */
        for (int j = 0; j < 100; j++) {
            volatile_sink = j;  /* Prevent optimization */
        }
    }
    
    /* Multiple copy assignments */
    int v1 = val;
    int v2 = v1;
    int v3 = propagate(v2);
    int v4 = v3;
    
    /* Final conditional */
    if (v4 == 1) {
        return x * 100;
    } else if (v4 == 0) {
        return x * 200;
    }
    return x;
}

/* Test 7: Static variable creating phi across calls */
int test_static_phi(int x) {
    static int counter = 0;
    int old_counter = counter;
    
    counter = (counter + x) % 2;  /* Only 0 or 1 */
    
    /* counter is defined by phi (previous value vs new value) */
    int c1 = counter;
    int c2 = propagate(c1);
    
    if (c2 == 0) {
        return old_counter * 10;
    } else {
        return old_counter * 20;
    }
}

/* Main function to drive execution */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Hot loop to make paths "hot" for profile estimation */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Mix different test functions to create varied control flow */
        total += test_phi_after_loop(i % 100);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_phi_from_switch(i);
        
        if (i % 100 == 0) {  /* Less frequent calls for some tests */
            total += test_recursive_phi(i % 10, 3);
            total += test_nested_phi(i % 20);
        }
        
        total += test_bool_phi(i, i / 3) ? 1 : 0;
        total += test_static_phi(i % 7);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
