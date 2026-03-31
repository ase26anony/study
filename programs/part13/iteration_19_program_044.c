/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates phi-node-defined condition variables with copy chains
 * and constant comparisons (0/1) in hot basic blocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

volatile int global_counter = 0;
volatile int force_volatile = 0;

/* Helper to create copy chains */
static inline int copy_once(int x) {
    volatile int tmp = x;  /* Prevent optimization */
    return tmp + force_volatile - force_volatile;
}

static inline int copy_twice(int x) {
    int a = copy_once(x);
    int b = copy_once(a);
    return b;
}

/* Pattern 1: Phi from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;  /* Will become phi node at loop header */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create phi for 'flag' with values from different paths */
        if (i % 3 == 0) {
            flag = 1;  /* Hot path */
            result += i;
        } else {
            flag = 0;  /* Cold path */
            result -= i;
        }
        global_counter++;  /* Side effect to prevent optimization */
    }
    
    /* Create copy chain from phi-defined variable */
    int a = copy_once(flag);
    int b = copy_twice(a);
    int c = copy_once(b);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (c == 1) {  /* This should be the hot comparison */
        return result * 2;
    } else {
        return result / 2;
    }
}

/* Pattern 2: Phi from multiple return paths */
int test_phi_from_multiple_returns(int x) {
    int condition;
    
    if (x > 1000) {
        condition = 1;  /* Hot path */
        global_counter += x;
    } else if (x > 100) {
        condition = 0;
        global_counter -= x;
    } else {
        condition = 0;
        global_counter += 1;
    }
    
    /* Create copy chain */
    volatile int tmp1 = condition;
    int tmp2 = tmp1;
    int tmp3 = copy_once(tmp2);
    
    /* Compare against 0 */
    if (tmp3 == 0) {
        return x * 3;
    } else {
        return x + 100;
    }
}

/* Pattern 3: Phi from switch statement */
int test_phi_from_switch(int mode) {
    int flag;
    
    switch (mode % 4) {
        case 0:
            flag = 1;  /* Hot case */
            for (int i = 0; i < 10; i++) global_counter++;
            break;
        case 1:
            flag = 0;
            global_counter--;
            break;
        case 2:
            flag = 1;
            global_counter += 2;
            break;
        default:
            flag = 0;
            global_counter = global_counter * 2;
            break;
    }
    
    /* Longer copy chain */
    int a = flag;
    int b = a;
    int c = copy_once(b);
    int d = copy_twice(c);
    int e = copy_once(d);
    
    /* Compare against 1 */
    if (e == 1) {
        return mode * 10;
    }
    return mode;
}

/* Pattern 4: Boolean phi with implicit 0/1 comparison */
bool test_bool_phi(int x) {
    bool flag;
    
    /* Create phi with boolean values */
    if (x % 7 == 0) {
        flag = true;  /* true == 1 */
        global_counter += x;
    } else {
        flag = false; /* false == 0 */
        global_counter -= x;
    }
    
    /* Copy chain with boolean */
    bool a = flag;
    bool b = copy_once(a) != 0;
    bool c = copy_twice(b) != 0;
    
    /* if (c) is equivalent to if (c == 1) for booleans */
    if (c) {
        return true;
    }
    return false;
}

/* Pattern 5: Nested loops creating complex phi */
int test_nested_loop_phi(int outer) {
    int final_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < outer; i++) {
        int inner_flag = 0;
        
        for (int j = 0; j < 100; j++) {
            /* Phi for inner_flag */
            if ((i + j) % 5 == 0) {
                inner_flag = 1;
                sum += j;
            } else {
                inner_flag = 0;
                sum -= j;
            }
        }
        
        /* Phi for final_flag across outer loop iterations */
        if (inner_flag == 1) {
            final_flag = 1;
        } else {
            final_flag = 0;
        }
    }
    
    /* Copy chain and comparison */
    int a = final_flag;
    int b = copy_once(a);
    
    if (b == 0) {
        return sum * -1;
    } else {
        return sum;
    }
}

/* Pattern 6: Recursive function creating phi */
int test_recursive_phi(int depth, int current) {
    int flag;
    
    if (depth <= 0) {
        flag = 1;
        return current;
    }
    
    /* Recursive calls create phi for return value */
    int left = test_recursive_phi(depth - 1, current * 2);
    int right = test_recursive_phi(depth - 1, current + 1);
    
    if (left > right) {
        flag = 1;
    } else {
        flag = 0;
    }
    
    /* Copy chain */
    volatile int tmp = flag;
    int result_flag = tmp;
    
    /* Comparison */
    if (result_flag == 1) {
        return left;
    } else {
        return right;
    }
}

/* Main driver to create hot execution paths */
int main() {
    int total = 0;
    
    /* Execute many times to make paths hot */
    for (int iteration = 0; iteration < 100000; iteration++) {
        /* Mix different patterns to exercise various phi formations */
        total += test_phi_after_loop(iteration % 100 + 1);
        total += test_phi_from_multiple_returns(iteration);
        total += test_phi_from_switch(iteration);
        total += test_bool_phi(iteration) ? 1 : 0;
        total += test_nested_loop_phi(iteration % 10 + 1);
        
        /* Less frequent recursive call to avoid stack overflow */
        if (iteration % 1000 == 0) {
            total += test_recursive_phi(3, iteration);
        }
        
        /* Add some branching to create more profile data */
        if (iteration % 7 == 0) {
            force_volatile = iteration;
        }
    }
    
    printf("Result: %d (global_counter: %d)\n", total, global_counter);
    return total != 0 ? 0 : 1;
}
