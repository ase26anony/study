/* test_auto_profile.c - Test program for GCC auto-profile coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Helper function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    return c;
}

/* Test 1: Phi node from loop-carried dependency */
int test_phi_from_loop(int iterations) {
    int result = 0;
    int flag = 0;  /* This will become a phi node */
    
    for (int i = 0; i < iterations; i++) {
        /* Create phi node for flag with two incoming edges */
        if (i % 2 == 0) {
            flag = 0;  /* First incoming value to phi */
        } else {
            flag = 1;  /* Second incoming value to phi */
        }
        
        /* Use flag in computation to prevent dead code elimination */
        result += flag;
    }
    
    /* Create copy chain from phi-defined variable */
    int copied_flag = copy_chain(flag);
    
    /* Critical conditional: compare phi-defined variable against 0 */
    if (copied_flag == 0) {  /* Line should be annotated as hot */
        result += 1000;
    } else {
        result += 1;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x) {
    int condition;
    
    if (x < 0) {
        condition = 0;  /* First incoming value */
    } else if (x > 100) {
        condition = 1;  /* Second incoming value */
    } else {
        condition = x % 2;  /* Third incoming value */
    }
    
    /* Multiple assignments to create copy chain */
    int a = condition;
    volatile_sink = a;
    int b = a;
    int c = b;
    
    /* Conditional comparing against 1 */
    if (c == 1) {  /* Should trigger the uncovered code */
        return x * 2;
    } else {
        return x / 2;
    }
}

/* Test 3: Phi node from switch statement */
int test_phi_from_switch(int mode) {
    int flag;
    
    switch (mode % 4) {
        case 0:
            flag = 0;
            break;
        case 1:
            flag = 1;
            break;
        case 2:
            flag = 0;
            break;
        case 3:
            flag = 1;
            break;
    }
    
    /* Extended copy chain */
    int tmp1 = flag;
    volatile_sink = tmp1;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    int tmp4 = tmp3;
    
    /* Boolean context comparison (implicit == 1) */
    if (tmp4) {  /* Equivalent to tmp4 == 1 */
        return mode * 3;
    }
    return mode;
}

/* Test 4: Recursive function creating phi for depth */
int test_phi_from_recursion(int n, int depth) {
    int should_continue;
    
    if (depth > 10 || n <= 0) {
        should_continue = 0;
    } else {
        should_continue = 1;
    }
    
    /* Copy chain */
    int a = should_continue;
    volatile_sink = a;
    int b = a;
    
    /* Compare against 0 */
    if (b == 0) {
        return n;
    }
    
    /* Recursive call creates phi for return value */
    return test_phi_from_recursion(n - 1, depth + 1);
}

/* Test 5: Complex phi with nested control flow */
int test_complex_phi(int x) {
    int value = 0;
    int i;
    
    for (i = 0; i < 100; i++) {
        int inner_flag;
        
        if (i % 3 == 0) {
            inner_flag = 1;
        } else {
            inner_flag = 0;
        }
        
        /* Multiple copy assignments */
        int t1 = inner_flag;
        int t2 = t1;
        volatile_sink = t2;
        int t3 = t2;
        
        /* Conditional inside loop (hot path) */
        if (t3 == 1) {
            value += x + i;
        } else {
            value += 1;
        }
    }
    
    /* Final conditional with phi from loop */
    int final_flag = (value > 5000) ? 1 : 0;
    int f1 = final_flag;
    int f2 = f1;
    
    if (f2 == 1) {
        return value * 2;
    }
    return value;
}

/* Test 6: Global variable creating phi */
static int global_counter = 0;

int test_phi_with_global(int x) {
    int local_flag;
    
    /* Update global in different paths */
    if (x % 2 == 0) {
        global_counter++;
        local_flag = 1;
    } else {
        global_counter--;
        local_flag = 0;
    }
    
    /* Long copy chain */
    int chain1 = local_flag;
    volatile_sink = chain1;
    int chain2 = chain1;
    int chain3 = chain2;
    int chain4 = chain3;
    int chain5 = chain4;
    
    /* Compare against 0 with != operator */
    if (chain5 != 0) {  /* Should be transformed to == 1 comparison */
        return x + global_counter;
    }
    return x - global_counter;
}

/* Main function to create hot execution paths */
int main(void) {
    int total = 0;
    
    /* Execute many times to create hot paths */
    for (int i = 0; i < 100000; i++) {
        /* Mix different test functions to create varied control flow */
        total += test_phi_from_loop(i % 100);
        total += test_phi_from_multiple_returns(i);
        total += test_phi_from_switch(i);
        
        if (i % 1000 == 0) {
            total += test_phi_from_recursion(i % 20, 0);
        }
        
        total += test_complex_phi(i % 50);
        total += test_phi_with_global(i);
    }
    
    printf("Result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
