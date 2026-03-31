/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates patterns where:
 * 1. Conditional branches compare SSA names against 0/1
 * 2. Those SSA names are defined by phi nodes
 * 3. There are copy chains between phi and comparison
 * 4. Basic blocks are annotated as hot via profiling
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

volatile int global_counter = 0;

/* Helper to create copy chains */
static inline int copy_once(int x) {
    volatile int tmp = x;  /* volatile prevents optimization */
    return tmp;
}

static inline int copy_twice(int x) {
    int a = copy_once(x);
    int b = copy_once(a);
    return b;
}

/* Pattern 1: Phi from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;  /* Will become phi node */
    
    /* Loop creates phi for 'flag' */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Other incoming value to phi */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int a = copy_once(flag);
    int b = copy_twice(a);
    int c = copy_once(b);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (c == 1) {  /* Line should trigger: cmp against 1 */
        result += 1000;
    } else if (b == 0) {  /* Another cmp against 0 */
        result += 500;
    }
    
    return result;
}

/* Pattern 2: Phi from multiple returns with copy chain */
int test_phi_from_multiple_returns(int x) {
    int condition;
    
    /* Multiple returns create phi for return value */
    if (x < 0) {
        condition = 1;
    } else if (x > 100) {
        condition = 0;
    } else {
        condition = (x % 2);
    }
    
    /* Extended copy chain */
    int a = condition;
    int b = a;
    int c = copy_once(b);
    int d = copy_twice(c);
    
    /* Hot path conditional */
    if (d == 0) {
        return x * 2;
    } else if (c == 1) {
        return x * 3;
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
    
    /* Multi-step copy chain */
    int tmp1 = flag;
    int tmp2 = tmp1;
    volatile int tmp3 = tmp2;  /* Force SSA copy */
    int tmp4 = tmp3;
    
    /* Conditional in hot loop */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        if (tmp4 == 1) {  /* Compare against 1 */
            sum += i * 2;
        } else if (tmp2 == 0) {  /* Compare against 0 */
            sum += i;
        }
    }
    
    return sum;
}

/* Pattern 4: Recursive function creating phi for condition */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n % 2;  /* Returns 0 or 1 */
    }
    
    int left = test_recursive_phi(n + 1, depth - 1);
    int right = test_recursive_phi(n - 1, depth - 1);
    
    /* Phi created from recursive calls */
    int combined = (left + right) > 0 ? 1 : 0;
    
    /* Copy chain */
    int a = combined;
    int b = copy_once(a);
    
    /* Conditional */
    if (b == 1) {
        return 1;
    }
    return 0;
}

/* Pattern 5: Boolean variable with phi */
bool test_bool_phi(int x) {
    bool flag;
    
    if (x > 50) {
        flag = true;  /* true = 1 */
    } else {
        flag = false; /* false = 0 */
    }
    
    /* Boolean copy chain */
    bool a = flag;
    volatile bool b = a;  /* volatile creates SSA copy */
    bool c = b;
    
    /* Direct boolean test (implicit == 1) */
    if (c) {
        return true;
    }
    
    /* Explicit comparison against 0 */
    if (a == false) {  /* false = 0 */
        return false;
    }
    
    return true;
}

/* Pattern 6: Complex copy chain with ternary operator */
int test_ternary_phi(int x, int y) {
    /* Ternary creates phi */
    int value = (x > y) ? 1 : 0;
    
    /* Long copy chain */
    int v1 = value;
    int v2 = v1;
    int v3 = copy_once(v2);
    int v4 = copy_twice(v3);
    int v5 = v4;
    volatile int v6 = v5;
    int v7 = v6;
    
    /* Multiple comparisons in hot loop */
    int result = 0;
    for (int i = 0; i < 1000; ++i) {
        if (v7 == 1) {
            result += x;
        } else if (v4 == 0) {
            result += y;
        }
        
        /* Modify values to prevent optimization */
        if (i % 100 == 0) {
            v7 = copy_once(v7);
        }
    }
    
    return result;
}

/* Main driver with hot loop */
int main() {
    int total = 0;
    const int iterations = 100000;
    
    /* Hot loop to make basic blocks annotated as hot */
    for (int i = 0; i < iterations; ++i) {
        /* Mix different patterns */
        total += test_phi_after_loop(i % 100);
        
        if (i % 3 == 0) {
            total += test_phi_from_multiple_returns(i);
        }
        
        if (i % 5 == 0) {
            total += test_phi_from_switch(i);
        }
        
        if (i % 20 == 0) {
            total += test_recursive_phi(i, 3);
        }
        
        if (test_bool_phi(i % 100)) {
            total += 1;
        }
        
        if (i % 7 == 0) {
            total += test_ternary_phi(i, i * 2);
        }
        
        /* Prevent dead code elimination */
        global_counter += (total % 1000);
    }
    
    printf("Result: %d (global: %d)\n", total, global_counter);
    return total != 0 ? 0 : 1;
}
