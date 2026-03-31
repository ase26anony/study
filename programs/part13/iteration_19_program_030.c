/* Test program for GCC auto-profile coverage of phi-node-defined conditional branches */
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
    volatile_sink = c;
    return c;
}

/* Test 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            flag = 1;  /* Hot path - executed often */
        } else {
            flag = 0;  /* Cold path */
        }
    }
    
    /* Create copy chain from phi-defined variable */
    int chain_var = copy_chain(flag);
    
    /* Critical conditional comparing against 0/1 */
    if (chain_var == 1) {  /* Compare against constant 1 */
        result = 100;
    } else if (chain_var == 0) {  /* Compare against constant 0 */
        result = 200;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x) {
    int condition;
    
    /* Multiple returns create phi node for return value */
    if (x > 1000) {
        condition = 1;
    } else if (x > 500) {
        condition = 0;
    } else if (x > 100) {
        condition = 1;
    } else {
        condition = 0;
    }
    
    /* Propagate through assignments */
    int temp1 = condition;
    int temp2 = temp1;
    volatile_sink = temp2;
    
    /* Conditional with 0/1 comparison */
    if (temp2 != 0) {  /* Compare against 0 (implicitly) */
        return 1;
    }
    return 0;
}

/* Test 3: Phi node from switch statement */
int test_phi_from_switch(int mode) {
    int flag;
    
    switch (mode % 4) {
        case 0:
            flag = 1;  /* Hot case */
            break;
        case 1:
            flag = 0;
            break;
        case 2:
            flag = 1;  /* Hot case */
            break;
        case 3:
            flag = 0;
            break;
    }
    
    /* Multiple copy assignments */
    int a = flag;
    int b = a;
    int c = b;
    volatile_sink = c;
    
    /* Conditional with explicit 0/1 comparison */
    if (c == 1) {  /* Compare against constant 1 */
        return 10;
    }
    return 20;
}

/* Test 4: Boolean phi node with copy chain */
bool test_boolean_phi(int x, int y) {
    bool condition;
    
    /* Create phi for boolean */
    if (x > y) {
        condition = true;  /* true = 1 */
    } else {
        condition = false; /* false = 0 */
    }
    
    /* Copy chain for boolean */
    bool a = condition;
    bool b = a;
    bool c = b;
    volatile_sink = c;
    
    /* Boolean in conditional (implicit 0/1 comparison) */
    if (c) {  /* Equivalent to if (c != 0) */
        return true;
    }
    return false;
}

/* Test 5: Nested loops creating complex phi */
int test_nested_loop_phi(int limit) {
    int sum = 0;
    int flag = 0;
    
    /* Outer loop */
    for (int i = 0; i < limit; i++) {
        /* Inner loop with phi */
        for (int j = 0; j < 10; j++) {
            if (j % 3 == 0) {
                flag = 1;
            } else {
                flag = 0;
            }
        }
        
        /* Copy and test inside hot loop */
        int test_var = flag;
        volatile_sink = test_var;
        
        /* Hot conditional */
        if (test_var == 0) {  /* Compare against 0 */
            sum += 1;
        } else if (test_var == 1) {  /* Compare against 1 */
            sum += 2;
        }
    }
    
    return sum;
}

/* Test 6: Recursive function creating phi */
int test_recursive_phi(int depth, int current) {
    int flag;
    
    if (current >= depth) {
        flag = 1;
    } else {
        /* Recursive call creates phi for return value */
        int recurse_result = test_recursive_phi(depth, current + 1);
        flag = (recurse_result > 0) ? 1 : 0;
    }
    
    /* Copy chain */
    int a = flag;
    int b = a;
    volatile_sink = b;
    
    /* Conditional test */
    if (b == 1) {
        return 1;
    }
    return 0;
}

/* Test 7: Phi with __builtin_expect to hint hot path */
int test_phi_with_expect(int x) {
    int condition;
    
    if (__builtin_expect(x > 100, 1)) {  /* Hint as hot */
        condition = 1;
    } else {
        condition = 0;
    }
    
    /* Multiple SSA copies */
    int var1 = condition;
    int var2 = var1;
    int var3 = var2;
    volatile_sink = var3;
    
    /* Conditional with 0/1 comparison */
    if (var3 != 1) {  /* Compare against 1 */
        return 5;
    }
    return 10;
}

/* Main driver to create hot execution paths */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Hot loop to make basic blocks annotated as hot */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Call all test functions to exercise different patterns */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i);
        total += test_phi_from_switch(i);
        total += test_boolean_phi(i, i / 2);
        total += test_nested_loop_phi(10);
        
        /* Less frequent to create variation */
        if (i % 100 == 0) {
            total += test_recursive_phi(5, 0);
            total += test_phi_with_expect(i);
        }
        
        /* Prevent overflow */
        if (total > 1000000) {
            total = total % 1000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
