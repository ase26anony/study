/* Test program for GCC auto-profile.cc lines 1312-1333
 * Creates patterns where:
 * 1. Conditional branch compares SSA name against 0/1
 * 2. SSA name is defined by phi node
 * 3. Phi definition comes from copy chain
 * 4. Basic block is annotated as hot
 */

#include <stdio.h>
#include <stdlib.h>

volatile int global_counter = 0;
volatile int prevent_opt = 0;

/* Helper to create copy chains */
static inline int copy_once(int x) {
    volatile int tmp = x;  /* Prevent optimization */
    return tmp + prevent_opt - prevent_opt;
}

static inline int copy_twice(int x) {
    int a = copy_once(x);
    int b = copy_once(a);
    return b;
}

/* Pattern 1: Phi from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int flag = 0;
    
    /* Loop creates phi for flag at entry */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* First incoming edge to phi */
        } else {
            flag = 0;  /* Second incoming edge to phi */
        }
        global_counter += i;  /* Side effect to prevent removal */
    }
    
    /* Create copy chain from phi result */
    int flag_copy1 = copy_once(flag);
    int flag_copy2 = copy_twice(flag_copy1);
    int flag_copy3 = copy_once(flag_copy2);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (flag_copy3 == 1) {  /* Must compare against 0 or 1 */
        return 100;
    } else if (flag_copy3 == 0) {  /* Another comparison against 0 */
        return 200;
    }
    return 0;
}

/* Pattern 2: Phi from multiple return paths */
int test_phi_from_multiple_returns(int x) {
    int result;
    
    if (x > 1000) {
        result = 1;  /* First incoming value to phi */
        /* Early return pattern forces phi at merge point */
        return result;
    } else if (x > 500) {
        result = 0;  /* Second incoming value to phi */
        return result;
    } else {
        result = (x % 2);  /* Third incoming value (0 or 1) */
    }
    
    /* Merge point creates phi for result */
    int r1 = copy_once(result);
    int r2 = copy_twice(r1);
    
    /* Critical conditional with 0/1 comparison */
    if (r2 == 0) {
        return x * 2;
    } else {
        return x * 3;
    }
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
    int f1 = flag;
    volatile int f2 = f1;  /* Volatile prevents optimization */
    int f3 = f2;
    int f4 = copy_once(f3);
    int f5 = copy_twice(f4);
    
    /* Multiple comparisons against 0/1 */
    if (f5 == 1) {
        return mode + 1;
    }
    
    if (f5 != 0) {  /* Another 0/1 comparison (negated) */
        return mode - 1;
    }
    
    return mode;
}

/* Pattern 4: Recursive function creating phi for condition */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n % 2;  /* Returns 0 or 1 */
    }
    
    int left = test_recursive_phi(n * 3 + 1, depth - 1);
    int right = test_recursive_phi(n / 2, depth - 1);
    
    /* Phi created from two recursive calls */
    int combined = (left > right) ? 1 : 0;
    
    /* Copy chain */
    int c1 = combined;
    int c2 = copy_once(c1);
    int c3 = copy_twice(c2);
    
    /* Hot conditional */
    if (c3 == 0) {
        return left;
    } else {
        return right;
    }
}

/* Pattern 5: Phi with boolean context */
int test_boolean_phi(int a, int b) {
    /* Create boolean values that become 0/1 */
    int cmp1 = (a > b);
    int cmp2 = (a < b * 2);
    
    int flag;
    if (cmp1 && cmp2) {
        flag = 1;
    } else {
        flag = 0;
    }
    
    /* Boolean use in if condition */
    int f1 = flag;
    int f2 = copy_once(f1);
    
    if (f2) {  /* Equivalent to f2 != 0 */
        return a + b;
    }
    
    /* Another comparison against 1 */
    if (f2 == 1) {  /* Redundant but creates another comparison */
        return a - b;
    }
    
    return 0;
}

/* Main driver to create hot paths */
int main(void) {
    int total = 0;
    
    /* Execute many times to make paths hot */
    for (int i = 0; i < 100000; ++i) {
        /* Mix different patterns to create various phi structures */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i);
        total += test_phi_from_switch(i);
        
        if (i % 100 == 0) {
            total += test_recursive_phi(i, 3);
        }
        
        total += test_boolean_phi(i, i * 2);
        
        /* Prevent compiler from optimizing away loops */
        if (total > 1000000) {
            total = total % 1000000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
