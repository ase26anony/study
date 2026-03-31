/* Test program for GCC auto-profile.cc uncovered lines 1312-1333 */
/* Compile with: gcc -O2 -fauto-profile -fprofile-arcs -ftree-vectorize -o test_autofdo test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi node for 'flag' */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* Copy chain between phi and conditional */
    int a = flag;
    int b = a;
    int c = b;
    
    /* Critical conditional comparing against 0/1 */
    if (c == 1) {  /* cmp_rhs is integer constant 1 */
        result += 1000;
    } else {
        result += 2000;
    }
    
    return result;
}

/* Test 2: Phi node from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Multiple returns create phi node for return value */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Same value from different path */
    }
    
    /* Create copy chain */
    int a = condition;
    int b = a;
    int c = copy_chain(b);
    
    /* Conditional with 0/1 comparison */
    if (c == 0) {  /* cmp_rhs is integer constant 0 */
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Test 3: Phi node from switch statement */
int test_phi_from_switch(int mode) {
    int flag;
    
    /* Switch creates phi node for flag */
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
    
    /* Multiple copy assignments */
    int tmp1 = flag;
    int tmp2 = tmp1;
    int tmp3 = tmp2;
    int tmp4 = copy_chain(tmp3);
    
    /* Conditional branch with 0/1 comparison */
    if (tmp4 != 0) {  /* Tests against 0 (inverted) */
        return 1;
    } else {
        return -1;
    }
}

/* Test 4: Recursive function creating phi nodes */
int test_phi_from_recursion(int depth, int limit) {
    if (depth >= limit) {
        return 0;
    }
    
    int child_result = test_phi_from_recursion(depth + 1, limit);
    
    /* Phi node for condition based on recursive result */
    int condition = (child_result % 2 == 0) ? 1 : 0;
    
    /* Copy chain */
    int a = condition;
    int b = a;
    int c = copy_chain(b);
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(c == 1, 1)) {  /* cmp_rhs is 1 */
        return depth + 100;
    } else {
        return depth + 200;
    }
}

/* Test 5: Complex phi with nested control flow */
int test_complex_phi_nested(int iterations) {
    int sum = 0;
    int hot_flag = 0;
    
    /* Outer loop to make block hot */
    for (int i = 0; i < iterations; i++) {
        int inner_flag;
        
        /* Inner conditional creates phi */
        if (i % 10 == 0) {
            inner_flag = 1;
        } else {
            inner_flag = 0;
        }
        
        /* Multiple copy assignments */
        int x = inner_flag;
        int y = x;
        int z = y;
        
        /* Critical conditional in hot loop */
        if (z == 0) {  /* cmp_rhs is 0 */
            sum += i;
            hot_flag = 1;
        } else {
            sum += i * 2;
            hot_flag = 0;
        }
        
        /* Another conditional using the same pattern */
        int a = hot_flag;
        int b = a;
        if (b == 1) {  /* cmp_rhs is 1 */
            sum += 100;
        }
    }
    
    return sum;
}

/* Test 6: Boolean variable creating phi */
int test_bool_phi(int n) {
    _Bool flag1 = 0;
    _Bool flag2 = 1;
    _Bool result_flag;
    
    /* Loop creates phi for boolean */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            result_flag = flag1;  /* false/0 */
        } else {
            result_flag = flag2;  /* true/1 */
        }
        
        /* Copy chain with boolean */
        _Bool a = result_flag;
        _Bool b = a;
        _Bool c = b;
        
        /* Boolean in conditional context (implicit 0/1 comparison) */
        if (c) {  /* Equivalent to c != 0 */
            volatile_sink += i;
        }
    }
    
    /* Final conditional with copy chain */
    int x = result_flag;
    int y = x;
    if (y == 1) {  /* Explicit 0/1 comparison */
        return n * 10;
    }
    return n * 20;
}

/* Main driver to create hot execution paths */
int main() {
    clock_t start = clock();
    int total_result = 0;
    
    /* Run many iterations to create hot paths */
    const int hot_iterations = 100000;
    
    for (int i = 0; i < hot_iterations; i++) {
        /* Mix different test functions to exercise various patterns */
        total_result += test_phi_after_loop(i % 100 + 1);
        total_result += test_phi_from_multiple_returns(i, i / 2);
        total_result += test_phi_from_switch(i);
        
        if (i % 1000 == 0) {
            total_result += test_phi_from_recursion(0, 5);
        }
        
        total_result += test_complex_phi_nested(10);
        total_result += test_bool_phi(i % 50 + 1);
        
        /* Prevent loop unrolling from simplifying too much */
        volatile_sink = i;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total result: %d\n", total_result);
    printf("Elapsed time: %.2f seconds\n", elapsed);
    printf("Iterations: %d\n", hot_iterations);
    
    return total_result != 0 ? 0 : 1;
}
