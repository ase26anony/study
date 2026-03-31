/* Test program for GCC auto-profile.cc uncovered lines */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Helper to create copy chains */
static inline int copy1(int x) { return x; }
static inline int copy2(int x) { return copy1(x); }
static inline int copy3(int x) { return copy2(x); }

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Pattern 1: Phi from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;  /* This will become a phi node */
    
    /* Loop creates phi for 'flag' at entry of loop body */
    for (int i = 0; i < iterations; ++i) {
        /* Multiple incoming edges to this block create phi for 'flag' */
        if (i % 3 == 0) {
            flag = 1;
        } else if (i % 3 == 1) {
            flag = 0;
        } else {
            flag = flag;  /* Self-dependency creates phi */
        }
        
        /* Use volatile to prevent optimization */
        volatile_sink = i;
    }
    
    /* Create copy chain from phi-defined variable */
    int a = flag;
    int b = copy1(a);
    int c = copy2(b);
    int d = copy3(c);
    
    /* Critical conditional comparing phi-defined variable against 0/1 */
    if (d == 0) {  /* cmp_lhs is SSA_NAME from phi via copy chain */
        result += 1;
    } else if (d == 1) {  /* Another comparison against 1 */
        result += 2;
    }
    
    return result;
}

/* Pattern 2: Phi from multiple return paths */
int test_phi_from_multiple_returns(int x) {
    int condition;
    
    /* Different return paths create phi for return value */
    if (x < 0) {
        condition = 0;
    } else if (x > 100) {
        condition = 1;
    } else {
        condition = (x % 2);
    }
    
    /* Copy chain */
    int t1 = condition;
    int t2 = t1;
    int t3 = copy1(t2);
    int t4 = copy2(t3);
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(t4 == 1, 1)) {
        return x * 2;
    } else if (t4 == 0) {
        return x / 2;
    }
    
    return x;
}

/* Pattern 3: Phi from switch statement */
int test_phi_from_switch(int mode) {
    int flag;
    
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
    int v1 = flag;
    int v2 = v1;
    int v3 = copy1(v2);
    int v4 = copy2(v3);
    int v5 = copy3(v4);
    int v6 = v5;
    
    /* Conditional with phi-defined variable */
    if (v6 == 0) {
        return mode + 1;
    } else if (v6 == 1) {
        return mode - 1;
    }
    
    return mode;
}

/* Pattern 4: Recursive function creating phi */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return (n % 2 == 0) ? 1 : 0;
    }
    
    int left = test_recursive_phi(n * 3 + 1, depth - 1);
    int right = test_recursive_phi(n / 2, depth - 1);
    
    /* Phi created from two recursive calls */
    int combined = (left == right) ? 1 : 0;
    
    /* Copy chain */
    int c1 = combined;
    int c2 = copy1(c1);
    int c3 = copy2(c2);
    
    /* Conditional test */
    if (c3 == 1) {
        return n + 1;
    } else if (c3 == 0) {
        return n - 1;
    }
    
    return n;
}

/* Pattern 5: Boolean phi with direct boolean comparison */
bool test_bool_phi(int x) {
    bool flag1 = (x % 3 == 0);
    bool flag2 = (x % 5 == 0);
    
    /* Phi for boolean from two conditions */
    bool result = flag1 && flag2;
    
    /* Boolean copy chain */
    bool b1 = result;
    bool b2 = b1;
    bool b3 = b2;
    
    /* Direct boolean comparison (implicit 0/1) */
    if (b3) {  /* Equivalent to b3 == 1 */
        return true;
    } else {   /* Equivalent to b3 == 0 */
        return false;
    }
}

/* Pattern 6: Global variable creating phi */
static int global_counter = 0;

int test_global_phi(int x) {
    int old_counter = global_counter;
    
    /* Modify global in different paths */
    if (x > 0) {
        global_counter += 1;
    } else {
        global_counter -= 1;
    }
    
    /* Phi created from old_counter (different from global_counter update) */
    int diff = (global_counter > old_counter) ? 1 : 0;
    
    /* Multi-step copy chain */
    int d1 = diff;
    int d2 = d1;
    int d3 = copy1(d2);
    int d4 = copy2(d3);
    int d5 = copy3(d4);
    
    /* Hot conditional in loop */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        if (d5 == 1) {
            sum += i;
        } else if (d5 == 0) {
            sum -= i;
        }
        volatile_sink = i;  /* Prevent optimization */
    }
    
    return sum;
}

/* Main driver to create hot execution paths */
int main() {
    int total = 0;
    
    /* Execute many times to create hot paths */
    for (int i = 0; i < 100000; ++i) {
        /* Mix different patterns to exercise various phi formations */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i);
        total += test_phi_from_switch(i);
        
        if (i % 1000 == 0) {
            total += test_recursive_phi(i, 3);
            total += test_global_phi(i - 50000);
        }
        
        /* Boolean test */
        if (test_bool_phi(i)) {
            total += 1;
        }
        
        /* Prevent optimization */
        if (total > 1000000) {
            total = total % 1000000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
