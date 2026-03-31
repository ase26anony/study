/* Test program for GCC auto-profile coverage of phi-defined conditional branches */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile variables to prevent optimization of copy chains */
static volatile int volatile_sink;

/* Helper to create copy chains */
static inline int copy_once(int x) {
    volatile_sink = x;
    return volatile_sink;
}

static inline int copy_twice(int x) {
    int a = copy_once(x);
    int b = copy_once(a);
    return b;
}

/* Pattern 1: Loop-carried phi with copy chain */
int test_loop_phi(int iterations) {
    int result = 0;
    int flag = 0;  /* This will become a phi node */
    
    for (int i = 0; i < iterations; i++) {
        /* Create multiple incoming edges to form phi */
        if (i % 3 == 0) {
            flag = 1;  /* First incoming value */
        } else if (i % 3 == 1) {
            flag = 0;  /* Second incoming value */
        } else {
            flag = (i & 1);  /* Third incoming value (0 or 1) */
        }
        
        /* Use result to prevent dead code elimination */
        result += flag;
    }
    
    /* After loop: flag is defined by phi node at block entry */
    /* Create copy chain */
    int a = copy_once(flag);
    int b = copy_twice(a);
    int c = copy_once(b);
    
    /* Conditional comparing phi-defined variable against 0 or 1 */
    if (c == 0) {  /* Comparison against 0 */
        result += 1000;
    } else if (c == 1) {  /* Comparison against 1 */
        result += 2000;
    }
    
    return result;
}

/* Pattern 2: Multiple return paths creating phi */
int test_multi_return_phi(int x) {
    int condition;
    
    if (x < 0) {
        condition = 1;  /* First incoming value */
    } else if (x > 100) {
        condition = 0;  /* Second incoming value */
    } else {
        condition = (x & 1);  /* Third incoming value (0 or 1) */
    }
    
    /* Create copy chain */
    int a = condition;
    int b = a;
    int c = copy_once(b);
    int d = copy_twice(c);
    
    /* Conditional with comparison against 1 */
    if (d == 1) {
        return x * 2;
    } else {
        return x / 2;
    }
}

/* Pattern 3: Switch statement creating phi */
int test_switch_phi(int mode) {
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
    
    /* Longer copy chain */
    int t1 = flag;
    int t2 = t1;
    int t3 = copy_once(t2);
    int t4 = copy_once(t3);
    int t5 = t4;
    
    /* Comparison against 0 */
    if (t5 == 0) {
        return mode + 100;
    }
    return mode - 100;
}

/* Pattern 4: Nested loops with phi */
int test_nested_loop_phi(int n) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_flag = (i & 1);  /* 0 or 1 */
        
        for (int j = 0; j < 10; j++) {
            /* Mix of assignments to create phi */
            if (j % 2 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            sum += inner_flag;
        }
        
        /* Phi from loop exit */
        outer_flag = inner_flag;
        
        /* Copy and test */
        int tmp = outer_flag;
        tmp = copy_once(tmp);
        
        /* Hot path: use __builtin_expect */
        if (__builtin_expect(tmp == 1, 1)) {
            sum += 100;
        }
    }
    
    return sum;
}

/* Pattern 5: Recursive function with phi */
int test_recursive_phi(int depth, int max_depth) {
    if (depth >= max_depth) {
        return 0;
    }
    
    int flag;
    if (depth % 2 == 0) {
        flag = 1;
    } else {
        flag = 0;
    }
    
    /* Create copy chain */
    int a = flag;
    int b = copy_once(a);
    
    /* Test condition */
    int result = 0;
    if (b == 0) {
        result = depth * 2;
    } else {
        result = depth * 3;
    }
    
    return result + test_recursive_phi(depth + 1, max_depth);
}

/* Pattern 6: Boolean variable (guaranteed 0/1) */
bool test_bool_phi(int x) {
    bool flag1 = (x > 50);
    bool flag2 = (x < 25);
    
    bool combined;
    if (x % 2 == 0) {
        combined = flag1;
    } else {
        combined = flag2;
    }
    
    /* Boolean creates implicit 0/1 comparison */
    bool a = combined;
    bool b = copy_once(a);
    
    /* if (bool_var) is equivalent to if (bool_var == 1) */
    if (b) {  /* Comparison against 1 */
        return true;
    }
    return false;
}

/* Pattern 7: Complex control flow with merge points */
int test_complex_cfg(int x) {
    int value;
    
    if (x < 0) goto negative;
    if (x > 100) goto large;
    
    /* Default path */
    value = 0;
    goto merge;
    
negative:
    value = 1;
    goto merge;
    
large:
    value = (x & 1);  /* 0 or 1 */
    /* fall through */
    
merge:
    /* Here value is defined by phi from 3 predecessors */
    int v1 = value;
    int v2 = copy_once(v1);
    int v3 = v2;
    
    if (v3 == 0) {
        return x * 10;
    } else if (v3 == 1) {
        return x * 20;
    }
    return x;
}

/* Main driver to create hot execution paths */
int main(void) {
    int total = 0;
    const int hot_iterations = 100000;
    
    /* Hot loop to trigger profile annotation */
    for (int i = 0; i < hot_iterations; i++) {
        /* Mix of different patterns */
        total += test_loop_phi(10 + (i % 5));
        total += test_multi_return_phi(i);
        total += test_switch_phi(i);
        total += test_nested_loop_phi(5 + (i % 3));
        
        if (i % 100 == 0) {
            total += test_recursive_phi(0, 5);
            total += test_complex_cfg(i);
        }
        
        /* Boolean test */
        if (test_bool_phi(i)) {
            total += 1;
        }
    }
    
    /* Additional cold calls for coverage */
    total += test_loop_phi(1);
    total += test_multi_return_phi(-1);
    total += test_switch_phi(100);
    
    printf("Result: %d\n", total);
    return 0;
}
