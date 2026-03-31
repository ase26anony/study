/* Test program to cover phi-node-defined conditional branch analysis in GCC AutoFDO */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile variables to prevent optimization from removing copy chains */
static volatile int volatile_sink;

/* Helper to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    volatile_sink = a;  /* Prevent dead code elimination */
    int b = a;
    int c = b;
    return c;
}

/* Another copy chain helper */
static int another_copy(int x) {
    int tmp1 = x;
    int tmp2 = tmp1;
    volatile_sink = tmp2;
    return tmp2;
}

/* Test 1: Loop-carried dependency forming phi, then conditional test */
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
        result += i;
    }
    
    /* 'flag' is defined by phi node at start of this block */
    int flag_copy = copy_chain(flag);  /* Create copy chain */
    
    /* Critical conditional: compares phi-defined variable against 0/1 */
    if (flag_copy == 0) {  /* Comparison against constant 0 */
        result += 100;
    } else {
        result += 200;
    }
    
    return result;
}

/* Test 2: Multiple return paths creating phi for return value */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Same value from different path */
    }
    
    /* 'condition' is defined by phi node here */
    int cond_copy = another_copy(condition);
    int cond_copy2 = copy_chain(cond_copy);  /* Extended copy chain */
    
    /* Compare against constant 1 */
    if (cond_copy2 == 1) {
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Test 3: Switch statement creating phi, then conditional test */
int test_phi_from_switch(int mode) {
    int status;
    
    switch (mode % 4) {
        case 0:
            status = 0;
            break;
        case 1:
            status = 1;
            break;
        case 2:
            status = 0;  /* Same as case 0 */
            break;
        default:
            status = 1;  /* Same as case 1 */
            break;
    }
    
    /* 'status' defined by phi from switch cases */
    int status_copy = status;
    volatile_sink = status_copy;
    int final_status = status_copy;  /* Simple copy chain */
    
    /* Compare against 0 */
    if (final_status != 0) {  /* != comparison with 0 */
        return 1;
    }
    return 0;
}

/* Test 4: Recursive function creating phi for condition */
int test_phi_from_recursion(int n, int depth) {
    if (depth <= 0) {
        return n > 0 ? 1 : 0;
    }
    
    int left = test_phi_from_recursion(n + 1, depth - 1);
    int right = test_phi_from_recursion(n - 1, depth - 1);
    
    /* 'left' and 'right' come from different recursive calls */
    int combined = (left + right) > 1 ? 1 : 0;
    
    /* Create copy chain */
    int combined_copy = combined;
    int final_combined = copy_chain(combined_copy);
    
    /* Compare against 1 */
    if (final_combined == 1) {
        return 100 + n;
    }
    return 50 + n;
}

/* Test 5: Boolean variable with explicit 0/1 comparison */
bool test_boolean_phi(int a, int b) {
    bool is_greater;
    
    /* Different paths setting boolean */
    if (a > b) {
        is_greater = true;  /* Becomes 1 */
    } else {
        is_greater = false; /* Becomes 0 */
    }
    
    /* Boolean variable creates phi */
    bool is_greater_copy = is_greater;
    volatile_sink = is_greater_copy;
    
    /* Implicit comparison against 0 in boolean context */
    if (is_greater_copy) {  /* Equivalent to != 0 */
        return true;
    }
    return false;
}

/* Test 6: Complex copy chain with multiple assignments */
int test_complex_copy_chain(int x) {
    int value;
    
    if (x % 2 == 0) {
        value = 1;
    } else {
        value = 0;
    }
    
    /* Create a long copy chain */
    int v1 = value;
    int v2 = v1;
    volatile_sink = v2;
    int v3 = v2;
    int v4 = v3;
    int v5 = v4;
    int v6 = v5;
    
    /* Final comparison against 1 */
    if (v6 == 1) {
        return x * 10;
    }
    return x * 20;
}

/* Test 7: Nested loops creating hot path annotation */
int test_hot_path_annotation(int outer_iters, int inner_iters) {
    int total = 0;
    int flag = 0;
    
    /* Outer loop to make path hot */
    for (int i = 0; i < outer_iters; i++) {
        /* Inner loop with varying flag */
        for (int j = 0; j < inner_iters; j++) {
            if ((i + j) % 5 == 0) {
                flag = 1;
            } else {
                flag = 0;
            }
            total += j;
        }
        
        /* Phi-defined flag tested in hot path */
        int flag_copy = flag;
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect(flag_copy == 0, 1)) {  /* Hot path */
            total += 1;
        } else {
            total += 2;
        }
    }
    
    return total;
}

/* Main driver that executes all tests repeatedly */
int main(void) {
    int total_result = 0;
    
    /* Execute many times to create hot paths */
    for (int iteration = 0; iteration < 100000; iteration++) {
        /* Mix different test patterns */
        total_result += test_phi_after_loop(iteration % 100 + 1);
        total_result += test_phi_from_multiple_returns(iteration, iteration / 2);
        total_result += test_phi_from_switch(iteration);
        
        if (iteration % 10 == 0) {
            total_result += test_phi_from_recursion(iteration % 5, 2);
        }
        
        total_result += test_boolean_phi(iteration, iteration / 3) ? 1 : 0;
        total_result += test_complex_copy_chain(iteration);
        
        if (iteration % 1000 == 0) {
            total_result += test_hot_path_annotation(10, 50);
        }
    }
    
    printf("Final result: %d\n", total_result);
    return 0;
}
