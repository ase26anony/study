/* Test program for GCC auto-profile coverage of phi-node-defined conditional branches */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Helper to create copy chains */
static inline int copy1(int x) { return x; }
static inline int copy2(int x) { return x; }
static inline int copy3(int x) { return x; }

/* Volatile helper to prevent optimization */
static volatile int sink;

/* Test 1: Phi node created after a loop with multiple exit conditions */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int i;
    
    /* Loop creates phi for 'i' at entry */
    for (i = 0; i < iterations; ++i) {
        if (i % 100 == 0) {
            result += 1;
        }
        sink = i; /* Prevent optimization */
    }
    
    /* 'i' is defined by phi node at block entry after loop */
    int a = i;          /* Start copy chain */
    int b = copy1(a);   /* Copy through function */
    int c = copy2(b);   /* Another copy */
    int d = copy3(c);   /* Final copy */
    
    /* Conditional comparing phi-defined variable against 0 */
    if (d == 0) {       /* cmp_lhs is SSA_NAME from phi, cmp_rhs is 0 */
        result += 10;
    }
    
    return result;
}

/* Test 2: Phi node from switch statement with different assignments */
int test_phi_from_switch(int mode) {
    int flag;
    
    switch (mode % 4) {
        case 0:
            flag = 0;
            break;
        case 1:
            flag = 1;
            break;
        case 2:
            flag = 0;
            break;
        default:
            flag = 1;
            break;
    }
    
    /* 'flag' is defined by phi node from switch */
    int x = flag;
    int y = x;
    int z = y;
    
    /* Compare against 1 */
    if (z == 1) {       /* cmp_rhs is 1 */
        return 100;
    }
    
    return 200;
}

/* Test 3: Multiple return paths creating phi for return value */
int test_phi_multiple_returns(int x, int y) {
    int temp;
    
    if (x > y) {
        temp = 1;
    } else if (x < y) {
        temp = 0;
    } else {
        temp = 1;
    }
    
    /* 'temp' is phi-defined from three predecessors */
    int a = temp;
    int b = a;
    
    /* Boolean context - equivalent to comparison with 0 */
    if (b) {            /* if (b != 0) */
        return x * 2;
    }
    
    return y * 2;
}

/* Test 4: Recursive function creating phi nodes */
int test_phi_recursive(int n, int depth) {
    if (depth <= 0) {
        return n % 2;   /* Returns 0 or 1 */
    }
    
    int left = test_phi_recursive(n + 1, depth - 1);
    int right = test_phi_recursive(n - 1, depth - 1);
    
    /* 'result' is phi-defined from recursive calls */
    int result = (left > right) ? left : right;
    int r1 = result;
    int r2 = r1;
    
    /* Compare against 1 */
    if (r2 == 1) {
        return 1;
    }
    
    return 0;
}

/* Test 5: Loop with early exit creating phi for condition */
int test_phi_early_exit(int limit) {
    int found = 0;
    int i;
    
    for (i = 0; i < limit; ++i) {
        if (i == limit / 2) {
            found = 1;
            break;
        }
        sink = i * 2;
    }
    
    /* 'found' is phi-defined (from loop body or after loop) */
    int f1 = found;
    int f2 = f1;
    int f3 = f2;
    
    /* Compare against 0 */
    if (f3 == 0) {
        return -1;
    }
    
    return i;
}

/* Test 6: Complex copy chain with volatile to ensure SSA copies remain */
int test_phi_complex_chain(int x) {
    int val;
    
    if (x > 1000) {
        val = 1;
    } else {
        val = 0;
    }
    
    /* Create a longer copy chain */
    volatile int v1 = val;  /* volatile prevents optimization */
    int c1 = v1;
    int c2 = copy1(c1);
    int c3 = copy2(c2);
    int c4 = copy3(c3);
    int c5 = c4;
    
    /* Multiple comparisons in hot path */
    int result = 0;
    if (c5 == 0) {
        result += 5;
    }
    if (c5 == 1) {
        result += 10;
    }
    
    return result;
}

/* Test 7: Boolean variable from ternary operator */
int test_phi_ternary(int a, int b) {
    /* Ternary creates phi node */
    int is_greater = (a > b) ? 1 : 0;
    
    int t1 = is_greater;
    int t2 = t1;
    
    /* Compare against 1 */
    if (t2 == 1) {
        return a;
    }
    
    return b;
}

/* Main function to create hot execution paths */
int main(void) {
    int total = 0;
    int i;
    
    /* Hot loop - runs many times to make blocks "annotated" as hot */
    for (i = 0; i < 100000; ++i) {
        /* Call each test function multiple times */
        total += test_phi_after_loop(i % 100);
        total += test_phi_from_switch(i);
        total += test_phi_multiple_returns(i, i / 2);
        total += test_phi_recursive(i, 3);
        total += test_phi_early_exit(100);
        total += test_phi_complex_chain(i);
        total += test_phi_ternary(i, i * 2);
        
        /* Use __builtin_expect to hint at hot path */
        if (__builtin_expect((i % 1000) == 0, 0)) {
            total += 1000;  /* Cold path */
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
