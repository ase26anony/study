/* Test program for GCC auto-profile coverage of phi-node-defined conditional branches */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_once(int x) { return x; }
static inline int copy_twice(int x) { int a = x; int b = a; return b; }
static inline int copy_thrice(int x) { 
    volatile int v1 = x;
    int v2 = v1;
    volatile int v3 = v2;
    return v3;
}

/* Test 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for 'flag' */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* Hot path */
        } else {
            flag = 0;  /* Cold path */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined 'flag' */
    int a = flag;
    int b = copy_once(a);
    int c = copy_twice(b);
    int d = copy_thrice(c);
    
    /* Conditional comparing against 0/1 */
    if (d == 1) {  /* Hot path - should be annotated */
        result += 1000;
    } else if (d == 0) {  /* Cold path */
        result -= 1000;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int computed;
    
    /* Multiple returns create phi at merge point */
    if (x > 100) {
        computed = 1;
    } else if (x > 50) {
        computed = 0;
    } else {
        computed = (y % 2 == 0) ? 1 : 0;
    }
    
    /* Copy chain */
    int a = computed;
    int b = a;
    volatile int c = b;
    int d = c;
    
    /* Comparison against constant 0/1 */
    if (d != 0) {  /* Should be hot with proper inputs */
        return x * 2 + y;
    } else {
        return x + y * 2;
    }
}

/* Test 3: Phi node from switch statement */
int test_phi_from_switch(int mode) {
    int status;
    
    switch (mode % 4) {
        case 0:
            status = 1;  /* Hot case */
            break;
        case 1:
            status = 0;  /* Cold case */
            break;
        case 2:
            status = 1;  /* Hot case */
            break;
        default:
            status = 0;  /* Cold case */
            break;
    }
    
    /* Extended copy chain */
    int v1 = status;
    int v2 = copy_once(v1);
    int v3 = copy_twice(v2);
    int v4 = copy_thrice(v3);
    int v5 = v4;
    
    /* Conditional with 0/1 comparison */
    if (v5 == 1) {
        return mode * 3;
    } else {
        return mode * 2;
    }
}

/* Test 4: Boolean phi with copy propagation */
bool test_bool_phi(bool cond1, bool cond2) {
    bool flag;
    
    /* Phi for boolean */
    if (cond1 && cond2) {
        flag = true;   /* true = 1 */
    } else if (cond1 || cond2) {
        flag = false;  /* false = 0 */
    } else {
        flag = true;   /* true = 1 */
    }
    
    /* Boolean copy chain */
    bool a = flag;
    bool b = a;
    volatile int c = b;  /* Force memory traffic */
    bool d = c;
    
    /* Implicit comparison against 0/1 in boolean context */
    if (d) {  /* if (d != 0) */
        return true;
    } else {
        return false;
    }
}

/* Test 5: Recursive phi creation */
int test_recursive_phi(int n, int depth) {
    int indicator;
    
    if (depth <= 0) {
        indicator = 1;
    } else if (n % 2 == 0) {
        indicator = test_recursive_phi(n / 2, depth - 1);
    } else {
        indicator = 0;
    }
    
    /* Copy chain */
    int x = indicator;
    int y = copy_once(x);
    int z = y;
    
    /* Comparison against 1 */
    if (z == 1) {
        return n + depth;
    }
    return n - depth;
}

/* Test 6: Phi with __builtin_expect to influence annotation */
int test_phi_with_expect(int value) {
    int classification;
    
    /* Multiple paths to create phi */
    if (value > 1000) {
        classification = 1;
    } else if (value > 100) {
        classification = 0;
    } else {
        classification = 1;
    }
    
    /* Copy chain with volatile to prevent optimization */
    int tmp = classification;
    volatile int guard = tmp;
    int final = guard;
    
    /* Use __builtin_expect to hint hot path */
    if (__builtin_expect(final == 1, 1)) {  /* Likely hot */
        return value * 10;
    } else {
        return value * 5;
    }
}

/* Main driver to create hot execution paths */
int main(void) {
    int total = 0;
    volatile_sink = 0;
    
    /* Execute many times to create hot paths */
    for (int i = 0; i < 100000; i++) {
        /* Mix of test calls to create various phi patterns */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i, i * 2);
        total += test_phi_from_switch(i);
        total += test_bool_phi(i % 3 == 0, i % 5 == 0);
        total += test_recursive_phi(i, 3);
        total += test_phi_with_expect(i);
        
        /* Prevent loop optimization */
        if (i % 10000 == 0) {
            volatile_sink = total;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
