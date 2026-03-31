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
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int flag_copy = copy_chain(flag);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (flag_copy == 1) {  /* Comparison against constant 1 */
        result += 1000;
    }
    
    volatile_sink = result;
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x) {
    int status;
    
    /* Different paths set status differently */
    if (x < 0) {
        status = 0;  /* First incoming value */
    } else if (x < 100) {
        status = 1;  /* Second incoming value */
    } else {
        status = 0;  /* Third incoming value */
    }
    
    /* Multiple copy assignments */
    int a = status;
    int b = a;
    int c = b;
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(c == 0, 1)) {  /* Comparison against constant 0 */
        return x * 2;
    } else {
        return x + 1;
    }
}

/* Test 3: Phi node from switch statement */
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
        case 3:
            flag = 0;
            break;
        default:
            flag = 1;
    }
    
    /* Create longer copy chain */
    int t1 = flag;
    volatile_sink = t1;
    int t2 = t1;
    int t3 = t2;
    volatile_sink = t3;
    int t4 = t3;
    
    /* Conditional with boolean context (implicit comparison to 0) */
    if (t4) {  /* Equivalent to t4 != 0 */
        return mode * 3;
    }
    return mode;
}

/* Test 4: Recursive function creating phi for condition */
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
    
    /* Phi node created for return value from recursion */
    int result = next_val;
    
    /* Copy chain */
    int r1 = result;
    int r2 = r1;
    
    /* Conditional comparing against 1 */
    if (r2 == 1) {
        return 1;
    }
    return 0;
}

/* Test 5: Phi with global variable interaction */
static int global_counter = 0;

int test_phi_with_global(int x) {
    int local_flag;
    
    if (x > 0) {
        global_counter++;
        local_flag = 1;
    } else {
        global_counter--;
        local_flag = 0;
    }
    
    /* Multiple SSA copies */
    int f1 = local_flag;
    volatile_sink = f1;
    int f2 = f1;
    int f3 = f2;
    
    /* Conditional with != comparison */
    if (f3 != 0) {  /* Comparison against constant 0 */
        return x + global_counter;
    }
    return x - global_counter;
}

/* Test 6: Complex nested control flow */
int test_complex_phi(int x) {
    int value = 0;
    int i;
    
    /* Outer loop */
    for (i = 0; i < 10; ++i) {
        int inner_flag;
        
        /* Inner conditional */
        if (x % 2 == 0) {
            inner_flag = 1;
        } else {
            inner_flag = 0;
        }
        
        /* Use inner_flag to modify value */
        if (inner_flag == 1) {
            value += i * 2;
        } else {
            value += i;
        }
        
        /* Modify x to change paths */
        x = (x * 3 + 1) % 100;
    }
    
    /* After loop, test a phi-defined variable */
    int final_flag = (value > 50) ? 1 : 0;
    
    /* Copy chain */
    int chain1 = final_flag;
    int chain2 = chain1;
    int chain3 = chain2;
    
    /* Hot conditional - placed in frequently executed path */
    if (chain3 == 1) {
        return value * 2;
    }
    return value / 2;
}

/* Main function to create hot execution paths */
int main(void) {
    int total = 0;
    
    /* Execute many times to create hot paths */
    for (int iteration = 0; iteration < 100000; ++iteration) {
        /* Mix different test functions to create varied control flow */
        total += test_phi_after_loop(iteration % 100);
        total += test_phi_from_multiple_returns(iteration);
        total += test_phi_from_switch(iteration);
        total += test_recursive_phi(iteration, 5);
        total += test_phi_with_global(iteration % 200 - 100);
        total += test_complex_phi(iteration % 50);
        
        /* Occasionally reset to avoid overflow */
        if (iteration % 1000 == 0) {
            volatile_sink = total;
            total = total % 1000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
