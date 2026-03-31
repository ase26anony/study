/* Test program for GCC auto-profile coverage of phi-node-defined conditionals */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization of copy chains */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    return c;
}

/* Test 1: Loop-carried dependency forming phi, with copy chain */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for 'flag' at loop header */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
    }
    
    /* Copy chain between phi and conditional */
    int chain1 = copy_chain(flag);
    int chain2 = chain1;
    
    /* Conditional comparing phi-defined variable against 0 */
    if (chain2 == 0) {  /* Should trigger: cmp_rhs is integer 0 */
        result += 1;
    }
    
    return result;
}

/* Test 2: Multiple return paths creating phi for return value */
int test_phi_from_multiple_returns(int x, int y) {
    int flag;
    
    if (x > y) {
        flag = 1;
    } else if (x < y) {
        flag = 0;
    } else {
        flag = 1;  /* Third incoming path */
    }
    
    /* Create copy chain */
    int a = flag;
    int b = a;
    volatile_sink = b;
    
    /* Compare against 1 */
    if (b == 1) {  /* Should trigger: cmp_rhs is integer 1 */
        return 100;
    }
    return 0;
}

/* Test 3: Switch statement with phi merging */
int test_phi_from_switch(int code) {
    int status = 0;
    
    switch (code % 4) {
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
    
    /* Multiple copy assignments */
    int tmp1 = status;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    volatile_sink = tmp3;
    
    /* Conditional with != comparison */
    if (tmp3 != 0) {  /* Should be transformed to == 0 or == 1 */
        return 50;
    }
    return 10;
}

/* Test 4: Boolean variable with explicit 0/1 values */
bool test_bool_phi(int a, int b) {
    bool condition;
    
    /* Create phi with boolean values */
    if (a > 100) {
        condition = true;  /* Becomes 1 */
    } else {
        condition = false; /* Becomes 0 */
    }
    
    /* Copy through volatile to prevent optimization */
    bool cond1 = condition;
    volatile_sink = cond1;
    bool cond2 = cond1;
    
    /* Direct boolean test - should compare against 0/1 */
    if (cond2) {  /* if (cond2 != 0) */
        return true;
    }
    return false;
}

/* Test 5: Recursive function creating phi for depth */
int test_recursive_phi(int n, int depth) {
    if (n <= 0 || depth >= 5) {
        return 0;
    }
    
    int should_continue;
    if (n % 2 == 0) {
        should_continue = 1;
    } else {
        should_continue = 0;
    }
    
    /* Copy chain */
    int chain = should_continue;
    volatile_sink = chain;
    
    /* Test the phi-defined variable */
    if (chain == 1) {
        return 1 + test_recursive_phi(n - 1, depth + 1);
    }
    return test_recursive_phi(n - 2, depth + 1);
}

/* Test 6: Nested loops with phi */
int test_nested_loop_phi(int limit) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < limit; i++) {
        int inner_flag = 0;
        
        /* Inner loop creates phi */
        for (int j = 0; j < 10; j++) {
            if (j % 2 == 0) {
                inner_flag = 1;
            }
        }
        
        /* Copy and test */
        int copy = inner_flag;
        if (copy == 1) {  /* Compare against 1 */
            sum += i;
        }
        
        /* Update outer phi */
        if (i % 3 == 0) {
            outer_flag = 1;
        }
    }
    
    /* Test outer phi with copy chain */
    int ocopy1 = outer_flag;
    int ocopy2 = ocopy1;
    if (ocopy2 == 0) {
        sum += 1000;
    }
    
    return sum;
}

/* Test 7: Function with __builtin_expect to hint hot path */
int test_hot_path_hint(int x) {
    int result = 0;
    int flag = 0;
    
    /* Multiple incoming paths to phi */
    if (x > 1000) {
        flag = 1;
    } else if (x > 500) {
        flag = 0;
    } else if (x > 100) {
        flag = 1;
    } else {
        flag = 0;
    }
    
    /* Long copy chain */
    int a = flag;
    int b = a;
    int c = b;
    int d = c;
    volatile_sink = d;
    
    /* Use __builtin_expect to mark as hot */
    if (__builtin_expect(d == 1, 1)) {
        result = x * 2;
    } else {
        result = x / 2;
    }
    
    return result;
}

/* Main driver that creates hot execution paths */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Hot loop to make basic blocks "annotated" as hot */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Call all test functions to exercise different patterns */
        total += test_phi_after_loop(i % 100);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_phi_from_switch(i);
        total += test_bool_phi(i, i * 2);
        total += test_recursive_phi(i % 20, 0);
        total += test_nested_loop_phi(50);
        total += test_hot_path_hint(i);
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 1000 == 0) {
            volatile_sink = i;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
