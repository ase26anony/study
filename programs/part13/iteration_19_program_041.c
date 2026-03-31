/* test_auto_profile.c - Test program for GCC auto-profile coverage */
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

/* Test 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < n; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* Copy chain between phi and comparison */
    int chain1 = copy_chain(flag);
    int chain2 = chain1;
    
    /* Conditional comparing phi-defined variable against 0 */
    if (chain2 == 0) {  /* Should trigger: cmp_rhs is 0 */
        result += 100;
    } else {
        result += 200;
    }
    
    return result;
}

/* Test 2: Phi node from switch statement */
int test_phi_from_switch(int mode) {
    int status = 0;
    
    /* Switch creates phi node for 'status' */
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
    
    /* Multiple copy assignments */
    int tmp1 = status;
    int tmp2 = tmp1;
    volatile_sink = tmp2;
    int tmp3 = tmp2;
    
    /* Conditional comparing against 1 */
    if (tmp3 == 1) {  /* Should trigger: cmp_rhs is 1 */
        return 1000;
    }
    return 2000;
}

/* Test 3: Phi node with multiple incoming paths */
int test_phi_multiple_paths(int x, int y) {
    int condition;
    
    /* Multiple returns create phi at merge point */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;
    }
    
    /* Complex copy chain */
    int a = condition;
    int b = a;
    int c = b;
    volatile_sink = c;
    int d = c;
    int e = d;
    
    /* Boolean context (implicit comparison with 0) */
    if (e) {  /* Should trigger: if (e != 0) */
        return x * 2;
    }
    return y * 3;
}

/* Test 4: Nested loops with phi-defined condition */
int test_nested_loop_phi(int limit) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < limit; ++i) {
        int inner_flag = 0;
        
        /* Inner loop creates phi */
        for (int j = 0; j < 10; ++j) {
            if ((i + j) % 2 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            sum += j;
        }
        
        /* Copy and test inner_flag */
        int chain = inner_flag;
        volatile_sink = chain;
        
        if (chain == 0) {  /* Compare against 0 */
            outer_flag = 1;
        }
        
        if (outer_flag == 1) {  /* Compare against 1 */
            sum += i * 10;
        }
    }
    
    return sum;
}

/* Test 5: Recursive function creating phi */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int left = test_recursive_phi(n * 2, depth - 1);
    int right = test_recursive_phi(n + 1, depth - 1);
    
    /* Phi node for comparison variable */
    int compare_val = (left > right) ? 1 : 0;
    
    /* Copy chain */
    int chain1 = compare_val;
    int chain2 = chain1;
    volatile_sink = chain2;
    
    /* Test both 0 and 1 comparisons */
    if (chain2 == 0) {
        return left;
    } else {
        return right;
    }
}

/* Test 6: Global variable with phi */
static int global_counter = 0;
static bool global_flag = false;

int test_global_phi(int iterations) {
    int local_sum = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Toggle global flag - creates phi in SSA */
        global_flag = !global_flag;
        global_counter++;
        
        /* Convert bool to int for comparison */
        int flag_int = global_flag ? 1 : 0;
        
        /* Copy chain */
        int tmp = flag_int;
        volatile_sink = tmp;
        
        /* Compare against 1 */
        if (tmp == 1) {
            local_sum += i;
        }
    }
    
    return local_sum;
}

/* Main function to create hot paths */
int main(void) {
    int total = 0;
    const int hot_iterations = 100000;
    
    printf("Starting auto-profile coverage test...\n");
    
    /* Hot loop to trigger block annotation */
    for (int i = 0; i < hot_iterations; ++i) {
        /* Mix different test patterns */
        total += test_phi_after_loop(i % 100);
        
        if (i % 3 == 0) {
            total += test_phi_from_switch(i);
        }
        
        if (i % 5 == 0) {
            total += test_phi_multiple_paths(i, i / 2);
        }
        
        if (i % 7 == 0) {
            total += test_nested_loop_phi(i % 20);
        }
        
        if (i % 11 == 0) {
            total += test_recursive_phi(i % 10, 3);
        }
        
        if (i % 13 == 0) {
            total += test_global_phi(10);
        }
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect((i % 100) != 0, 1)) {
            /* Hot path code */
            volatile_sink = i;
        }
    }
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total > 0 ? 0 : 1;
}
