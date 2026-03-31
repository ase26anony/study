/* test_auto_profile.c - Test program for GCC auto-profile coverage */

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

/* Test 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < iterations; ++i) {
        if (i % 2 == 0) {
            flag = 1;  /* Even iterations */
        } else {
            flag = 0;  /* Odd iterations */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int flag_copy = copy_chain(flag);
    
    /* Critical conditional comparing against 0/1 */
    if (flag_copy == 1) {  /* Compare against 1 */
        result += 1000;
    } else if (flag_copy == 0) {  /* Compare against 0 */
        result += 2000;
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
        condition = 1;  /* Equal case */
    }
    
    /* Copy chain */
    int a = condition;
    int b = a;
    volatile_sink = b;
    int c = b;
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(c == 1, 1)) {
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Test 3: Phi node from switch statement */
int test_phi_from_switch(int mode) {
    int status = 0;
    
    /* Switch creates phi for status variable */
    switch (mode % 4) {
        case 0:
            status = 1;
            break;
        case 1:
            status = 0;
            break;
        case 2:
            status = 1;
            break;
        case 3:
            status = 0;
            break;
    }
    
    /* Extended copy chain */
    int tmp1 = status;
    volatile_sink = tmp1;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    volatile_sink = tmp3;
    int final = tmp3;
    
    /* Conditional comparing against 0 */
    if (final == 0) {
        return mode * 10;
    }
    return mode * 20;
}

/* Test 4: Boolean phi with copy propagation */
bool test_bool_phi(int a, int b) {
    bool cmp_result;
    
    /* Create boolean phi */
    if (a > b) {
        cmp_result = true;  /* Becomes 1 */
    } else {
        cmp_result = false; /* Becomes 0 */
    }
    
    /* Copy through multiple SSA names */
    bool x = cmp_result;
    bool y = x;
    volatile_sink = y;
    bool z = y;
    
    /* Direct boolean test (implicit comparison with 0) */
    if (z) {  /* Equivalent to if (z != 0) */
        return true;
    }
    return false;
}

/* Test 5: Recursive phi creation */
int test_recursive_phi(int n, int depth) {
    static int counter = 0;
    
    if (depth <= 0) {
        /* Base case creates phi for return value */
        int base_val = (counter++ % 2 == 0) ? 1 : 0;
        
        /* Copy chain */
        int a = base_val;
        int b = a;
        volatile_sink = b;
        
        /* Compare against 1 */
        if (b == 1) {
            return n + 100;
        }
        return n + 200;
    }
    
    /* Recursive calls create phi for return values */
    int left = test_recursive_phi(n * 2, depth - 1);
    int right = test_recursive_phi(n * 3, depth - 1);
    
    /* Phi from two recursive calls */
    int result = (left > right) ? left : right;
    
    /* Final conditional */
    int check = (result % 2 == 0) ? 1 : 0;
    if (check == 0) {  /* Compare against 0 */
        return result * 2;
    }
    return result * 3;
}

/* Test 6: Complex phi network with hot loop */
int test_complex_phi_hot_loop(int iterations) {
    int sum = 0;
    int toggle = 0;
    
    /* Hot loop - will make the inner block annotated as hot */
    for (int i = 0; i < iterations; ++i) {
        int value;
        
        /* Create phi for 'value' */
        if (i % 3 == 0) {
            value = 1;
        } else if (i % 3 == 1) {
            value = 0;
        } else {
            value = 1;
        }
        
        /* Multiple copy assignments */
        int v1 = value;
        int v2 = v1;
        volatile_sink = v2;
        int v3 = v2;
        
        /* Hot conditional inside hot loop */
        if (v3 == 1) {
            sum += i * 2;
            toggle = 1;
        } else {
            sum += i;
            toggle = 0;
        }
        
        /* Another conditional using toggle (phi from loop) */
        int t_copy = toggle;
        if (t_copy == 0) {
            sum -= 5;
        }
    }
    
    return sum;
}

/* Main driver to execute all tests repeatedly */
int main(void) {
    int total_result = 0;
    const int outer_iterations = 100000;  /* Large number to create hot paths */
    
    printf("Starting auto-profile coverage test...\n");
    
    /* Execute all test functions many times to create hot execution paths */
    for (int i = 0; i < outer_iterations; ++i) {
        /* Mix different patterns to cover various phi scenarios */
        total_result += test_phi_after_loop(i % 100 + 1);
        total_result += test_phi_from_multiple_returns(i, i / 2);
        total_result += test_phi_from_switch(i);
        total_result += test_bool_phi(i, i / 3);
        
        if (i % 1000 == 0) {
            total_result += test_recursive_phi(i % 10, 3);
        }
        
        total_result += test_complex_phi_hot_loop(10);
        
        /* Prevent compiler from optimizing everything away */
        if (total_result > 1000000000) {
            total_result = total_result % 1000000;
        }
    }
    
    printf("Final result: %d\n", total_result);
    printf("Test completed. Check coverage with:\n");
    printf("  gcc -O2 -fauto-profile -fprofile-arcs test_auto_profile.c -o test\n");
    printf("  ./test\n");
    printf("  gcov -b test_auto_profile.c\n");
    
    return 0;
}
