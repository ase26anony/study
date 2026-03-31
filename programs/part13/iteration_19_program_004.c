/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates phi-defined condition variables with copy chains and hot paths
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_counter = 0;

/* Helper to create copy chains */
static inline int copy_once(int x) {
    volatile_counter++;  /* Prevent optimization */
    return x;
}

static inline int copy_twice(int x) {
    int a = x;
    int b = a;
    return copy_once(b);
}

/* Function 1: Loop-carried phi with copy chain */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;  /* This will become a phi node */
    
    /* Create loop with multiple exits to force phi at merge point */
    for (int i = 0; i < iterations; i++) {
        volatile_counter++;  /* Prevent loop optimization */
        
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
            result += i;
        } else if (i % 3 == 1) {
            flag = 0;  /* Another incoming value to phi */
            result -= i;
        } else {
            flag = (i % 2);  /* Third incoming value to phi */
            result += i * 2;
        }
        
        /* Early exit creates additional control flow */
        if (i > iterations / 2 && result > 1000) {
            flag = 1;
            break;
        }
    }
    
    /* Create copy chain from phi-defined variable */
    int a = flag;          /* First copy */
    int b = copy_once(a);  /* Second copy via function */
    int c = copy_twice(b); /* Third copy via chain */
    
    /* Critical conditional comparing against 0/1 */
    if (c == 0) {  /* cmp_rhs is integer constant 0 */
        result += 100;
    } else if (c == 1) {  /* cmp_rhs is integer constant 1 */
        result += 200;
    }
    
    return result;
}

/* Function 2: Switch statement creating phi */
int test_phi_from_switch(int value) {
    int condition = 0;  /* Will become phi node */
    
    switch (value % 4) {
        case 0:
            condition = 1;
            break;
        case 1:
            condition = 0;
            break;
        case 2:
            condition = 1;
            break;
        case 3:
            condition = 0;
            break;
    }
    
    /* Multiple copy assignments */
    int x = condition;
    int y = x;
    int z = y;
    int w = copy_once(z);
    
    /* Conditional with 0/1 comparison on hot path */
    if (w == 1) {
        return value * 2;
    } else {
        return value * 3;
    }
}

/* Function 3: Multiple returns creating phi for return value test */
bool test_phi_from_multiple_returns(int x) {
    if (x < 0) {
        return false;  /* One incoming phi value (0) */
    }
    
    if (x > 100) {
        return true;   /* Another incoming phi value (1) */
    }
    
    /* Complex computation to prevent simplification */
    for (int i = 0; i < 10; i++) {
        volatile_counter += i;
    }
    
    return (x % 2) == 0;  /* Third incoming phi value */
}

/* Function 4: Recursive function creating phi for condition */
int test_phi_from_recursion(int depth, int max_depth) {
    static int call_count = 0;
    call_count++;
    
    if (depth >= max_depth) {
        return 1;  /* Base case returns 1 */
    }
    
    /* Recursive calls with different returns */
    int left = test_phi_from_recursion(depth + 1, max_depth);
    int right = test_phi_from_recursion(depth + 2, max_depth);
    
    /* This creates a phi node for 'result' */
    int result = (left > right) ? left : right;
    
    /* Copy chain */
    int a = result;
    int b = a;
    int c = copy_twice(b);
    
    /* Conditional with 0/1 comparison */
    if (c == 1) {  /* Compare against constant 1 */
        return depth * 10;
    } else {
        return depth * 20;
    }
}

/* Function 5: Boolean variable with explicit 0/1 comparison */
int test_bool_phi(int n) {
    bool flag = false;  /* Boolean will be 0/1 */
    
    /* Loop creating phi for flag */
    for (int i = 0; i < n; i++) {
        if (i % 5 == 0) {
            flag = true;   /* 1 */
        } else if (i % 5 == 1) {
            flag = false;  /* 0 */
        } else {
            flag = (i % 3 == 0);  /* 0 or 1 */
        }
        
        /* Hot path - executed many times */
        volatile_counter += i;
    }
    
    /* Explicit copy chain with volatile to prevent optimization */
    volatile int v1 = flag;
    int v2 = v1;
    int v3 = copy_once(v2);
    
    /* Direct comparison against 0/1 constants */
    if (v3 == 0) {
        return n * 100;
    } else if (v3 == 1) {
        return n * 200;
    }
    
    return n;
}

/* Function 6: Nested loops with phi propagation */
int test_nested_loop_phi(int outer, int inner) {
    int final_flag = 0;
    
    for (int i = 0; i < outer; i++) {
        int inner_flag = 0;
        
        /* Inner loop creates phi for inner_flag */
        for (int j = 0; j < inner; j++) {
            if ((i + j) % 3 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            volatile_counter += j;
        }
        
        /* Phi for final_flag from loop iterations */
        if (i % 2 == 0) {
            final_flag = inner_flag;
        } else {
            final_flag = 1 - inner_flag;
        }
    }
    
    /* Multi-step copy chain */
    int step1 = final_flag;
    int step2 = step1;
    int step3 = copy_twice(step2);
    int step4 = step3;
    
    /* Conditional branch with 0/1 comparison */
    if (step4 == 1) {
        return outer * inner * 10;
    } else {
        return outer * inner * 20;
    }
}

/* Main function to create hot execution paths */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;  /* Large number to create hot paths */
    
    printf("Starting auto-profile test patterns...\n");
    
    /* Hot loop calling all test functions repeatedly */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Mix different patterns to exercise various phi formations */
        total += test_phi_after_loop(i % 100 + 1);
        
        if (i % 10 == 0) {
            total += test_phi_from_switch(i);
            
            bool bool_result = test_phi_from_multiple_returns(i % 50);
            total += bool_result ? 1 : 0;
            
            total += test_phi_from_recursion(0, 3);
        }
        
        if (i % 5 == 0) {
            total += test_bool_phi(i % 20 + 1);
            total += test_nested_loop_phi(i % 5 + 1, i % 10 + 1);
        }
        
        /* Use __builtin_expect to hint at hot path */
        if (__builtin_expect(i < ITERATIONS - 100, 1)) {
            volatile_counter++;
        }
    }
    
    printf("Total result: %d\n", total);
    printf("Volatile counter: %d\n", volatile_counter);
    
    return total > 0 ? 0 : 1;
}
