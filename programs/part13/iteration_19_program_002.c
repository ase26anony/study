/* Test program for GCC auto-profile coverage of phi-node-defined conditional branches */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization of copy chains */
static volatile int volatile_sink;

/* Function to create copy chain */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    return c;
}

/* Test 1: Loop-carried dependency forming phi, then conditional test */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for 'flag' at loop header */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* First incoming edge to phi */
        } else {
            flag = 0;  /* Second incoming edge to phi */
        }
    }
    
    /* 'flag' is defined by phi node at start of this block */
    int flag_copy = copy_chain(flag);  /* Create copy chain */
    
    /* Conditional comparing phi-defined variable against 0 */
    if (flag_copy == 0) {  /* Should trigger: cmp_rhs is integer 0 */
        result = 1;
    }
    
    return result;
}

/* Test 2: Multiple return paths create phi for return value */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    if (x > 0) {
        condition = 1;  /* First incoming edge */
    } else {
        if (y > 0) {
            condition = 0;  /* Second incoming edge */
        } else {
            condition = 1;  /* Third incoming edge */
        }
    }
    
    /* 'condition' is defined by phi node here */
    int cond_copy1 = condition;
    int cond_copy2 = cond_copy1;  /* Two-level copy chain */
    
    /* Compare against 1 */
    if (cond_copy2 == 1) {  /* Should trigger: cmp_rhs is integer 1 */
        return x + y;
    }
    return x - y;
}

/* Test 3: Switch statement with flag variable */
int test_phi_from_switch(int mode) {
    int status = 0;
    
    switch (mode % 4) {
        case 0:
            status = 1;  /* First incoming edge */
            break;
        case 1:
            status = 0;  /* Second incoming edge */
            break;
        case 2:
            status = 1;  /* Third incoming edge */
            break;
        default:
            status = 0;  /* Fourth incoming edge */
            break;
    }
    
    /* Create longer copy chain */
    int s1 = status;
    volatile_sink = s1;
    int s2 = s1;
    int s3 = s2;
    
    /* Compare against 0 using != (still compares with 0) */
    if (s3 != 0) {  /* Should trigger: compares with 0 */
        return 100;
    }
    return 200;
}

/* Test 4: Boolean variable from complex condition */
bool test_phi_bool(int a, int b, int c) {
    bool flag;
    
    /* Complex condition creating phi */
    if (a > b) {
        flag = (c % 2 == 0);  /* true/false (1/0) */
    } else {
        flag = (b > c);  /* true/false (1/0) */
    }
    
    /* Boolean is 0 or 1 */
    bool flag_copy = flag;
    
    /* Implicit comparison against 0 in if(flag_copy) */
    if (flag_copy) {  /* Should trigger: if(phi_var) compares with 0 */
        return true;
    }
    return false;
}

/* Test 5: Recursive function creating phi for condition */
int test_phi_recursive(int n, int depth) {
    int result;
    
    if (depth <= 0) {
        result = (n > 0) ? 1 : 0;  /* Base case */
    } else {
        /* Recursive calls create phi for result */
        int r1 = test_phi_recursive(n + 1, depth - 1);
        int r2 = test_phi_recursive(n - 1, depth - 1);
        result = (r1 > r2) ? 1 : 0;
    }
    
    /* Copy chain */
    int r = result;
    int r_copy = r;
    
    /* Compare against 1 */
    if (r_copy == 1) {  /* Should trigger: cmp_rhs is integer 1 */
        return n * 2;
    }
    return n / 2;
}

/* Test 6: Hot loop with phi-defined condition on hot path */
int test_hot_path_phi(int iterations) {
    int sum = 0;
    int toggle = 0;
    
    /* Hot loop - should make basic block annotated as hot */
    for (int i = 0; i < iterations; i++) {
        /* Loop creates phi for 'toggle' */
        if (i % 100 == 0) {
            toggle = 1;
        } else {
            toggle = 0;
        }
        
        /* Immediate use of phi-defined variable */
        int t = toggle;
        int t_copy = t;
        
        /* Hot conditional - likely to be annotated */
        if (t_copy == 1) {  /* Should trigger in hot block */
            sum += i;
        }
    }
    
    return sum;
}

/* Test 7: __builtin_expect to hint hot path */
int test_builtin_expect_phi(int x) {
    int value;
    
    if (x > 1000) {
        value = 1;
    } else {
        value = 0;
    }
    
    int v1 = value;
    int v2 = v1;
    
    /* Use __builtin_expect to mark as hot */
    if (__builtin_expect((v2 == 0), 1)) {  /* Hint that v2 == 0 is likely */
        return x * 2;
    }
    return x * 3;
}

/* Main driver that calls all tests repeatedly to create hot paths */
int main(void) {
    int total = 0;
    const int hot_iterations = 100000;
    
    printf("Starting auto-profile coverage test...\n");
    
    /* Run tests many times to create hot execution paths */
    for (int i = 0; i < hot_iterations; i++) {
        /* Test 1: Loop phi */
        total += test_phi_after_loop(100);
        
        /* Test 2: Multiple returns phi */
        total += test_phi_from_multiple_returns(i % 100, (i + 1) % 100);
        
        /* Test 3: Switch phi */
        total += test_phi_from_switch(i);
        
        /* Test 4: Boolean phi */
        total += test_phi_bool(i, i/2, i/3) ? 1 : 0;
        
        /* Test 5: Recursive phi (limited depth) */
        total += test_phi_recursive(i % 10, 3);
        
        /* Test 6: Hot path phi */
        if (i % 1000 == 0) {  /* Call less frequently due to cost */
            total += test_hot_path_phi(1000);
        }
        
        /* Test 7: Builtin expect */
        total += test_builtin_expect_phi(i % 500);
    }
    
    printf("Total result: %d (used to prevent optimization)\n", total);
    printf("Test completed. Compile with:\n");
    printf("  gcc -O2 -fauto-profile -fprofile-arcs -ftree-vectorize test.c\n");
    printf("  gcc -O2 -fprofile-estimate -fprofile-reorder-functions test.c\n");
    
    return total > 0 ? 0 : 1;
}
