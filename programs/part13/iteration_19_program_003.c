/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates phi-node-defined condition variables with copy chains
 * and constant 0/1 comparisons in hot basic blocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_counter = 0;

/* Helper to create copy chains */
static inline int copy_once(int x) { return x; }
static inline int copy_twice(int x) { return copy_once(x); }
static inline int copy_thrice(int x) { return copy_twice(x); }

/* Test 1: Phi node created after loop with copy chain */
int test_phi_after_loop(int iterations) {
    int result = 0;
    
    /* Loop creates phi node for 'flag' */
    int flag = 0;
    for (int i = 0; i < iterations; i++) {
        /* Create multiple paths to force phi node */
        if (i % 3 == 0) {
            flag = 1;  /* Path A */
        } else if (i % 3 == 1) {
            flag = 0;  /* Path B */
        } else {
            flag = (i & 1);  /* Path C: 0 or 1 */
        }
        result += flag;
    }
    
    /* Create copy chain from phi-defined variable */
    int a = flag;           /* First copy */
    int b = copy_once(a);   /* Second copy via function */
    int c = copy_twice(b);  /* Third copy */
    int d = copy_thrice(c); /* Fourth copy */
    
    /* Critical conditional comparing against 0 */
    if (d == 0) {  /* cmp_lhs is SSA_NAME from phi via copy chain */
        result += 1000;
    } else {
        result += 2000;
    }
    
    /* Another conditional comparing against 1 */
    int e = copy_once(flag);
    if (e == 1) {  /* Another phi-defined comparison */
        result += 3000;
    }
    
    return result;
}

/* Test 2: Phi node from switch statement with multiple returns */
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
        default:
            status = 0;
            break;
    }
    
    /* Create copy chain */
    int x = status;
    int y = x;
    volatile_counter++;  /* Prevent optimization */
    int z = y;
    
    /* Conditional with phi-defined variable */
    if (z == 1) {
        return 42;
    } else {
        return 24;
    }
}

/* Test 3: Recursive function creating phi for condition */
int test_recursive_phi(int n, int acc) {
    if (n <= 0) {
        return acc;
    }
    
    /* Recursive calls create phi for 'next_val' */
    int next_val;
    if (n % 2 == 0) {
        next_val = 1;
    } else {
        next_val = 0;
    }
    
    /* Copy chain */
    int tmp1 = next_val;
    int tmp2 = copy_once(tmp1);
    
    /* Conditional in hot path (recursive call is hot) */
    int result;
    if (tmp2 == 0) {
        result = test_recursive_phi(n - 1, acc + 1);
    } else {
        result = test_recursive_phi(n - 1, acc + 2);
    }
    
    /* Another conditional after recursion */
    int final_check = copy_twice(next_val);
    if (final_check == 1) {
        return result * 2;
    }
    return result;
}

/* Test 4: Multiple incoming edges to basic block */
int test_multiple_incoming_edges(int x) {
    int value;
    
    if (x < 0) {
        value = 1;
    } else if (x < 100) {
        value = 0;
    } else if (x < 200) {
        value = 1;
    } else {
        value = 0;
    }
    
    /* Long copy chain */
    int chain1 = value;
    int chain2 = chain1;
    int chain3 = copy_once(chain2);
    int chain4 = copy_twice(chain3);
    int chain5 = copy_thrice(chain4);
    
    /* Multiple conditionals with same phi source */
    int result = 0;
    if (chain2 == 0) result += 10;
    if (chain3 == 1) result += 20;
    if (chain4 == 0) result += 30;
    if (chain5 == 1) result += 40;
    
    return result;
}

/* Test 5: Boolean variable (guaranteed 0/1) with phi */
bool test_boolean_phi(int a, int b) {
    bool flag;
    
    /* Different paths set the boolean */
    if (a > b) {
        flag = true;
    } else if (a < b) {
        flag = false;
    } else {
        flag = (volatile_counter & 1);
    }
    
    /* Copy chain with boolean */
    bool flag2 = flag;
    bool flag3 = flag2;
    
    /* Implicit comparison against 0 in boolean context */
    if (flag3) {  /* Equivalent to flag3 != 0 */
        return true;
    }
    
    /* Explicit comparison against 1 */
    bool flag4 = copy_once(flag);
    if (flag4 == 1) {
        return false;
    }
    
    return flag;
}

/* Test 6: Complex loop with phi and copy chain in hot path */
int test_complex_loop_hot_path(int limit) {
    int sum = 0;
    int last_decision = 0;
    
    /* Hot loop - will be annotated as hot */
    for (int i = 0; i < limit * 100; i++) {
        /* Create phi for decision */
        int decision;
        if (i % 7 == 0) {
            decision = 1;
        } else if (i % 5 == 0) {
            decision = 0;
        } else {
            decision = last_decision;
        }
        last_decision = decision;
        
        /* Copy chain inside hot loop */
        int d1 = decision;
        int d2 = d1;
        int d3 = copy_once(d2);
        
        /* Hot conditional comparing against 0/1 */
        if (d3 == 0) {
            sum += i;
        } else {
            sum -= i;
        }
        
        /* Another conditional in same hot block */
        int d4 = copy_twice(decision);
        if (d4 == 1) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Main driver to create hot execution paths */
int main() {
    int total = 0;
    
    /* Execute many times to create hot paths */
    for (int iteration = 0; iteration < 100000; iteration++) {
        /* Mix different test functions */
        total += test_phi_after_loop(iteration % 100);
        total += test_phi_from_switch(iteration);
        total += test_recursive_phi(iteration % 10, 0);
        total += test_multiple_incoming_edges(iteration);
        total += test_boolean_phi(iteration, iteration / 2);
        total += test_complex_loop_hot_path(1);
        
        /* Use __builtin_expect to hint hot path */
        if (__builtin_expect((iteration % 1000) == 0, 0)) {
            volatile_counter++;
        }
    }
    
    printf("Result: %d (volatile: %d)\n", total, volatile_counter);
    return total != 0 ? 0 : 1;
}
