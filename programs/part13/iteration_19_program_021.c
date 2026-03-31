/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates phi-node-defined condition variables with copy chains
 * and constant comparisons (0/1) in hot basic blocks.
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_counter = 0;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    volatile_counter++; /* Prevent optimization */
    int a = x;
    int b = a;
    int c = b;
    return c;
}

/* Function with loop-carried phi node */
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for flag */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Other incoming value to phi */
        }
        result += i;
    }
    
    /* After loop: flag is defined by phi node at block entry */
    int flag_copy = copy_chain(flag);  /* Create copy chain */
    
    /* Critical conditional comparing phi-defined variable against 0 */
    if (flag_copy == 0) {  /* cmp_rhs is integer constant 0 */
        result += 1000;
    } else {
        result += 2000;
    }
    
    return result;
}

/* Function with switch creating phi node */
int test_phi_from_switch(int x) {
    int status = 0;
    
    /* Switch with multiple cases creates phi for status */
    switch (x % 4) {
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
    
    /* Create copy chain */
    int a = status;
    int b = a;
    volatile_counter++;
    int c = b;
    
    /* Conditional with comparison against 1 */
    if (c == 1) {  /* cmp_rhs is integer constant 1 */
        return x * 2;
    } else {
        return x * 3;
    }
}

/* Function with multiple returns creating phi */
int test_phi_from_multiple_returns(int x, int y) {
    int decision;
    
    if (x > y) {
        decision = 1;
    } else if (x < y) {
        decision = 0;
    } else {
        decision = 1;  /* Equal case */
    }
    
    /* Multiple assignments to create copy chain */
    int temp = decision;
    for (int i = 0; i < 3; i++) {
        temp = copy_chain(temp);
    }
    
    /* Conditional branch */
    if (temp != 0) {  /* Comparison with 0 (via != operator) */
        return x - y;
    }
    return y - x;
}

/* Recursive function creating phi for condition */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int recurse_flag;
    if (n % 2 == 0) {
        recurse_flag = 1;
    } else {
        recurse_flag = 0;
    }
    
    /* Copy chain */
    int flag_copy = recurse_flag;
    flag_copy = copy_chain(flag_copy);
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(flag_copy == 1, 1)) {
        return test_recursive_phi(n / 2, depth - 1) + 1;
    } else {
        return test_recursive_phi(n * 3 + 1, depth - 1) + 1;
    }
}

/* Function with nested loops and phi */
int test_complex_phi_pattern(int limit) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < limit; i++) {
        int inner_flag = 0;
        
        /* Inner loop creates phi for inner_flag */
        for (int j = 0; j < 10; j++) {
            if ((i + j) % 7 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            sum += j;
        }
        
        /* After inner loop: inner_flag is phi-defined */
        int chain1 = inner_flag;
        int chain2 = chain1;
        volatile_counter++;
        int chain3 = chain2;
        
        /* Conditional in hot path (inside outer loop) */
        if (chain3 == 0) {
            outer_flag = 1;
        } else {
            outer_flag = 0;
        }
        
        /* Another conditional using outer_flag */
        int outer_copy = copy_chain(outer_flag);
        if (outer_copy == 1) {
            sum += i * 100;
        }
    }
    
    return sum;
}

/* Main function to create hot execution paths */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Hot loop to make basic blocks annotated as hot */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Call all test functions to exercise different patterns */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_switch(i);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_recursive_phi(i, 5);
        total += test_complex_phi_pattern(10);
        
        /* Occasionally reset to create varying patterns */
        if (i % 1000 == 0) {
            volatile_counter = 0;
        }
    }
    
    printf("Result: %d (volatile ops: %d)\n", total, volatile_counter);
    return 0;
}
