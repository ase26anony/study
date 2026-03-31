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
    int flag = 0;
    
    /* Loop creates phi node for flag */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        result += i;
    }
    
    /* Create copy chain */
    int flag_copy = copy_chain(flag);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (flag_copy == 0) {  /* Comparison against 0 */
        result += 100;
    } else if (flag_copy == 1) {  /* Comparison against 1 */
        result += 200;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Multiple paths create phi for condition */
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
    int c = b;
    int d = c;
    
    /* Hot path conditional */
    if (d == 1) {  /* Comparison against 1 */
        return x * 2;
    } else {
        return y * 2;
    }
}

/* Test 3: Phi node from switch statement */
int test_phi_from_switch(int mode) {
    int status = 0;
    
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
    
    /* Create longer copy chain */
    int s1 = status;
    int s2 = s1;
    volatile int s3 = s2;  /* Volatile to prevent optimization */
    int s4 = s3;
    
    /* Conditional in hot path */
    if (s4 == 0) {  /* Comparison against 0 */
        return mode + 100;
    } else {
        return mode + 200;
    }
}

/* Test 4: Recursive function creating phi nodes */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int recurse_result = test_recursive_phi(n + 1, depth - 1);
    
    /* Phi node from recursive calls */
    int phi_var = (recurse_result > n) ? 1 : 0;
    
    /* Copy chain */
    int chain1 = phi_var;
    int chain2 = chain1;
    
    /* Conditional with builtin expect for hot path */
    if (__builtin_expect(chain2 == 1, 1)) {  /* Hot path comparison against 1 */
        return recurse_result * 2;
    } else {
        return recurse_result;
    }
}

/* Test 5: Boolean phi with direct 0/1 comparison */
bool test_bool_phi(int a, int b) {
    bool flag;
    
    /* Multiple assignments create phi */
    if (a > b) {
        flag = true;  /* Becomes 1 */
    } else {
        flag = false; /* Becomes 0 */
    }
    
    /* Boolean copy chain */
    bool flag2 = flag;
    bool flag3 = flag2;
    
    /* Direct comparison in boolean context */
    if (flag3 == true) {  /* Comparison against 1 (true) */
        return a > b;
    } else {
        return a <= b;
    }
}

/* Test 6: Complex phi with nested loops */
int test_complex_phi_nested_loops(int limit) {
    int outer_flag = 0;
    int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < limit; i++) {
        int inner_flag = 0;
        
        /* Inner loop creates phi */
        for (int j = 0; j < 10; j++) {
            if ((i + j) % 2 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            sum += j;
        }
        
        /* Copy chain from inner loop phi */
        int copy1 = inner_flag;
        int copy2 = copy1;
        
        /* Hot path conditional */
        if (copy2 == 1) {  /* Comparison against 1 */
            sum += i * 10;
        }
        
        /* Update outer phi */
        if (sum > 1000) {
            outer_flag = 1;
        }
    }
    
    /* Final conditional on outer phi */
    int final_copy = outer_flag;
    if (final_copy == 0) {  /* Comparison against 0 */
        sum += 10000;
    }
    
    return sum;
}

/* Main function to create hot execution paths */
int main() {
    int total = 0;
    const int HOT_ITERATIONS = 100000;
    
    printf("Starting auto-profile coverage test...\n");
    
    /* Hot loop to make paths "hot" for profiling */
    for (int i = 0; i < HOT_ITERATIONS; i++) {
        /* Mix different test patterns */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_phi_from_switch(i);
        
        if (i % 10 == 0) {
            total += test_recursive_phi(i, 3);
            total += test_complex_phi_nested_loops(i % 20 + 1);
        }
        
        /* Boolean test */
        if (test_bool_phi(i, i / 2)) {
            total += 1;
        }
        
        /* Prevent optimization */
        if (vol_counter > 1000000) {
            vol_counter = 0;
        }
    }
    
    printf("Total result: %d\n", total);
    printf("Volatile counter: %d\n", vol_counter);
    
    return total > 0 ? 0 : 1;
}
