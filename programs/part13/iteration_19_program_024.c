/* Test program for GCC auto-profile coverage of phi-defined conditional branches */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_once(int x) { return x; }
static inline int copy_twice(int x) { return copy_once(x); }
static inline int copy_thrice(int x) { return copy_twice(x); }

/* Test 1: Loop-carried phi with copy chain */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;  /* This will become a phi node */
    
    /* Loop creates phi for 'flag' at loop header */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int a = flag;
    int b = copy_once(a);
    int c = copy_twice(b);
    int d = copy_thrice(c);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (d == 0) {  /* cmp_lhs is SSA_NAME from phi via copy chain */
        result += 1000;
    } else if (d == 1) {  /* Another comparison against 1 */
        result += 2000;
    }
    
    volatile_sink = result;  /* Prevent dead code elimination */
    return result;
}

/* Test 2: Multiple return paths creating phi for return value */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Different return paths create phi for return value */
    if (x > 0) {
        condition = 1;
    } else if (x < 0) {
        condition = 0;
    } else {
        condition = (y % 2 == 0) ? 1 : 0;
    }
    
    /* Copy chain */
    int a = condition;
    int b = a;
    int c = copy_once(b);
    
    /* Compare against 0/1 - this block should be hot */
    if (c == 1) {
        return x * 2 + y;
    } else {
        return x + y * 3;
    }
}

/* Test 3: Switch statement with phi merging */
int test_phi_from_switch(int mode, int value) {
    int flag;
    
    switch (mode % 4) {
        case 0:
            flag = 1;
            break;
        case 1:
            flag = 0;
            break;
        case 2:
            flag = (value > 100) ? 1 : 0;
            break;
        default:
            flag = (value < 50) ? 1 : 0;
            break;
    }
    
    /* Extended copy chain */
    int t1 = flag;
    int t2 = t1;
    int t3 = copy_once(t2);
    int t4 = copy_twice(t3);
    int t5 = t4;
    
    /* Multiple comparisons in hot block */
    int result = value;
    if (t5 == 0) {
        result += 10;
    }
    if (t5 == 1) {  /* Second comparison in same basic block */
        result *= 2;
    }
    
    return result;
}

/* Test 4: Recursive function creating phi for condition */
int test_phi_from_recursion(int n, int depth) {
    static int call_count = 0;
    call_count++;
    
    if (depth <= 0) {
        return (n % 2 == 0) ? 1 : 0;  /* Base case */
    }
    
    /* Recursive calls create phi for return value */
    int left = test_phi_from_recursion(n + 1, depth - 1);
    int right = test_phi_from_recursion(n - 1, depth - 1);
    
    /* Phi node for condition */
    int condition = (left == right) ? 1 : 0;
    
    /* Copy chain with volatile to prevent optimization */
    volatile int v1 = condition;
    int c1 = v1;
    int c2 = copy_once(c1);
    int c3 = c2;
    
    /* Hot conditional */
    if (c3 == 1) {
        return n * 10;
    } else {
        return n * 20;
    }
}

/* Test 5: Boolean variable with implicit 0/1 comparison */
int test_bool_phi(int x, int y) {
    bool flag1 = (x > y);
    bool flag2 = (x + y) % 2 == 0;
    
    /* Phi from conditional assignment */
    bool condition;
    if (x > 0) {
        condition = flag1;
    } else {
        condition = flag2;
    }
    
    /* Boolean copy chain - bool is 0/1 in C */
    bool b1 = condition;
    bool b2 = b1 && (y > 0);  /* Still boolean */
    bool b3 = b2 || (x > 100);
    
    /* Implicit comparison against 0/1 in if statement */
    if (b3) {  /* if (b3 != 0) */
        return x - y;
    } else {
        return y - x;
    }
}

/* Test 6: Complex copy chain with arithmetic that doesn't break SSA propagation */
int test_complex_copy_chain(int base) {
    int seed = base % 3;
    int phi_val;
    
    /* Create phi with two incoming paths */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            phi_val = 1;
        } else {
            phi_val = 0;
        }
        /* Some computation to make block non-trivial */
        seed += i * phi_val;
    }
    
    /* Long copy chain preserving SSA_NAME type */
    int v1 = phi_val;
    int v2 = v1 + 0;  /* Addition with 0 preserves SSA_NAME */
    int v3 = v2 * 1;  /* Multiplication by 1 preserves SSA_NAME */
    int v4 = v3;
    int v5 = copy_once(v4);
    int v6 = copy_twice(v5);
    int v7 = v6 & ~0;  /* Bitwise AND with ~0 (all ones) preserves value */
    
    /* Critical comparison against 0 */
    if (v7 == 0) {
        return seed * 2;
    }
    
    return seed + v7;
}

/* Main driver to create hot execution paths */
int main(void) {
    int total = 0;
    const int hot_iterations = 100000;
    
    printf("Starting auto-profile pattern tests...\n");
    
    /* Hot loop to make basic blocks annotated as hot */
    for (int i = 0; i < hot_iterations; ++i) {
        /* Mix different test patterns to exercise various phi formations */
        total += test_phi_after_loop(i % 100 + 1);
        
        if (i % 3 == 0) {
            total += test_phi_from_multiple_returns(i, i * 2);
        }
        
        if (i % 5 == 0) {
            total += test_phi_from_switch(i % 10, i);
        }
        
        if (i % 7 == 0) {
            total += test_phi_from_recursion(i % 20, 3);
        }
        
        if (i % 11 == 0) {
            total += test_bool_phi(i, i / 2);
        }
        
        if (i % 13 == 0) {
            total += test_complex_copy_chain(i);
        }
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect((i % 100) == 0, 0)) {
            /* Cold path - rarely executed */
            volatile_sink = i * 1000;
        }
    }
    
    printf("Total result: %d\n", total);
    printf("Volatile sink: %d\n", volatile_sink);
    
    return total != 0 ? 0 : 1;
}
