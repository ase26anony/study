/* test_auto_profile.c - Test program for GCC auto-profile coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile helper to prevent optimization */
static volatile int vol_counter = 0;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    int b = a;
    int c = b;
    vol_counter++; /* Prevent optimization */
    return c;
}

/* Test 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;  /* This will become a phi node */
    
    for (int i = 0; i < iterations; ++i) {
        /* Create multiple paths that set flag differently */
        if (i % 3 == 0) {
            flag = 0;  /* Path A */
        } else if (i % 3 == 1) {
            flag = 1;  /* Path B */
        } else {
            flag = (i & 1);  /* Path C - 0 or 1 */
        }
        
        /* Some work to prevent optimization */
        result += i * 2;
    }
    
    /* Create copy chain from phi-defined flag */
    int flag_copy = copy_chain(flag);
    
    /* Critical conditional comparing against 0/1 */
    if (flag_copy == 0) {  /* Compare against 0 */
        result += 100;
    } else if (flag_copy == 1) {  /* Compare against 1 */
        result += 200;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x) {
    int status;
    
    /* Multiple control flow paths creating phi for status */
    if (x < 0) {
        status = 0;  /* Path 1 */
    } else if (x < 100) {
        status = 1;  /* Path 2 */
    } else {
        status = (x % 2);  /* Path 3 - 0 or 1 */
    }
    
    /* Create a longer copy chain */
    int a = status;
    int b = a;
    int c = b;
    int d = c;
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(d == 1, 1)) {  /* Hot path comparison against 1 */
        return x * 2 + 100;
    }
    
    return x + 50;
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
    
    /* Multiple copy assignments */
    int tmp1 = flag;
    int tmp2 = tmp1;
    volatile int tmp3 = tmp2;  /* Volatile to prevent optimization */
    int tmp4 = tmp3;
    
    /* Conditional with both 0 and 1 comparisons */
    if (tmp4 == 0) {
        return mode * 10;
    } else {  /* tmp4 must be 1 here */
        return mode * 20;
    }
}

/* Test 4: Recursive function creating phi for condition */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int next_val;
    if (n % 2 == 0) {
        next_val = 0;
    } else {
        next_val = 1;
    }
    
    /* Recursive call creates phi for the return value */
    int result = test_recursive_phi(next_val, depth - 1);
    
    /* Copy chain */
    int r1 = result;
    int r2 = r1;
    
    /* Conditional test */
    if (r2 == 0) {
        return depth * 10;
    } else {
        return depth * 20;
    }
}

/* Test 5: Boolean variable with phi */
bool test_bool_phi(int x, int y) {
    bool condition;
    
    /* Create phi for boolean */
    if (x > y) {
        condition = true;  /* Becomes 1 */
    } else {
        condition = false; /* Becomes 0 */
    }
    
    /* Copy through multiple variables */
    bool c1 = condition;
    bool c2 = c1;
    bool c3 = c2;
    
    /* Direct boolean test (implicit comparison against 0) */
    if (c3) {  /* Equivalent to if (c3 != 0) */
        return true;
    }
    return false;
}

/* Test 6: Complex chain with function calls */
static int intermediate_copy(int val) {
    int local = val;
    return local;
}

int test_complex_chain(int x) {
    int base;
    
    /* Multiple incoming paths */
    if (x % 2 == 0) {
        base = 0;
    } else {
        base = 1;
    }
    
    /* Chain through function calls */
    int v1 = intermediate_copy(base);
    int v2 = intermediate_copy(v1);
    int v3 = copy_chain(v2);
    
    /* Final comparison against 1 */
    if (v3 == 1) {
        return x * 100;
    }
    return x * 50;
}

/* Main function to create hot execution paths */
int main(void) {
    int total = 0;
    const int hot_iterations = 100000;
    
    printf("Starting auto-profile coverage test...\n");
    
    /* Hot loop to make paths "hot" for profiling */
    for (int i = 0; i < hot_iterations; ++i) {
        /* Mix different test patterns */
        total += test_phi_after_loop(i % 100);
        
        if (i % 3 == 0) {
            total += test_phi_from_multiple_returns(i);
        }
        
        if (i % 5 == 0) {
            total += test_phi_from_switch(i);
        }
        
        if (i % 7 == 0) {
            total += test_recursive_phi(i % 10, 3);
        }
        
        if (i % 11 == 0) {
            total += test_bool_phi(i, i / 2) ? 1 : 0;
        }
        
        if (i % 13 == 0) {
            total += test_complex_chain(i);
        }
        
        /* Prevent overflow */
        if (total > 1000000) {
            total = total % 1000000;
        }
    }
    
    printf("Final result: %d\n", total);
    printf("Volatile counter: %d\n", vol_counter);
    
    return total > 0 ? 0 : 1;
}
