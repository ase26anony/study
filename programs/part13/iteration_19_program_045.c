/* Test program for GCC auto-profile coverage of phi-node-defined conditional branches */
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
    
    /* Loop creates phi node for flag */
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        result += i;
    }
    
    /* Create copy chain */
    int flag_copy = copy_chain(flag);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (flag_copy == 0) {  /* This should trigger the uncovered code */
        result += 100;
    } else if (flag_copy == 1) {
        result += 200;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Different return paths create phi for condition */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Equal case */
    }
    
    /* Multiple copy assignments */
    int a = condition;
    int b = a;
    volatile_sink = b;
    int c = b;
    
    /* Hot path conditional - use builtin_expect to hint */
    if (__builtin_expect(c == 1, 1)) {  /* Likely hot */
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Test 3: Phi node from switch statement */
int test_phi_from_switch(int mode) {
    int status;
    
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
        default:
            status = 1;
    }
    
    /* Create longer copy chain */
    int s1 = status;
    int s2 = s1;
    volatile_sink = s2;
    int s3 = s2;
    int s4 = s3;
    
    /* Conditional with 0/1 comparison */
    if (s4 == 0) {
        return mode + 10;
    } else {
        return mode + 20;
    }
}

/* Test 4: Recursive function creating phi nodes */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int next_val;
    if (n % 2 == 0) {
        next_val = test_recursive_phi(n / 2, depth - 1);
    } else {
        next_val = test_recursive_phi(n * 3 + 1, depth - 1);
    }
    
    /* Phi node from recursive calls */
    int phi_val = (next_val > 0) ? 1 : 0;
    
    /* Copy chain */
    int v1 = phi_val;
    volatile_sink = v1;
    int v2 = v1;
    
    /* Conditional test */
    if (v2 == 1) {
        return next_val * 2;
    } else {
        return next_val * 3;
    }
}

/* Test 5: Boolean variable with phi */
bool test_bool_phi(int x, int y) {
    bool flag;
    
    /* Create phi for boolean */
    if (x > 100) {
        flag = true;
    } else if (y > 50) {
        flag = false;
    } else {
        flag = (x + y) > 75;
    }
    
    /* Boolean used in conditional (implicit 0/1 comparison) */
    if (flag) {  /* if (flag != 0) */
        return true;
    } else {
        return false;
    }
}

/* Test 6: Nested loops with phi */
int test_nested_loop_phi(int limit) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < limit; i++) {
        int inner_flag = 0;
        
        /* Inner loop creates phi */
        for (int j = 0; j < 10; j++) {
            if ((i + j) % 3 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            sum += j;
        }
        
        /* Copy and test inner_flag */
        int f1 = inner_flag;
        int f2 = f1;
        volatile_sink = f2;
        
        if (f2 == 1) {
            outer_flag = 1;
        }
    }
    
    /* Final test on outer_flag */
    int of1 = outer_flag;
    int of2 = of1;
    
    if (of2 == 1) {
        return sum * 2;
    } else {
        return sum;
    }
}

/* Main function to create hot execution paths */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Hot loop to make basic blocks "annotated" as hot */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Call each test function to exercise different patterns */
        total += test_phi_after_loop(i % 100);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_phi_from_switch(i);
        total += test_recursive_phi(i, 3);
        total += test_bool_phi(i, ITERATIONS - i);
        total += test_nested_loop_phi(10);
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 1000 == 0) {
            volatile_sink = i;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
