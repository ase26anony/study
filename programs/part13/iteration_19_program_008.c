/* Test program for GCC auto-profile.cc uncovered lines */
#include <stdio.h>
#include <stdlib.h>

/* Volatile helper to prevent optimization */
static volatile int vol_counter = 0;

/* Function to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    int b = a;
    int c = b;
    vol_counter++;  /* Prevent optimization */
    return c;
}

/* Test 1: Phi node from loop-carried dependency */
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < n; ++i) {
        if (i % 3 == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int chain_var = copy_chain(flag);
    
    /* Critical conditional comparing against 0/1 */
    if (chain_var == 0) {  /* cmp_lhs is SSA_NAME from phi via copy chain */
        result += 100;
    } else {
        result += 200;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int flag;
    
    /* Multiple returns create phi for 'flag' at merge point */
    if (x > 100) {
        flag = 1;
    } else if (x > 50) {
        flag = 0;
    } else {
        flag = (y % 2 == 0) ? 1 : 0;
    }
    
    /* Multiple copy assignments */
    int a = flag;
    int b = a;
    int c = b;
    int d = c;
    
    /* Conditional with 0/1 comparison */
    if (d == 1) {  /* Should trigger the uncovered code */
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
        default:
            status = 0;
            break;
    }
    
    /* Create longer copy chain */
    int tmp1 = status;
    int tmp2 = tmp1;
    volatile int tmp3 = tmp2;  /* volatile prevents optimization */
    int tmp4 = tmp3;
    
    /* Conditional branch with phi-defined variable */
    if (tmp4 != 0) {  /* != 0 is equivalent to == 1 for 0/1 values */
        return 1;
    }
    return 0;
}

/* Test 4: Recursive function creating phi nodes */
int test_recursive_phi(int depth, int val) {
    if (depth <= 0) {
        return val;
    }
    
    int next_val;
    if (val > 100) {
        next_val = 1;
    } else {
        next_val = 0;
    }
    
    /* Recursive call creates phi for return value */
    int result = test_recursive_phi(depth - 1, next_val);
    
    /* Copy chain */
    int a = result;
    int b = a;
    
    /* Conditional on phi result */
    if (b == 0) {
        return depth * 10;
    } else {
        return depth * 20;
    }
}

/* Test 5: Complex nested control flow with phi */
int test_complex_phi(int x) {
    int flag = 0;
    
    /* Nested loops and conditionals */
    for (int i = 0; i < 10; ++i) {
        if (x > i) {
            for (int j = 0; j < 5; ++j) {
                if ((i + j) % 2 == 0) {
                    flag = 1;
                } else {
                    flag = 0;
                }
            }
        }
    }
    
    /* Multiple copy operations */
    int chain1 = flag;
    int chain2 = chain1;
    int chain3 = chain2;
    chain3 += vol_counter;  /* Use volatile to prevent dead code elimination */
    int chain4 = chain3;
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(chain4 == 1, 1)) {
        return x * 100;
    } else {
        return x * 50;
    }
}

/* Test 6: Boolean variable phi */
int test_bool_phi(int a, int b) {
    _Bool flag;
    
    if (a > b) {
        flag = 1;
    } else if (a < b) {
        flag = 0;
    } else {
        flag = (a % 2 == 0);
    }
    
    /* Boolean copy chain */
    _Bool b1 = flag;
    _Bool b2 = b1;
    _Bool b3 = b2;
    
    /* Boolean in conditional context (implicit comparison with 0) */
    if (b3) {  /* Equivalent to if (b3 != 0) */
        return a + b;
    }
    return a - b;
}

/* Main driver to create hot execution paths */
int main(void) {
    int total = 0;
    
    /* Execute many times to create hot paths */
    for (int iteration = 0; iteration < 100000; ++iteration) {
        /* Mix different test functions to create varied control flow */
        total += test_phi_after_loop(iteration % 100);
        total += test_phi_from_multiple_returns(iteration, iteration * 2);
        total += test_phi_from_switch(iteration);
        total += test_recursive_phi(iteration % 5, iteration);
        total += test_complex_phi(iteration % 50);
        total += test_bool_phi(iteration, iteration / 2);
        
        /* Occasionally reset volatile to prevent overflow */
        if (iteration % 10000 == 0) {
            vol_counter = 0;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
